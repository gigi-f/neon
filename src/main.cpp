#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <cstdarg>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "ecs.h"
#include "components.h"
#include "equipment.h"
#include "inventory.h"
#include "target_selection.h"
#include "simulation_systems.h"
#include "simulation_coordinator.h"
#include "world_generation.h"
#include "stb_image.h"

SDL_Texture* loadTextureTransparentBg(SDL_Renderer* renderer, const char* path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image " << path << ": " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    // Punch out near-white background pixels (threshold 230 handles compression artifacts)
    for (int i = 0; i < width * height * 4; i += 4) {
        if (data[i] > 230 && data[i+1] > 230 && data[i+2] > 230)
            data[i+3] = 0;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        data, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        stbi_image_free(data);
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(data);

    return texture;
}

// ─── Omelette bitmap font ─────────────────────────────────────────────────────
static const int FONT_GLYPH_W    = 16;   // atlas: 256px / 16 cols
static const int FONT_GLYPH_H    = 16;   // atlas:  96px /  6 rows
static const int FONT_COLS       = 16;
static const int FONT_FIRST_CHAR = 32;   // ASCII space

// Load om_large_plain_black_rgba.png (black glyphs on transparent RGBA).
// Inverts each pixel's RGB so glyphs become white — SDL_SetTextureColorMod
// can then tint them to any color at draw time.
SDL_Texture* loadFontTexture(SDL_Renderer* renderer, const char* path) {
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        std::cerr << "Failed to load font: " << path << std::endl;
        return nullptr;
    }
    for (int i = 0; i < w * h * 4; i += 4) {
        data[i]   = 255 - data[i];
        data[i+1] = 255 - data[i+1];
        data[i+2] = 255 - data[i+2];
        // alpha (i+3) unchanged — preserves transparency
    }
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        data, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surf) { stbi_image_free(data); return nullptr; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surf);
    stbi_image_free(data);
    return tex;
}

static void drawGlyph(SDL_Renderer* r, SDL_Texture* font, char c,
                      int x, int y, SDL_Color col, float scale) {
    int idx = static_cast<int>(static_cast<unsigned char>(c)) - FONT_FIRST_CHAR;
    if (idx < 0 || idx >= FONT_COLS * 6) return;
    SDL_Rect src = { (idx % FONT_COLS) * FONT_GLYPH_W,
                     (idx / FONT_COLS) * FONT_GLYPH_H,
                     FONT_GLYPH_W, FONT_GLYPH_H };
    SDL_Rect dst = { x, y,
                     static_cast<int>(FONT_GLYPH_W * scale),
                     static_cast<int>(FONT_GLYPH_H * scale) };
    SDL_SetTextureColorMod(font, col.r, col.g, col.b);
    SDL_SetTextureAlphaMod(font, col.a);
    SDL_RenderCopy(r, font, &src, &dst);
}

static void drawText(SDL_Renderer* r, SDL_Texture* font, const char* text,
                     int x, int y, SDL_Color col, float scale) {
    int gw = static_cast<int>(FONT_GLYPH_W * scale);
    for (int i = 0; text[i]; ++i)
        drawGlyph(r, font, text[i], x + i * gw, y, col, scale);
}

static SDL_Color alertColor(SimulationAlertSeverity severity) {
    switch (severity) {
        case SimulationAlertSeverity::DANGER: return {230, 70, 70, 235};
        case SimulationAlertSeverity::WARNING: return {230, 195, 70, 235};
        case SimulationAlertSeverity::INFO: return {120, 220, 220, 225};
    }
    return {120, 220, 220, 225};
}

// Centers the string (horizontally and vertically) on world-space position (cx, cy).
static void drawTextCentered(SDL_Renderer* r, SDL_Texture* font, const char* text,
                              int cx, int cy, SDL_Color col, float scale) {
    int len = static_cast<int>(strlen(text));
    int gw  = static_cast<int>(FONT_GLYPH_W * scale);
    int gh  = static_cast<int>(FONT_GLYPH_H * scale);
    drawText(r, font, text, cx - (len * gw) / 2, cy - gh / 2, col, scale);
}

static SDL_Color infectedGlyphColor(const PathogenComponent& p) {
    uint8_t intensity = static_cast<uint8_t>(120 + std::min(1.0f, p.severity) * 90.0f);
    return {220, intensity, 90, 255};
}

enum class ScanPanelMode { NONE, SURFACE, BIOLOGY, COGNITIVE, FINANCIAL, STRUCTURAL };
enum class LayerOverlayMode { OFF, SURFACE, L0_ENVIRONMENT, L1_BIOLOGY, L2_SOCIAL, L3_ECONOMY, L4_FACTION };

static const char* scanPanelTitle(ScanPanelMode mode) {
    switch (mode) {
        case ScanPanelMode::SURFACE:    return "SURFACE SCAN";
        case ScanPanelMode::BIOLOGY:    return "BIOLOGICAL AUDIT";
        case ScanPanelMode::COGNITIVE:  return "COGNITIVE PROFILE";
        case ScanPanelMode::FINANCIAL:  return "FINANCIAL FORENSICS";
        case ScanPanelMode::STRUCTURAL: return "STRUCTURAL ANALYSIS";
        case ScanPanelMode::NONE:       return "";
    }
    return "";
}

static ScanPanelMode scanModeForEquipment(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::SURFACE_SCAN:        return ScanPanelMode::SURFACE;
        case EquipmentSlot::BIOLOGY_AUDIT:       return ScanPanelMode::BIOLOGY;
        case EquipmentSlot::COGNITIVE_PROFILE:   return ScanPanelMode::COGNITIVE;
        case EquipmentSlot::FINANCIAL_FORENSICS: return ScanPanelMode::FINANCIAL;
        case EquipmentSlot::STRUCTURAL_ANALYSIS: return ScanPanelMode::STRUCTURAL;
        case EquipmentSlot::NONE:
        case EquipmentSlot::FOOD:
        case EquipmentSlot::WATER:
        case EquipmentSlot::MEDICAL:
            return ScanPanelMode::NONE;
    }
    return ScanPanelMode::NONE;
}

static EquipmentSlot equipmentForScanMode(ScanPanelMode mode) {
    switch (mode) {
        case ScanPanelMode::SURFACE:    return EquipmentSlot::SURFACE_SCAN;
        case ScanPanelMode::BIOLOGY:    return EquipmentSlot::BIOLOGY_AUDIT;
        case ScanPanelMode::COGNITIVE:  return EquipmentSlot::COGNITIVE_PROFILE;
        case ScanPanelMode::FINANCIAL:  return EquipmentSlot::FINANCIAL_FORENSICS;
        case ScanPanelMode::STRUCTURAL: return EquipmentSlot::STRUCTURAL_ANALYSIS;
        case ScanPanelMode::NONE:       return EquipmentSlot::NONE;
    }
    return EquipmentSlot::NONE;
}

static int numericHotkeyIndex(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_0: return 0;
        case SDL_SCANCODE_1: return 1;
        case SDL_SCANCODE_2: return 2;
        case SDL_SCANCODE_3: return 3;
        case SDL_SCANCODE_4: return 4;
        case SDL_SCANCODE_5: return 5;
        case SDL_SCANCODE_6: return 6;
        case SDL_SCANCODE_7: return 7;
        case SDL_SCANCODE_8: return 8;
        case SDL_SCANCODE_9: return 9;
        default:             return -1;
    }
}

static const char* layerOverlayName(LayerOverlayMode mode) {
    switch (mode) {
        case LayerOverlayMode::OFF:            return "OFF";
        case LayerOverlayMode::SURFACE:        return "SURFACE";
        case LayerOverlayMode::L0_ENVIRONMENT: return "L0 ENV";
        case LayerOverlayMode::L1_BIOLOGY:     return "L1 BIO";
        case LayerOverlayMode::L2_SOCIAL:      return "L2 SOCIAL";
        case LayerOverlayMode::L3_ECONOMY:     return "L3 ECON";
        case LayerOverlayMode::L4_FACTION:     return "L4 FACTION";
    }
    return "OFF";
}

static LayerOverlayMode nextLayerOverlay(LayerOverlayMode mode) {
    switch (mode) {
        case LayerOverlayMode::OFF:            return LayerOverlayMode::SURFACE;
        case LayerOverlayMode::SURFACE:        return LayerOverlayMode::L0_ENVIRONMENT;
        case LayerOverlayMode::L0_ENVIRONMENT: return LayerOverlayMode::L1_BIOLOGY;
        case LayerOverlayMode::L1_BIOLOGY:     return LayerOverlayMode::L2_SOCIAL;
        case LayerOverlayMode::L2_SOCIAL:      return LayerOverlayMode::L3_ECONOMY;
        case LayerOverlayMode::L3_ECONOMY:     return LayerOverlayMode::L4_FACTION;
        case LayerOverlayMode::L4_FACTION:     return LayerOverlayMode::OFF;
    }
    return LayerOverlayMode::OFF;
}

static const char* zoneName(ZoneType zone) {
    switch (zone) {
        case ZoneType::URBAN_CORE:  return "URBAN_CORE";
        case ZoneType::CORPORATE:   return "CORPORATE";
        case ZoneType::COMMERCIAL:  return "COMMERCIAL";
        case ZoneType::RESIDENTIAL: return "RESIDENTIAL";
        case ZoneType::SLUM:        return "SLUM";
        case ZoneType::INDUSTRIAL:  return "INDUSTRIAL";
    }
    return "UNKNOWN";
}

static const char* roadName(RoadType road) {
    switch (road) {
        case RoadType::PRIMARY:         return "PRIMARY";
        case RoadType::SECONDARY:       return "SECONDARY";
        case RoadType::ALLEY:           return "ALLEY";
        case RoadType::PEDESTRIAN_PATH: return "PEDESTRIAN";
        case RoadType::MAGLIFT_TRACK:   return "MAGLIFT";
    }
    return "UNKNOWN";
}

static const char* vehicleName(VehicleComponent::Type type) {
    switch (type) {
        case VehicleComponent::EMMV:      return "EMMV";
        case VehicleComponent::MAGLIFT:   return "MAGLIFT";
        case VehicleComponent::TRANSPORT: return "TRANSPORT";
    }
    return "UNKNOWN";
}

static const char* materialName(MaterialType material) {
    switch (material) {
        case MaterialType::STEEL:               return "STEEL";
        case MaterialType::REINFORCED_CONCRETE: return "CONCRETE";
        case MaterialType::COMPOSITE:           return "COMPOSITE";
        case MaterialType::SCRAP:               return "SCRAP";
    }
    return "UNKNOWN";
}

static const char* rankName(SocialRank rank) {
    switch (rank) {
        case SocialRank::VAGRANT:         return "VAGRANT";
        case SocialRank::SLUM_DWELLER:    return "SLUM_DWELLER";
        case SocialRank::WORKING_POOR:    return "WORKING_POOR";
        case SocialRank::MIDDLE_CLASS:    return "MIDDLE_CLASS";
        case SocialRank::CORPORATE_ELITE: return "CORPORATE_ELITE";
    }
    return "UNKNOWN";
}

static const char* scheduleName(ScheduleState state) {
    switch (state) {
        case ScheduleState::SLEEPING: return "SLEEPING";
        case ScheduleState::WORKING:  return "WORKING";
        case ScheduleState::LEISURE:  return "LEISURE";
        case ScheduleState::TRANSIT:  return "TRANSIT";
    }
    return "UNKNOWN";
}

static const char* desireName(DesireType desire) {
    switch (desire) {
        case DesireType::NONE:           return "NONE";
        case DesireType::SATISFY_HUNGER: return "HUNGER";
        case DesireType::SATISFY_THIRST: return "THIRST";
        case DesireType::FIND_TRANSIT:   return "TRANSIT";
        case DesireType::BUY_FOOD:       return "BUY_FOOD";
        case DesireType::BUY_WATER:      return "BUY_WATER";
    }
    return "UNKNOWN";
}

static const char* itemName(ItemComponent::Type type) {
    switch (type) {
        case ItemComponent::FOOD:                       return "FOOD";
        case ItemComponent::WATER:                      return "WATER";
        case ItemComponent::MEDICAL:                    return "MEDICAL";
        case ItemComponent::SURFACE_SCAN_TOOL:          return "SURFACE SCAN";
        case ItemComponent::BIOLOGY_AUDIT_TOOL:         return "BIO AUDIT";
        case ItemComponent::COGNITIVE_PROFILE_TOOL:     return "COG PROFILE";
        case ItemComponent::FINANCIAL_FORENSICS_TOOL:   return "FIN FORENSICS";
        case ItemComponent::STRUCTURAL_ANALYSIS_TOOL:   return "STRUCT ANALYSIS";
    }
    return "UNKNOWN";
}

static ItemComponent::Type nextMarketTradeType(ItemComponent::Type type) {
    if (type == ItemComponent::FOOD) return ItemComponent::WATER;
    if (type == ItemComponent::WATER) return ItemComponent::MEDICAL;
    return ItemComponent::FOOD;
}

static const char* tradeModeName(PlayerTradeMode mode) {
    if (mode == PlayerTradeMode::BUY) return "BUY";
    if (mode == PlayerTradeMode::SELL) return "SELL";
    return "NONE";
}

static const char* primaryEntityType(Registry& registry, Entity e) {
    if (registry.has<PlayerComponent>(e)) return "PLAYER";
    if (registry.has<VehicleComponent>(e)) return "VEHICLE";
    if (registry.has<CitizenComponent>(e)) return "CITIZEN";
    if (registry.has<MarketComponent>(e)) return "MARKET";
    if (registry.has<BuildingComponent>(e)) return "BUILDING";
    if (registry.has<ItemComponent>(e)) return "ITEM";
    if (registry.has<StationComponent>(e)) return "STATION";
    if (registry.has<WaitingAreaComponent>(e)) return "WAITING_AREA";
    if (registry.has<StaircaseComponent>(e)) return "STAIRCASE";
    if (registry.has<TrafficLightComponent>(e)) return "TRAFFIC_LIGHT";
    if (registry.has<IntersectionComponent>(e)) return "INTERSECTION";
    if (registry.has<RoadComponent>(e)) return "ROAD";
    return "ENTITY";
}

static const char* surfaceOverlayLabel(Registry& registry, Entity e) {
    if (registry.has<PlayerComponent>(e)) return "PLY";
    if (registry.has<VehicleComponent>(e)) return "VEH";
    if (registry.has<CitizenComponent>(e)) return "CIT";
    if (registry.has<MarketComponent>(e)) return "MKT";
    if (registry.has<BuildingComponent>(e)) return "BLD";
    if (registry.has<ItemComponent>(e)) return "ITM";
    if (registry.has<StationComponent>(e)) return "STN";
    if (registry.has<WaitingAreaComponent>(e)) return "WAIT";
    if (registry.has<StaircaseComponent>(e)) return "STR";
    if (registry.has<TrafficLightComponent>(e)) return "SIG";
    if (registry.has<IntersectionComponent>(e)) return "X";
    if (registry.has<RoadComponent>(e)) return "RD";
    return "ENT";
}

static const char* rankOverlayLabel(SocialRank rank) {
    switch (rank) {
        case SocialRank::VAGRANT:         return "VAG";
        case SocialRank::SLUM_DWELLER:    return "SLUM";
        case SocialRank::WORKING_POOR:    return "WORK";
        case SocialRank::MIDDLE_CLASS:    return "MID";
        case SocialRank::CORPORATE_ELITE: return "ELIT";
    }
    return "SOC";
}

static void pushLine(std::vector<std::string>& lines, const char* fmt, ...) {
    char buf[160];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    lines.emplace_back(buf);
}

static std::vector<std::string> buildScanLines(Registry& registry, Entity target,
                                               Entity player, ScanPanelMode mode) {
    std::vector<std::string> lines;
    if (target == MAX_ENTITIES || !registry.alive(target) ||
        !registry.has<TransformComponent>(target)) {
        lines.emplace_back("NO TARGET IN RANGE");
        lines.emplace_back("Move closer and scan again.");
        return lines;
    }

    auto& t = registry.get<TransformComponent>(target);
    float distance = 0.0f;
    if (registry.has<TransformComponent>(player)) {
        auto& pt = registry.get<TransformComponent>(player);
        float dx = t.x - pt.x, dy = t.y - pt.y;
        distance = std::sqrt(dx * dx + dy * dy);
    }

    pushLine(lines, "ID:%u TYPE:%s", target, primaryEntityType(registry, target));
    pushLine(lines, "POS:%.0f,%.0f DIST:%.0f", t.x, t.y, distance);

    if (registry.has<GlyphComponent>(target))
        pushLine(lines, "GLYPH:%s", registry.get<GlyphComponent>(target).chars.c_str());
    if (registry.has<ZoningComponent>(target))
        pushLine(lines, "ZONE:%s", zoneName(registry.get<ZoningComponent>(target).type));

    if (mode == ScanPanelMode::SURFACE) {
        std::string tags;
        if (registry.has<SolidComponent>(target)) tags += " SOLID";
        if (registry.has<InteriorComponent>(target)) tags += " INTERIOR";
        if (registry.has<BiologyComponent>(target)) tags += " BIO";
        if (registry.has<CognitiveComponent>(target)) tags += " COG";
        if (registry.has<EconomicComponent>(target)) tags += " ECO";
        if (registry.has<StructuralComponent>(target)) tags += " STRUCT";
        if (registry.has<PowerNodeComponent>(target)) tags += " POWER";
        if (registry.has<PathogenComponent>(target)) tags += " INFECTED";
        if (!tags.empty()) pushLine(lines, "TAGS:%s", tags.c_str());
        if (registry.has<RoadComponent>(target))
            pushLine(lines, "ROAD:%s DENS:%.2f", roadName(registry.get<RoadComponent>(target).type),
                     registry.get<RoadComponent>(target).traffic_density);
        if (registry.has<VehicleComponent>(target))
            pushLine(lines, "VEH:%s SPEED:%.0f", vehicleName(registry.get<VehicleComponent>(target).type),
                     registry.get<VehicleComponent>(target).max_speed);
        if (registry.has<ItemComponent>(target))
            pushLine(lines, "ITEM:%s RESTORE:%.0f", itemName(registry.get<ItemComponent>(target).type),
                     registry.get<ItemComponent>(target).restore_value);
        if (registry.has<ItemComponent>(target)) {
            auto& item = registry.get<ItemComponent>(target);
            pushLine(lines, "FLAGS:%s", itemFlagSummary(item.flags).c_str());
            if (itemProvenanceTracked(item.flags, item.provenance))
                pushLine(lines, "PROV:%s", itemProvenanceSummary(item.flags, item.provenance).c_str());
        }
    } else if (mode == ScanPanelMode::BIOLOGY) {
        if (!registry.has<BiologyComponent>(target)) {
            lines.emplace_back("NO BIOLOGICAL DATA");
            return lines;
        }
        auto& b = registry.get<BiologyComponent>(target);
        pushLine(lines, "HP:%d HNG:%d THR:%d FAT:%d",
                 (int)b.health, (int)b.hunger, (int)b.thirst, (int)b.fatigue);
        pushLine(lines, "HR:%d O2:%d%% BP:%d/%d",
                 (int)b.vitals.heart_rate, (int)(b.vitals.oxygen_sat * 100.0f),
                 (int)b.vitals.blood_pressure[0], (int)b.vitals.blood_pressure[1]);
        pushLine(lines, "ORG H:%d L:%d B:%d",
                 (int)b.organs.heart, (int)b.organs.lungs, (int)b.organs.brain);
        if (registry.has<PathogenComponent>(target)) {
            auto& p = registry.get<PathogenComponent>(target);
            pushLine(lines, "PATH STR:%d LOAD:%d SEV:%d%%",
                     p.strain_id, (int)p.infection_load, (int)(p.severity * 100.0f));
        } else {
            lines.emplace_back("PATH: CLEAR");
        }
        if (registry.has<InjuryComponent>(target)) {
            auto& inj = registry.get<InjuryComponent>(target);
            int count = 0;
            for (const auto& slot : inj.slots)
                if (slot.type != InjuryType::NONE && slot.severity > 0.0f) ++count;
            pushLine(lines, "INJURIES:%d", count);
        } else {
            lines.emplace_back("INJURIES:0");
        }
    } else if (mode == ScanPanelMode::COGNITIVE) {
        if (!registry.has<CognitiveComponent>(target)) {
            lines.emplace_back("NO COGNITIVE DATA");
            return lines;
        }
        auto& c = registry.get<CognitiveComponent>(target);
        pushLine(lines, "PAD P:%+.2f A:%+.2f D:%+.2f", c.pleasure, c.arousal, c.dominance);
        pushLine(lines, "MEM:%d/%d", c.mem_size, CognitiveComponent::MEM_CAP);
        if (registry.has<IntentionComponent>(target)) {
            auto& intent = registry.get<IntentionComponent>(target);
            pushLine(lines, "DESIRE:%s COMMIT:%d", desireName(intent.active_desire),
                     (int)intent.commitment);
        } else {
            lines.emplace_back("DESIRE:NONE");
        }
        if (registry.has<ScheduleComponent>(target))
            pushLine(lines, "SCHEDULE:%s", scheduleName(registry.get<ScheduleComponent>(target).state));
        if (registry.has<SocialRankComponent>(target)) {
            auto& sr = registry.get<SocialRankComponent>(target);
            pushLine(lines, "RANK:%s PREST:%+.2f", rankName(sr.rank), sr.prestige);
        }
        if (registry.has<RelationshipComponent>(target))
            pushLine(lines, "RELATIONS:%d", registry.get<RelationshipComponent>(target).count);
    } else if (mode == ScanPanelMode::FINANCIAL) {
        bool any = false;
        if (registry.has<EconomicComponent>(target)) {
            any = true;
            auto& e = registry.get<EconomicComponent>(target);
            pushLine(lines, "CREDITS:%.1f WAGE:%.1f", e.credits, e.daily_wage);
            pushLine(lines, "MKT REP:%+.2f", e.market_reputation);
            pushLine(lines, "EMPLOYER:%s", e.employer == MAX_ENTITIES ? "NONE" : "ASSIGNED");
        }
        if (registry.has<EmployerComponent>(target)) {
            any = true;
            auto& emp = registry.get<EmployerComponent>(target);
            pushLine(lines, "EMPLOYEES:%d/%d", emp.employee_count, emp.capacity);
        }
        if (registry.has<MarketComponent>(target)) {
            any = true;
            auto& m = registry.get<MarketComponent>(target);
            pushLine(lines, "STOCK F:%d W:%d M:%d", (int)m.food_stock, (int)m.water_stock,
                     (int)m.medical_stock);
            pushLine(lines, "BASE PRICE:%.1f RESTOCK:%.1f", m.base_price, m.restock_rate);
            pushLine(lines, "GREED:%+.2f", m.greed_margin);
        }
        if (registry.has<ItemComponent>(target)) {
            any = true;
            auto& item = registry.get<ItemComponent>(target);
            pushLine(lines, "ITEM:%s RESTORE:%.0f", itemName(item.type), item.restore_value);
            pushLine(lines, "FLAGS:%s", itemFlagSummary(item.flags).c_str());
            if (itemProvenanceTracked(item.flags, item.provenance))
                pushLine(lines, "PROV:%s", itemProvenanceSummary(item.flags, item.provenance).c_str());
        }
        if (!any) lines.emplace_back("NO FINANCIAL DATA");
    } else if (mode == ScanPanelMode::STRUCTURAL) {
        bool any = false;
        if (registry.has<BuildingComponent>(target)) {
            any = true;
            auto& b = registry.get<BuildingComponent>(target);
            pushLine(lines, "BUILDING:%llu FLOORS:%d",
                     (unsigned long long)b.stable_id, b.floors);
            pushLine(lines, "ENTERABLE:%s", b.is_enterable ? "YES" : "NO");
        }
        if (registry.has<StructuralComponent>(target)) {
            any = true;
            auto& s = registry.get<StructuralComponent>(target);
            pushLine(lines, "MAT:%s INTEGRITY:%d%%", materialName(s.material_type),
                     (int)s.integrity);
            pushLine(lines, "EXPOSED:%s COLLAPSED:%s", s.is_exposed ? "YES" : "NO",
                     registry.has<CollapsedComponent>(target) ? "YES" : "NO");
        }
        if (registry.has<PowerNodeComponent>(target)) {
            any = true;
            auto& p = registry.get<PowerNodeComponent>(target);
            pushLine(lines, "POWER SUP:%.0f DEM:%.0f", p.supply, p.demand);
            pushLine(lines, "POWERED:%s", p.powered ? "YES" : "NO");
        }
        if (!any) lines.emplace_back("NO STRUCTURAL DATA");
    }
    return lines;
}

static void drawScanPanel(SDL_Renderer* renderer, SDL_Texture* font, Registry& registry,
                          Entity target, Entity player, ScanPanelMode mode,
                          int screenW, int screenH) {
    if (!font || mode == ScanPanelMode::NONE) return;

    SDL_Rect panel = { screenW - 330, 12, 318, 178 };
    SDL_SetRenderDrawColor(renderer, 4, 8, 12, 220);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 70, 210, 190, 230);
    SDL_RenderDrawRect(renderer, &panel);

    SDL_Color titleCol = {120, 245, 220, 255};
    SDL_Color textCol = {215, 235, 220, 235};
    drawText(renderer, font, scanPanelTitle(mode), panel.x + 8, panel.y + 8, titleCol, 0.75f);

    auto lines = buildScanLines(registry, target, player, mode);
    int y = panel.y + 26;
    for (const auto& line : lines) {
        if (y > panel.y + panel.h - 16) break;
        drawText(renderer, font, line.c_str(), panel.x + 8, y, textCol, 0.65f);
        y += 12;
    }
}

static bool visibleWorldRect(const TransformComponent& t, float viewLeft, float viewRight,
                             float viewTop, float viewBottom) {
    return !(t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
             t.y + t.height/2.0f < viewTop  || t.y - t.height/2.0f > viewBottom);
}

static SDL_Rect screenRectFor(const TransformComponent& t, float camX, float camY,
                              float centerX, float centerY, float scale) {
    int w = std::max(8, static_cast<int>(t.width * scale));
    int h = std::max(8, static_cast<int>(t.height * scale));
    int x = static_cast<int>(centerX + (t.x - camX) * scale - w / 2.0f);
    int y = static_cast<int>(centerY + (t.y - camY) * scale - h / 2.0f);
    return {x, y, w, h};
}

static void drawEquippedToolRange(SDL_Renderer* renderer, Registry& registry, Entity player,
                                  float camX, float camY, float centerX, float centerY, float scale) {
    if (!registry.has<EquipmentComponent>(player) || !registry.has<TransformComponent>(player)) return;

    EquipmentSlot slot = registry.get<EquipmentComponent>(player).equipped;
    if (!isScanEquipment(slot) || !hasInventoryItemForEquipment(registry, player, slot)) return;

    float range = equipmentRange(slot);
    if (range <= 0.0f) return;

    const auto& pt = registry.get<TransformComponent>(player);
    SDL_Rect rangeRect = {
        static_cast<int>(centerX + (pt.x - range - camX) * scale),
        static_cast<int>(centerY + (pt.y - range - camY) * scale),
        static_cast<int>(range * 2.0f * scale),
        static_cast<int>(range * 2.0f * scale)
    };
    SDL_SetRenderDrawColor(renderer, 120, 245, 220, 130);
    SDL_RenderDrawRect(renderer, &rangeRect);
    rangeRect.x += 2;
    rangeRect.y += 2;
    rangeRect.w = std::max(2, rangeRect.w - 4);
    rangeRect.h = std::max(2, rangeRect.h - 4);
    SDL_SetRenderDrawColor(renderer, 20, 80, 75, 110);
    SDL_RenderDrawRect(renderer, &rangeRect);
}

static void drawLayerMarker(SDL_Renderer* renderer, SDL_Texture* font,
                            const SDL_Rect& rect, SDL_Color color, const char* label) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_Rect inset = {rect.x + 2, rect.y + 2, std::max(2, rect.w - 4), std::max(2, rect.h - 4)};
    Uint8 insetAlpha = color.a < 120 ? color.a : 120;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, insetAlpha);
    SDL_RenderDrawRect(renderer, &inset);

    if (font && label && label[0]) {
        SDL_Color textCol = {color.r, color.g, color.b, 235};
        drawTextCentered(renderer, font, label, rect.x + rect.w / 2, rect.y + rect.h / 2, textCol, 0.55f);
    }
}

static bool layerMarkerForEntity(Registry& registry, Entity e, LayerOverlayMode mode,
                                 SDL_Color& color, const char*& label) {
    label = "";
    switch (mode) {
        case LayerOverlayMode::SURFACE:
            label = surfaceOverlayLabel(registry, e);
            color = {120, 220, 245, 190};
            return registry.has<GlyphComponent>(e) || registry.has<RoadComponent>(e) ||
                   registry.has<BuildingComponent>(e) || registry.has<ItemComponent>(e);

        case LayerOverlayMode::L0_ENVIRONMENT:
            if (registry.has<CollapsedComponent>(e)) {
                label = "COL";
                color = {230, 70, 70, 230};
                return true;
            }
            if (registry.has<StructuralComponent>(e)) {
                const auto& s = registry.get<StructuralComponent>(e);
                label = "STR";
                color = (s.integrity <= 40.0f) ? SDL_Color{230, 90, 70, 220} :
                        (s.integrity <= 80.0f) ? SDL_Color{220, 180, 80, 200} :
                                                  SDL_Color{100, 210, 150, 170};
                return true;
            }
            if (registry.has<PowerNodeComponent>(e)) {
                const auto& p = registry.get<PowerNodeComponent>(e);
                label = p.powered ? "PWR" : "NO P";
                color = p.powered ? SDL_Color{100, 220, 255, 200} : SDL_Color{240, 80, 80, 220};
                return true;
            }
            return false;

        case LayerOverlayMode::L1_BIOLOGY:
            if (registry.has<PathogenComponent>(e)) {
                label = "INF";
                color = {235, 110, 70, 230};
                return true;
            }
            if (registry.has<BiologyComponent>(e)) {
                const auto& b = registry.get<BiologyComponent>(e);
                label = "BIO";
                color = (b.health < 35.0f || b.hunger < 25.0f || b.thirst < 25.0f)
                    ? SDL_Color{235, 80, 80, 220}
                    : SDL_Color{120, 230, 130, 180};
                return true;
            }
            if (registry.has<ItemComponent>(e)) {
                label = "ITEM";
                color = {230, 190, 80, 210};
                return true;
            }
            return false;

        case LayerOverlayMode::L2_SOCIAL:
            if (registry.has<SocialRankComponent>(e)) {
                const auto& sr = registry.get<SocialRankComponent>(e);
                label = rankOverlayLabel(sr.rank);
                color = (sr.rank >= SocialRank::MIDDLE_CLASS)
                    ? SDL_Color{190, 150, 255, 210}
                    : SDL_Color{120, 170, 240, 190};
                return true;
            }
            if (registry.has<CognitiveComponent>(e)) {
                label = "COG";
                color = {150, 210, 255, 180};
                return true;
            }
            if (registry.has<RelationshipComponent>(e)) {
                label = "REL";
                color = {120, 220, 210, 180};
                return true;
            }
            return false;

        case LayerOverlayMode::L3_ECONOMY:
            if (registry.has<MarketComponent>(e)) {
                label = "MKT";
                color = {255, 215, 70, 230};
                return true;
            }
            if (registry.has<EmployerComponent>(e)) {
                label = "JOB";
                color = {120, 220, 255, 210};
                return true;
            }
            if (registry.has<EconomicComponent>(e)) {
                const auto& eco = registry.get<EconomicComponent>(e);
                label = (eco.employer == MAX_ENTITIES) ? "UNEMP" : "ECO";
                color = (eco.credits < 25.0f) ? SDL_Color{240, 90, 80, 210}
                                               : SDL_Color{160, 230, 120, 180};
                return true;
            }
            return false;

        case LayerOverlayMode::L4_FACTION:
        case LayerOverlayMode::OFF:
            return false;
    }
    return false;
}

static void drawLayerOverlay(SDL_Renderer* renderer, SDL_Texture* font, Registry& registry,
                             LayerOverlayMode mode, float camX, float camY,
                             float centerX, float centerY, float scale,
                             float viewLeft, float viewRight,
                             float viewTop, float viewBottom) {
    if (mode == LayerOverlayMode::OFF || mode == LayerOverlayMode::L4_FACTION) return;

    auto view = registry.view<TransformComponent>();
    for (Entity e : view) {
        if (!registry.alive(e)) continue;
        const auto& t = registry.get<TransformComponent>(e);
        if (!visibleWorldRect(t, viewLeft, viewRight, viewTop, viewBottom)) continue;

        SDL_Color color{};
        const char* label = "";
        if (!layerMarkerForEntity(registry, e, mode, color, label)) continue;

        SDL_Rect rect = screenRectFor(t, camX, camY, centerX, centerY, scale);
        rect.x -= 2;
        rect.y -= 2;
        rect.w += 4;
        rect.h += 4;
        drawLayerMarker(renderer, font, rect, color, label);
    }
}
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Neon Oubliette",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set blend mode for transparent PNG
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Initialize ECS
    Registry registry;
    SimulationCoordinator coordinator;
    PlayerInputSystem playerInput;
    MovementSystem mover;
    CameraSystem cameraSystem;
    TrafficLightSystem tlSystem;
    CitizenAISystem citizenAI;
    AmbientSpawnSystem ambient;
    TrafficDensitySystem trafficDensity;
    IntersectionSystem intersectionSystem;
    TimeOfDaySystem timeOfDaySystem;
    TransitSystem transitSystem;
    BiologySystem biologySystem;
    PathogenSystem pathogenSystem;
    InjurySystem  injurySystem;
    MemoryFormationSystem memoryFormation;
    ConsumableSystem consumables;
    CognitiveSystem cognitiveSystem;
    ScheduleSystem scheduleSystem;
    GDISystem gdiSystem;
    RelationshipSystem relationshipSystem;
    RumorSystem rumorSystem;
    WageSystem  wageSystem;
    MarketSystem marketSystem;
    SocialHierarchySystem socialHierarchySystem;
    TemperatureSystem temperatureSystem;
    AtmosphereSystem atmosphereSystem;
    StructuralDecaySystem structuralDecaySystem;
    PowerGridSystem powerGridSystem;

    // ... player and camera entity creation ...
    Entity player = registry.create();
    registry.assign<TransformComponent>(player, -50.0f, -50.0f, 16.0f, 16.0f); // Size in WU
    registry.assign<MovementComponent>(player, 0.0f, 0.0f, MovementComponent::NORMAL);
    registry.assign<PlayerComponent>(player, 100.0f); // speed in WU/s
    registry.assign<GlyphComponent>(player, std::string("@"),
        (uint8_t)255, (uint8_t)200, (uint8_t)50, (uint8_t)255, 1.0f, true);
    registry.assign<BiologyComponent>(player);
    registry.assign<CognitiveComponent>(player);
    registry.assign<SurvivalInventoryComponent>(player);
    auto& playerInventory = registry.assign<DiscreteInventoryComponent>(player);
    addBaselineScanTools(playerInventory);
    registry.assign<EquipmentComponent>(player);

    Entity camera = registry.create();
    registry.assign<CameraComponent>(camera, 0.0f, 0.0f, 2.0f, 800.0f, 600.0f, player);

    // ... vehicle creation ...
    Entity pVehicle = registry.create();
    registry.assign<TransformComponent>(pVehicle, -50.0f, -20.0f, 60.0f, 30.0f);
    registry.assign<MovementComponent>(pVehicle, 0.0f, 0.0f, MovementComponent::NORMAL);
    registry.assign<VehicleComponent>(pVehicle, VehicleComponent::MAGLIFT, MAX_ENTITIES, 250.0f);
    registry.assign<SolidComponent>(pVehicle); // Vehicles are solid
    registry.assign<GlyphComponent>(pVehicle, std::string("M"),
        (uint8_t)255, (uint8_t)80, (uint8_t)140, (uint8_t)255, 0.5f, true, true);
    registry.assign<OwnershipComponent>(pVehicle, player);
    registry.assign<HomeLocationComponent>(pVehicle, -50.0f, -20.0f);

    Entity worldConfig = registry.create();
    auto& config = registry.assign<WorldConfigComponent>(worldConfig);
    registry.assign<TimeOfDayComponent>(worldConfig); // starts at 08:00 DAY

    // Initialize Temperature Grid
    auto& tempGrid = registry.assign<TemperatureGridComponent>(worldConfig);
    tempGrid.cols = static_cast<int>((config.world_max - config.world_min) / config.macro_cell_size);
    tempGrid.rows = static_cast<int>((config.world_max - config.world_min) / config.macro_cell_size);
    tempGrid.grid.assign(tempGrid.cols * tempGrid.rows, 20.0f); // default 20C
    tempGrid.back.assign(tempGrid.cols * tempGrid.rows, 20.0f);

    // Initialize Atmosphere Grid
    auto& atmoGrid = registry.assign<AtmosphereGridComponent>(worldConfig);
    atmoGrid.cols = tempGrid.cols;
    atmoGrid.rows = tempGrid.rows;
    atmoGrid.grid.assign(atmoGrid.cols * atmoGrid.rows, 100.0f); // default 100 air quality
    atmoGrid.back.assign(atmoGrid.cols * atmoGrid.rows, 100.0f);

    generateChicagoCity(registry, 4, 4);

    char* basePath = SDL_GetBasePath();
    std::string base(basePath ? basePath : "");
    if (basePath) SDL_free(basePath);

    SDL_Texture* fontTex = loadFontTexture(
        renderer, (base + "assets/om_large_plain_black_rgba.png").c_str());

    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float ambientTimer = 0.0f;
    std::vector<ViolenceEvent> violenceEvents;
    ScanPanelMode scanMode = ScanPanelMode::NONE;
    Entity scanTarget = MAX_ENTITIES;
    LayerOverlayMode layerOverlay = LayerOverlayMode::OFF;
    SimulationAlertStack alertStack(8);
    FeedbackState feedback(12);
    WeatherState lastWeather = registry.get<TimeOfDayComponent>(worldConfig).weather;
    PlayerMarketTradePreview tradePreview;
    ItemComponent::Type selectedBuyType = ItemComponent::MEDICAL;

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        bool interactPressed = false;
        bool consumeInventoryPressed = false;
        bool useDiscreteInventoryPressed = false;
        bool dropDiscreteInventoryPressed = false;
        bool confirmTradePressed = false;
        ItemComponent::Type consumeInventoryType = ItemComponent::FOOD;
        EquipmentSlot pendingScanEquipment = EquipmentSlot::NONE;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                const bool shiftHeld = (event.key.keysym.mod & KMOD_SHIFT) != 0;
                const bool ctrlHeld = (event.key.keysym.mod & KMOD_CTRL) != 0;
                const int hotkeyIndex = numericHotkeyIndex(event.key.keysym.scancode);
                auto equipSlot = [&](int index) {
                    if (!registry.has<EquipmentComponent>(player)) return EquipmentSlot::NONE;
                    auto& equipment = registry.get<EquipmentComponent>(player);
                    EquipmentSlot slot = equipment.hotkeys[static_cast<size_t>(index)];
                    return slot;
                };
                auto useEquipment = [&](EquipmentSlot slot) {
                    if (isConsumableEquipment(slot)) {
                        consumeInventoryPressed = true;
                        consumeInventoryType = equipmentItemType(slot);
                        return true;
                    }
                    if (isScanEquipment(slot)) {
                        if (!hasInventoryItemForEquipment(registry, player, slot)) {
                            feedback.push(FeedbackCue::DENIED);
                            return false;
                        }
                        pendingScanEquipment = slot;
                        return true;
                    }
                    return false;
                };
                auto selectEquipment = [&](EquipmentSlot slot) {
                    if (isScanEquipment(slot) && !hasInventoryItemForEquipment(registry, player, slot)) {
                        feedback.push(FeedbackCue::DENIED);
                        return false;
                    }
                    if (registry.has<EquipmentComponent>(player)) {
                        registry.get<EquipmentComponent>(player).equipped = slot;
                    }
                    return true;
                };
                auto moveScanCursor = [&](Facing direction) {
                    EquipmentSlot scanSlot = EquipmentSlot::NONE;
                    if (scanMode != ScanPanelMode::NONE) {
                        scanSlot = equipmentForScanMode(scanMode);
                    } else if (registry.has<EquipmentComponent>(player)) {
                        scanSlot = registry.get<EquipmentComponent>(player).equipped;
                    }
                    if (!isScanEquipment(scanSlot) ||
                        !hasInventoryItemForEquipment(registry, player, scanSlot)) {
                        return false;
                    }
                    scanTarget = directionalScanTarget(
                        registry, player, scanTarget, direction, equipmentRange(scanSlot));
                    if (scanTarget == MAX_ENTITIES) {
                        scanMode = ScanPanelMode::NONE;
                    }
                    return true;
                };
                if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                    moveScanCursor(Facing::UP);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                    moveScanCursor(Facing::DOWN);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    moveScanCursor(Facing::LEFT);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    moveScanCursor(Facing::RIGHT);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    EquipmentSlot slot = registry.has<EquipmentComponent>(player)
                        ? registry.get<EquipmentComponent>(player).equipped
                        : EquipmentSlot::NONE;
                    useEquipment(slot);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_E) {
                    interactPressed = true;
                } else if (hotkeyIndex >= 0) {
                    if (ctrlHeld && registry.has<EquipmentComponent>(player)) {
                        auto& equipment = registry.get<EquipmentComponent>(player);
                        assignEquipmentHotkey(equipment, static_cast<size_t>(hotkeyIndex), equipment.equipped);
                    } else {
                        EquipmentSlot slot = equipSlot(hotkeyIndex);
                        if (isConsumableEquipment(slot)) {
                            selectEquipment(slot);
                            useEquipment(slot);
                        } else {
                            selectEquipment(slot);
                        }
                    }
                    if (hotkeyIndex == 0) {
                        equipSlot(0);
                        scanMode = ScanPanelMode::NONE;
                        scanTarget = MAX_ENTITIES;
                    }
                } else if (event.key.keysym.scancode == SDL_SCANCODE_I && shiftHeld) {
                    selectEquipment(EquipmentSlot::BIOLOGY_AUDIT);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_I) {
                    selectEquipment(EquipmentSlot::SURFACE_SCAN);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_C) {
                    selectEquipment(EquipmentSlot::COGNITIVE_PROFILE);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_F && !shiftHeld) {
                    selectEquipment(EquipmentSlot::FINANCIAL_FORENSICS);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_T) {
                    selectEquipment(EquipmentSlot::STRUCTURAL_ANALYSIS);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_O) {
                    layerOverlay = nextLayerOverlay(layerOverlay);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_LEFTBRACKET) {
                    if (registry.has<DiscreteInventoryComponent>(player))
                        selectInventorySlot(registry.get<DiscreteInventoryComponent>(player), -1);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHTBRACKET) {
                    if (registry.has<DiscreteInventoryComponent>(player))
                        selectInventorySlot(registry.get<DiscreteInventoryComponent>(player), 1);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_U) {
                    EquipmentSlot equippedTool = equipSelectedInventoryTool(registry, player);
                    if (equippedTool == EquipmentSlot::NONE) {
                        useDiscreteInventoryPressed = true;
                    }
                } else if (event.key.keysym.scancode == SDL_SCANCODE_G) {
                    dropDiscreteInventoryPressed = true;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_B) {
                    selectedBuyType = nextMarketTradeType(selectedBuyType);
                    tradePreview = previewPlayerMarketBuy(registry, player, selectedBuyType);
                    if (!tradePreview.available) feedback.push(FeedbackCue::DENIED);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                    if (registry.has<DiscreteInventoryComponent>(player)) {
                        selectNextMarketSellItem(registry.get<DiscreteInventoryComponent>(player));
                    }
                    tradePreview = previewPlayerMarketSell(registry, player);
                    if (!tradePreview.available) feedback.push(FeedbackCue::DENIED);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_Y) {
                    confirmTradePressed = true;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    scanMode = ScanPanelMode::NONE;
                    scanTarget = MAX_ENTITIES;
                    tradePreview = {};
                } else if (event.key.keysym.scancode == SDL_SCANCODE_F && shiftHeld) {
                    auto tv = registry.view<TimeOfDayComponent>();
                    for (Entity te : tv) {
                        auto& tod = registry.get<TimeOfDayComponent>(te);
                        tod.is_flooded = !tod.is_flooded;
                        emitFloodAlert(alertStack, tod.is_flooded, tod.game_hour);
                        feedback.push(FeedbackCue::WARNING);
                        std::printf("Flooding event toggled: %s\n", tod.is_flooded ? "ON" : "OFF");
                    }
                }
            }
        }

        if (pendingScanEquipment != EquipmentSlot::NONE) {
            ScanPanelMode pendingMode = scanModeForEquipment(pendingScanEquipment);
            scanMode = pendingMode;
            if (!scanTargetInRange(registry, player, scanTarget, equipmentRange(pendingScanEquipment))) {
                scanTarget = nearestScanTarget(registry, player, equipmentRange(pendingScanEquipment));
            }
            feedback.push(FeedbackCue::SCAN);
        }

        if (registry.has<EquipmentComponent>(player)) {
            EquipmentSlot activeSlot = registry.get<EquipmentComponent>(player).equipped;
            if (isScanEquipment(activeSlot) &&
                !scanTargetInRange(registry, player, scanTarget, equipmentRange(activeSlot))) {
                scanTarget = MAX_ENTITIES;
                scanMode = ScanPanelMode::NONE;
            }
        }

        // Extract camera position for this frame's spawn/despawn decisions
        float frameCamX = 0.0f, frameCamY = 0.0f;
        {
            auto cv = registry.view<CameraComponent>();
            for (Entity ce : cv) {
                auto& cam = registry.get<CameraComponent>(ce);
                frameCamX = cam.x; frameCamY = cam.y;
            }
        }

        // Read time-of-day state for this frame (tint + spawn rate)
        uint8_t todR = 40, todG = 42, todB = 45;
        float todSpawnMult = 1.0f;
        float todScale = 2.0f;
        float todHour = 8.0f;
        TimeOfDay todPhase = TimeOfDay::DAY;
        {
            auto tv = registry.view<TimeOfDayComponent>();
            for (Entity te : tv) {
                auto& tod = registry.get<TimeOfDayComponent>(te);
                todR = tod.tint_r; todG = tod.tint_g; todB = tod.tint_b;
                todSpawnMult = tod.spawn_multiplier;
                todScale = tod.time_scale;
                todHour = tod.game_hour;
                todPhase = tod.phase;
            }
        }

        // Ambient Spawning (camera-radius gated, time-of-day modulated)
        ambientTimer -= dt;
        if (ambientTimer <= 0.0f) {
            ambient.spawnVehicles(registry, frameCamX, frameCamY, 800.0f, todSpawnMult);
            ambient.spawnCitizens(registry, frameCamX, frameCamY, 600.0f, todSpawnMult);
            ambient.spawnItems(registry, frameCamX, frameCamY, 600.0f);
            ambientTimer = 2.0f;
        }

        coordinator.advance();

        const Uint8* state = SDL_GetKeyboardState(NULL);

        if (consumeInventoryPressed) {
            bool consumed = useFirstInventoryItemOfType(registry, player, consumeInventoryType);
            if (!consumed) {
                consumed = consumables.consumePlayerInventory(registry, player, consumeInventoryType);
            }
            feedback.push(consumed ? FeedbackCue::CONSUME : FeedbackCue::DENIED);
            if (consumed && registry.has<CognitiveComponent>(player)) {
                auto& cog = registry.get<CognitiveComponent>(player);
                MemoryEventType evt = (consumeInventoryType == ItemComponent::FOOD)
                    ? MemoryEventType::ATE_FOOD : MemoryEventType::DRANK_WATER;
                cog.record({evt, todHour, 0.4f, MAX_ENTITIES});
                cog.pleasure = std::min(1.0f, cog.pleasure + 0.3f);
            }
        }

        // Item pickup takes priority over vehicle boarding on interact
        const bool originalInteractPressed = interactPressed;
        Entity passengerBeforeInteract = MAX_ENTITIES;
        if (registry.has<PassengerComponent>(player)) {
            passengerBeforeInteract = registry.get<PassengerComponent>(player).vehicle;
        }
        if (interactPressed) {
            bool inVehicle = registry.has<PassengerComponent>(player) &&
                             registry.get<PassengerComponent>(player).vehicle != MAX_ENTITIES;
            if (!inVehicle) {
                auto pickup = collectNearestInventoryItem(registry, player);
                bool pickedUp = pickup.picked_up;
                ItemComponent::Type pickedType = pickup.type;
                if (pickedUp) {
                    feedback.push(FeedbackCue::PICKUP);
                    tradePreview = {};
                    interactPressed = false;
                    if (registry.has<CognitiveComponent>(player)) {
                        auto& cog = registry.get<CognitiveComponent>(player);
                        if (pickedType == ItemComponent::FOOD)
                            cog.record({MemoryEventType::SAW_FOOD, todHour, 0.2f, MAX_ENTITIES});
                    }
                }
            }
            
            // Building entry/exit takes next priority
            if (!inVehicle && interactPressed) {
                if (registry.has<InteriorComponent>(player)) {
                    registry.remove<InteriorComponent>(player);
                    interactPressed = false;
                } else {
                    auto& pt = registry.get<TransformComponent>(player);
                    auto buildings = registry.view<BuildingAtmosphereComponent, TransformComponent>();
                    for (Entity b : buildings) {
                        auto& bt = registry.get<TransformComponent>(b);
                        if (pt.x >= bt.x - bt.width/2.0f && pt.x <= bt.x + bt.width/2.0f &&
                            pt.y >= bt.y - bt.height/2.0f && pt.y <= bt.y + bt.height/2.0f) {
                            registry.assign<InteriorComponent>(player, b);
                            interactPressed = false;
                            break;
                        }
                    }
                }
            }
        }

        float playerBeforeX = 0.0f;
        float playerBeforeY = 0.0f;
        if (registry.has<TransformComponent>(player)) {
            auto& pt = registry.get<TransformComponent>(player);
            playerBeforeX = pt.x;
            playerBeforeY = pt.y;
        }
        playerInput.handleInput(registry, state, interactPressed);

        if (useDiscreteInventoryPressed) {
            bool used = useSelectedInventoryItem(registry, player);
            feedback.push(used ? FeedbackCue::CONSUME : FeedbackCue::DENIED);
            if (used) tradePreview = {};
        }

        if (dropDiscreteInventoryPressed) {
            Entity dropped = dropSelectedInventoryItem(registry, player);
            feedback.push(dropped != MAX_ENTITIES ? FeedbackCue::PICKUP : FeedbackCue::DENIED);
            if (dropped != MAX_ENTITIES) tradePreview = {};
        }

        if (confirmTradePressed) {
            bool traded = executePlayerMarketTrade(registry, player, tradePreview);
            feedback.push(traded ? FeedbackCue::PICKUP : FeedbackCue::DENIED);
            tradePreview = {};
        }

        Entity passengerAfterInteract = MAX_ENTITIES;
        if (registry.has<PassengerComponent>(player)) {
            passengerAfterInteract = registry.get<PassengerComponent>(player).vehicle;
        }
        if (originalInteractPressed && interactPressed && passengerAfterInteract == passengerBeforeInteract) {
            feedback.push(FeedbackCue::DENIED);
        }

        bool playerMoveBlockedCandidate = false;
        float expectedMoveDistance = 0.0f;
        if (registry.has<MovementComponent>(player)) {
            auto& pm = registry.get<MovementComponent>(player);
            expectedMoveDistance = std::sqrt(pm.vx * pm.vx + pm.vy * pm.vy) * dt;
            playerMoveBlockedCandidate = expectedMoveDistance > 0.1f;
        }

        // Update player walk animation
        {
            auto pv = registry.view<PlayerComponent, MovementComponent>();
            for (Entity e : pv) {
                auto& p = registry.get<PlayerComponent>(e);
                auto& m = registry.get<MovementComponent>(e);
                bool moving = (m.vx != 0.0f || m.vy != 0.0f);
                if (moving) {
                    p.anim_timer -= dt;
                    if (p.anim_timer <= 0.0f) {
                        p.anim_timer = 0.125f; // 8 FPS
                        int maxFrames = (p.facing == Facing::DOWN) ? 4 : 3;
                        p.anim_frame = (p.anim_frame + 1) % maxFrames;
                    }
                } else {
                    p.anim_frame = 0;
                    p.anim_timer = 0.0f;
                }
            }
        }

        // L0: every frame
        transitSystem.update(registry, dt);  // must run before MovementSystem
        mover.update(registry, dt, registry.get<WorldConfigComponent>(worldConfig));
        if (playerMoveBlockedCandidate && registry.has<TransformComponent>(player)) {
            auto& pt = registry.get<TransformComponent>(player);
            const float actualDx = pt.x - playerBeforeX;
            const float actualDy = pt.y - playerBeforeY;
            const float actualMoveDistance = std::sqrt(actualDx * actualDx + actualDy * actualDy);
            if (actualMoveDistance < expectedMoveDistance * 0.35f) {
                feedback.push(FeedbackCue::IMPACT);
            }
        }
        cameraSystem.update(registry, dt, state);

        // L0: traffic lights (low overhead, every frame is fine)
        tlSystem.update(registry, dt);

        // L1: traffic density propagation + intersection FIFO + time of day + biology at ~6 Hz
        if (coordinator.tick_l1()) {
            float playerHealthBefore = 0.0f;
            bool hadPlayerHealth = false;
            if (registry.has<BiologyComponent>(player)) {
                playerHealthBefore = registry.get<BiologyComponent>(player).health;
                hadPlayerHealth = true;
            }
            trafficDensity.update(registry);
            intersectionSystem.update(registry);
            timeOfDaySystem.update(registry, dt);
            violenceEvents.clear();
            auto& configRef = registry.get<WorldConfigComponent>(worldConfig);
            auto& tempGridRef = registry.get<TemperatureGridComponent>(worldConfig);
            auto& todRef = registry.get<TimeOfDayComponent>(worldConfig);
            biologySystem.update(registry, dt, todScale, configRef, tempGridRef, todRef, violenceEvents);
            if (hadPlayerHealth && registry.has<BiologyComponent>(player) &&
                registry.get<BiologyComponent>(player).health < playerHealthBefore - 0.01f) {
                feedback.push(FeedbackCue::HAZARD_DAMAGE);
            }
            pathogenSystem.update(registry, dt, todScale, coordinator.tick_l2(), todHour, &alertStack);
            injurySystem.update(registry, dt, todScale, coordinator.tick_l2());
            memoryFormation.update(registry, violenceEvents, todHour);
        }

        // L2: schedule → GDI (goal selection) → citizen AI (movement) → relationships → social hierarchy → consumables → cognitive
        if (coordinator.tick_l2()) {
            scheduleSystem.update(registry, todHour);
            gdiSystem.update(registry, dt, todScale);
            citizenAI.update(registry, dt);
            relationshipSystem.update(registry);
            rumorSystem.update(registry, todHour);
            socialHierarchySystem.update(registry);
            consumables.update(registry, todHour);
            cognitiveSystem.update(registry, dt, todScale);
            temperatureSystem.update(registry, dt, todScale);
            atmosphereSystem.update(registry, dt, todScale);
            powerGridSystem.update(registry, dt, todScale);
        }

        // L3: economic simulation + structural decay + debug HUD (~1 Hz)
        if (coordinator.tick_l3()) {
            auto& tod = registry.get<TimeOfDayComponent>(worldConfig);
            wageSystem.update(registry, todScale);
            marketSystem.update(registry, dt, todScale);
            structuralDecaySystem.update(registry, dt, todScale, &alertStack);
            float fps = (dt > 0.0f) ? 1.0f / dt : 0.0f;
            float camX = 0.0f, camY = 0.0f;
            auto camView = registry.view<CameraComponent>();
            for (Entity ce : camView) {
                auto& cam = registry.get<CameraComponent>(ce);
                camX = cam.x; camY = cam.y;
            }
            const char* phaseStr = "DAY";
            if (todPhase == TimeOfDay::DAWN) phaseStr = "DAWN";
            else if (todPhase == TimeOfDay::DUSK) phaseStr = "DUSK";
            else if (todPhase == TimeOfDay::NIGHT) phaseStr = "NIGHT";
            int gh = static_cast<int>(todHour);
            int gm = static_cast<int>((todHour - gh) * 60.0f);
            
            const char* weatherStr = weatherStateName(tod.weather);

            char title[160];
            std::snprintf(title, sizeof(title),
                "Neon Oubliette | FPS: %.0f | Entities: %zu | Cam: (%.0f, %.0f) | %02d:%02d %s [%s] | Frame: %llu",
                fps, registry.entity_count(), camX, camY,
                gh, gm, phaseStr, weatherStr,
                static_cast<unsigned long long>(coordinator.frame));
            SDL_SetWindowTitle(window, title);

            // Log temperature + weather stats
            auto& tempGrid = registry.get<TemperatureGridComponent>(worldConfig);
            if (!tempGrid.grid.empty()) {
                float minT = tempGrid.grid[0], maxT = tempGrid.grid[0], sumT = 0.0f;
                for (float t : tempGrid.grid) {
                    if (t < minT) minT = t;
                    if (t > maxT) maxT = t;
                    sumT += t;
                }
                auto& todRef = registry.get<TimeOfDayComponent>(worldConfig);
                auto& config = registry.get<WorldConfigComponent>(worldConfig);
                float centerTemp = TemperatureSystem::getTempAt(config, tempGrid, 0, 0);
                float edgeTemp = TemperatureSystem::getTempAt(config, tempGrid, 900, 900);
                std::printf("ENV | %s | Center: %.1fC | Edge: %.1fC | Min: %.1fC | Max: %.1fC | Ambient: %.1fC\n",
                    weatherStr, centerTemp, edgeTemp, minT, maxT, todRef.ambient_target);
            }
        }

        {
            const auto& tod = registry.get<TimeOfDayComponent>(worldConfig);
            if (lastWeather != tod.weather) {
                feedback.push(FeedbackCue::WARNING);
            }
            emitWeatherAlertIfChanged(alertStack, lastWeather, tod.weather, tod.game_hour);
            lastWeather = tod.weather;
        }
        alertStack.update(dt);
        feedback.update(dt);

        // Get Camera
        float camX = 0.0f, camY = 0.0f, scale = 2.0f;
        float screenW = 800.0f, screenH = 600.0f;
        auto camView = registry.view<CameraComponent>();
        for (Entity e : camView) {
            auto& cam = registry.get<CameraComponent>(e);
            camX = cam.x;
            camY = cam.y;
            scale = cam.scale;
            screenW = cam.screenWidth;
            screenH = cam.screenHeight;
            ambient.despawnEntities(registry, cam.x, cam.y, 2000.0f);
        }
        
        float centerX = screenW / 2.0f;
        float centerY = screenH / 2.0f;
        centerX += static_cast<float>(feedback.shakeOffsetX());
        centerY += static_cast<float>(feedback.shakeOffsetY());

        // Render Background (tinted by time of day)
        SDL_SetRenderDrawColor(renderer, todR, todG, todB, 255);
        SDL_RenderClear(renderer);

        // Render Grid (Playable Area Visualizer)
        SDL_SetRenderDrawColor(renderer, 60, 62, 65, 255);
        int gridWU = 20;
        
        int startX = std::floor((camX - (centerX / scale)) / gridWU) * gridWU;
        int endX = std::ceil((camX + (centerX / scale)) / gridWU) * gridWU;
        
        int startY = std::floor((camY - (centerY / scale)) / gridWU) * gridWU;
        int endY = std::ceil((camY + (centerY / scale)) / gridWU) * gridWU;
        
        for (int x = startX; x <= endX; x += gridWU) {
            int screenX = static_cast<int>(centerX + (x - camX) * scale);
            SDL_RenderDrawLine(renderer, screenX, 0, screenX, static_cast<int>(screenH));
        }
        for (int y = startY; y <= endY; y += gridWU) {
            int screenY = static_cast<int>(centerY + (y - camY) * scale);
            SDL_RenderDrawLine(renderer, 0, screenY, static_cast<int>(screenW), screenY);
        }

        // Define screen bounds for culling
        float viewLeft = camX - (centerX / scale);
        float viewRight = camX + (centerX / scale);
        float viewTop = camY - (centerY / scale);
        float viewBottom = camY + (centerY / scale);

        // Render Roads (Pass 1: Ground-level roads, alleys, sidewalks)
        auto roadView = registry.view<TransformComponent, RoadComponent>();
        for (Entity e : roadView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            auto& r = registry.get<RoadComponent>(e);
            if (r.type == RoadType::MAGLIFT_TRACK) continue; // Render in Pass 2

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            if (r.type == RoadType::PRIMARY) {
                SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
                SDL_RenderFillRect(renderer, &destRect);
            } else if (r.type == RoadType::SECONDARY) {
                SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255);
                SDL_RenderFillRect(renderer, &destRect);
            } else if (r.type == RoadType::PEDESTRIAN_PATH) {
                SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
                int cornerRadius = static_cast<int>(5.0f * scale);
                if (cornerRadius * 2 > destRect.w || cornerRadius * 2 > destRect.h) {
                    SDL_RenderFillRect(renderer, &destRect);
                } else {
                    SDL_Rect r1 = { destRect.x + cornerRadius, destRect.y, destRect.w - 2 * cornerRadius, destRect.h };
                    SDL_Rect r2 = { destRect.x, destRect.y + cornerRadius, destRect.w, destRect.h - 2 * cornerRadius };
                    SDL_RenderFillRect(renderer, &r1);
                    SDL_RenderFillRect(renderer, &r2);
                    auto drawCircleFill = [&](int cx, int cy, int radius) {
                        for (int w = 0; w < radius * 2; w++) {
                            for (int h = 0; h < radius * 2; h++) {
                                int dx = radius - w;
                                int dy = radius - h;
                                if ((dx*dx + dy*dy) <= (radius * radius)) {
                                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                                }
                            }
                        }
                    };
                    drawCircleFill(destRect.x + cornerRadius, destRect.y + cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + destRect.w - cornerRadius, destRect.y + cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + cornerRadius, destRect.y + destRect.h - cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + destRect.w - cornerRadius, destRect.y + destRect.h - cornerRadius, cornerRadius);
                }
            } else if (r.type == RoadType::ALLEY) {
                SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
                SDL_RenderFillRect(renderer, &destRect);
            }
        }

        // Render Waiting Areas (ground-level transit stop pads — light teal)
        auto waitView = registry.view<TransformComponent, WaitingAreaComponent>();
        for (Entity e : waitView) {
            auto& t = registry.get<TransformComponent>(e);
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop  || t.y - t.height/2.0f > viewBottom) continue;
            SDL_Rect dr = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width  * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width  * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 30, 90, 100, 255); // teal fill
            SDL_RenderFillRect(renderer, &dr);
            SDL_SetRenderDrawColor(renderer, 80, 200, 210, 255); // cyan border
            SDL_RenderDrawRect(renderer, &dr);
        }

        // Render Staircases (amber/orange with horizontal step stripes)
        auto stairView = registry.view<TransformComponent, StaircaseComponent>();
        for (Entity e : stairView) {
            auto& t = registry.get<TransformComponent>(e);
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop  || t.y - t.height/2.0f > viewBottom) continue;
            SDL_Rect dr = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width  * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width  * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 110, 70, 20, 255); // amber fill
            SDL_RenderFillRect(renderer, &dr);
            // Horizontal step stripes
            SDL_SetRenderDrawColor(renderer, 180, 120, 40, 255);
            int steps = 4;
            for (int s = 1; s < steps; ++s) {
                int sy = dr.y + (dr.h * s) / steps;
                SDL_RenderDrawLine(renderer, dr.x, sy, dr.x + dr.w, sy);
            }
            SDL_SetRenderDrawColor(renderer, 220, 160, 60, 255); // bright border
            SDL_RenderDrawRect(renderer, &dr);
        }

        // === Ground-Level Glyph Pass ===
        // Renders all entities with GlyphComponent that are NOT MAGLIFT-type vehicles.
        {
            auto gv = registry.view<TransformComponent, GlyphComponent>();
            for (Entity e : gv) {
                // Elevated entities rendered after maglift track in pass 2
                if (registry.has<VehicleComponent>(e) &&
                    registry.get<VehicleComponent>(e).type == VehicleComponent::MAGLIFT) continue;
                // Skip player if riding a vehicle
                if (registry.has<PassengerComponent>(e) &&
                    registry.get<PassengerComponent>(e).vehicle != MAX_ENTITIES) continue;
                // Skip entities inside buildings
                if (registry.has<InteriorComponent>(e)) continue;
                auto& tf = registry.get<TransformComponent>(e);
                if (tf.x + tf.width/2.0f < viewLeft || tf.x - tf.width/2.0f > viewRight ||
                    tf.y + tf.height/2.0f < viewTop  || tf.y - tf.height/2.0f > viewBottom) continue;
                auto& g  = registry.get<GlyphComponent>(e);
                int sx   = static_cast<int>(centerX + (tf.x - camX) * scale);
                int sy   = static_cast<int>(centerY + (tf.y - camY) * scale);
                float rs = g.scale * scale;
                SDL_Color col = {g.r, g.g, g.b, g.a};
                if (registry.has<PathogenComponent>(e)) {
                    col = infectedGlyphColor(registry.get<PathogenComponent>(e));
                }

                // Apply visual decay if it's a structural entity
                if (registry.has<StructuralComponent>(e)) {
                    auto& s = registry.get<StructuralComponent>(e);
                    if (registry.has<CollapsedComponent>(e)) {
                        col = {40, 40, 45, 255}; // Dark grey/collapsed
                    } else if (s.integrity <= 40.0f) {
                        col = {110, 60, 50, 255}; // Damaged rust
                    } else if (s.integrity <= 80.0f) {
                        col = {100, 105, 110, 255}; // Worn grey
                    }
                }

                if (g.tiled && !g.chars.empty() && fontTex) {
                    int left_s = static_cast<int>(centerX + (tf.x - camX) * scale - (tf.width  * scale) / 2.0f);
                    int top_s  = static_cast<int>(centerY + (tf.y - camY) * scale - (tf.height * scale) / 2.0f);
                    int ew     = static_cast<int>(tf.width  * scale);
                    int eh     = static_cast<int>(tf.height * scale);
                    int tgw    = std::max(1, static_cast<int>(FONT_GLYPH_W * rs));
                    int tgh    = std::max(1, static_cast<int>(FONT_GLYPH_H * rs));
                    int idx    = static_cast<int>(static_cast<unsigned char>(g.chars[0])) - FONT_FIRST_CHAR;
                    if (idx >= 0 && idx < FONT_COLS * 6) {
                        SDL_Rect src = { (idx % FONT_COLS) * FONT_GLYPH_W,
                                         (idx / FONT_COLS) * FONT_GLYPH_H,
                                         FONT_GLYPH_W, FONT_GLYPH_H };
                        SDL_SetTextureColorMod(fontTex, col.r, col.g, col.b);
                        SDL_SetTextureAlphaMod(fontTex, col.a);
                        for (int gy = top_s; gy < top_s + eh; gy += tgh)
                            for (int gx = left_s; gx < left_s + ew; gx += tgw) {
                                SDL_Rect dst = { gx, gy, tgw, tgh };
                                SDL_RenderCopy(renderer, fontTex, &src, &dst);
                            }
                    }
                } else if (g.centered) {
                    drawTextCentered(renderer, fontTex, g.chars.c_str(), sx, sy, col, rs);
                } else {
                    drawText(renderer, fontTex, g.chars.c_str(), sx, sy, col, rs);
                }
            }
        }

        // Render Infrastructure (Pass 2: Maglift tracks, etc.)
        for (Entity e : roadView) {
            auto& t = registry.get<TransformComponent>(e);
            auto& r = registry.get<RoadComponent>(e);
            if (r.type != RoadType::MAGLIFT_TRACK) continue;

            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255); // Cyan track
            SDL_RenderFillRect(renderer, &destRect);
            // Draw some rails
            SDL_SetRenderDrawColor(renderer, 200, 240, 255, 255);
            SDL_Rect rail1 = { destRect.x + destRect.w / 4, destRect.y, 2, destRect.h };
            SDL_Rect rail2 = { destRect.x + 3 * destRect.w / 4, destRect.y, 2, destRect.h };
            SDL_RenderFillRect(renderer, &rail1);
            SDL_RenderFillRect(renderer, &rail2);
        }

        // === Elevated Glyph Pass ===
        // Renders MAGLIFT-type vehicles (transit cars + player's vehicle) above track.
        {
            auto gv = registry.view<TransformComponent, GlyphComponent>();
            for (Entity e : gv) {
                if (!registry.has<VehicleComponent>(e) ||
                    registry.get<VehicleComponent>(e).type != VehicleComponent::MAGLIFT) continue;
                auto& tf = registry.get<TransformComponent>(e);
                if (tf.x + tf.width/2.0f < viewLeft || tf.x - tf.width/2.0f > viewRight ||
                    tf.y + tf.height/2.0f < viewTop  || tf.y - tf.height/2.0f > viewBottom) continue;
                auto& g  = registry.get<GlyphComponent>(e);
                int sx   = static_cast<int>(centerX + (tf.x - camX) * scale);
                int sy   = static_cast<int>(centerY + (tf.y - camY) * scale);
                float rs = g.scale * scale;
                SDL_Color col = {g.r, g.g, g.b, g.a};
                if (registry.has<PathogenComponent>(e)) {
                    col = infectedGlyphColor(registry.get<PathogenComponent>(e));
                }
                if (g.tiled && !g.chars.empty() && fontTex) {
                    int left_s = static_cast<int>(centerX + (tf.x - camX) * scale - (tf.width  * scale) / 2.0f);
                    int top_s  = static_cast<int>(centerY + (tf.y - camY) * scale - (tf.height * scale) / 2.0f);
                    int ew     = static_cast<int>(tf.width  * scale);
                    int eh     = static_cast<int>(tf.height * scale);
                    int tgw    = std::max(1, static_cast<int>(FONT_GLYPH_W * rs));
                    int tgh    = std::max(1, static_cast<int>(FONT_GLYPH_H * rs));
                    int idx    = static_cast<int>(static_cast<unsigned char>(g.chars[0])) - FONT_FIRST_CHAR;
                    if (idx >= 0 && idx < FONT_COLS * 6) {
                        SDL_Rect src = { (idx % FONT_COLS) * FONT_GLYPH_W,
                                         (idx / FONT_COLS) * FONT_GLYPH_H,
                                         FONT_GLYPH_W, FONT_GLYPH_H };
                        SDL_SetTextureColorMod(fontTex, col.r, col.g, col.b);
                        SDL_SetTextureAlphaMod(fontTex, col.a);
                        for (int gy = top_s; gy < top_s + eh; gy += tgh)
                            for (int gx = left_s; gx < left_s + ew; gx += tgw) {
                                SDL_Rect dst = { gx, gy, tgw, tgh };
                                SDL_RenderCopy(renderer, fontTex, &src, &dst);
                            }
                    }
                } else if (g.centered) {
                    drawTextCentered(renderer, fontTex, g.chars.c_str(), sx, sy, col, rs);
                } else {
                    drawText(renderer, fontTex, g.chars.c_str(), sx, sy, col, rs);
                }
            }
        }

        // Render Traffic Lights (Elevated)
        auto tlView = registry.view<TransformComponent, TrafficLightComponent>();
        for (Entity e : tlView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            auto& tl = registry.get<TrafficLightComponent>(e);
            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            if (tl.state == TrafficLightComponent::GREEN) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            } else if (tl.state == TrafficLightComponent::YELLOW) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            SDL_RenderFillRect(renderer, &destRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &destRect);

            // Draw facing indicator
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            int indicatorSize = static_cast<int>(4.0f * scale);
            SDL_Rect indicatorRect = destRect;
            if (tl.facing == Facing::UP) {
                indicatorRect.h = indicatorSize;
            } else if (tl.facing == Facing::DOWN) {
                indicatorRect.y = destRect.y + destRect.h - indicatorSize;
                indicatorRect.h = indicatorSize;
            } else if (tl.facing == Facing::LEFT) {
                indicatorRect.w = indicatorSize;
            } else if (tl.facing == Facing::RIGHT) {
                indicatorRect.x = destRect.x + destRect.w - indicatorSize;
                indicatorRect.w = indicatorSize;
            }
            SDL_RenderFillRect(renderer, &indicatorRect);
        }

        drawLayerOverlay(renderer, fontTex, registry, layerOverlay, camX, camY,
                         centerX, centerY, scale, viewLeft, viewRight, viewTop, viewBottom);
        drawEquippedToolRange(renderer, registry, player, camX, camY, centerX, centerY, scale);

        // Scan target marker
        if (scanTarget != MAX_ENTITIES &&
            registry.alive(scanTarget) && registry.has<TransformComponent>(scanTarget)) {
            auto& st = registry.get<TransformComponent>(scanTarget);
            if (!(st.x + st.width/2.0f < viewLeft || st.x - st.width/2.0f > viewRight ||
                  st.y + st.height/2.0f < viewTop  || st.y - st.height/2.0f > viewBottom)) {
                SDL_Rect marker = {
                    static_cast<int>(centerX + (st.x - camX) * scale - (st.width  * scale) / 2.0f) - 3,
                    static_cast<int>(centerY + (st.y - camY) * scale - (st.height * scale) / 2.0f) - 3,
                    static_cast<int>(st.width  * scale) + 6,
                    static_cast<int>(st.height * scale) + 6
                };
                if (scanMode == ScanPanelMode::NONE) {
                    int corner = std::max(6, std::min(marker.w, marker.h) / 3);
                    SDL_SetRenderDrawColor(renderer, 245, 185, 70, 230);
                    SDL_RenderDrawLine(renderer, marker.x, marker.y, marker.x + corner, marker.y);
                    SDL_RenderDrawLine(renderer, marker.x, marker.y, marker.x, marker.y + corner);
                    SDL_RenderDrawLine(renderer, marker.x + marker.w, marker.y, marker.x + marker.w - corner, marker.y);
                    SDL_RenderDrawLine(renderer, marker.x + marker.w, marker.y, marker.x + marker.w, marker.y + corner);
                    SDL_RenderDrawLine(renderer, marker.x, marker.y + marker.h, marker.x + corner, marker.y + marker.h);
                    SDL_RenderDrawLine(renderer, marker.x, marker.y + marker.h, marker.x, marker.y + marker.h - corner);
                    SDL_RenderDrawLine(renderer, marker.x + marker.w, marker.y + marker.h, marker.x + marker.w - corner, marker.y + marker.h);
                    SDL_RenderDrawLine(renderer, marker.x + marker.w, marker.y + marker.h, marker.x + marker.w, marker.y + marker.h - corner);
                } else {
                    SDL_SetRenderDrawColor(renderer, 120, 245, 220, 240);
                    SDL_RenderDrawRect(renderer, &marker);
                    marker.x -= 2; marker.y -= 2; marker.w += 4; marker.h += 4;
                    SDL_SetRenderDrawColor(renderer, 20, 80, 75, 220);
                    SDL_RenderDrawRect(renderer, &marker);
                }
            }
        }

        if (feedback.flashActive()) {
            SDL_Color flash = feedback.currentFlashColor();
            SDL_SetRenderDrawColor(renderer, flash.r, flash.g, flash.b, flash.a);
            SDL_Rect flashRect = {0, 0, static_cast<int>(screenW), static_cast<int>(screenH)};
            SDL_RenderFillRect(renderer, &flashRect);
        }

        // On-screen debug HUD (every frame, top-left corner)
        if (fontTex) {
            SDL_Color hudCol = {120, 220, 120, 200};
            float hs = 0.75f;
            char hudBuf[160];
            float hfps = (dt > 0.0f) ? 1.0f / dt : 0.0f;
            std::snprintf(hudBuf, sizeof(hudBuf), "FPS:%.0f ENT:%zu INF:%zu",
                hfps, registry.entity_count(), PathogenSystem::infectedCount(registry));
            drawText(renderer, fontTex, hudBuf, 4, 4, hudCol, hs);
            int gh2 = static_cast<int>(todHour);
            int gm2 = static_cast<int>((todHour - gh2) * 60.0f);
            const char* ps2 = (todPhase == TimeOfDay::DAWN) ? "DAWN" :
                              (todPhase == TimeOfDay::DUSK) ? "DUSK" :
                              (todPhase == TimeOfDay::NIGHT) ? "NIGHT" : "DAY";
            std::snprintf(hudBuf, sizeof(hudBuf), "%02d:%02d %s", gh2, gm2, ps2);
            drawText(renderer, fontTex, hudBuf, 4, 16, hudCol, hs);
            std::snprintf(hudBuf, sizeof(hudBuf), "CAM:%.0f,%.0f Z:%.2f", camX, camY, scale);
            drawText(renderer, fontTex, hudBuf, 4, 28, hudCol, hs);
            std::snprintf(hudBuf, sizeof(hudBuf), "LAYER:%s", layerOverlayName(layerOverlay));
            drawText(renderer, fontTex, hudBuf, 4, 40, hudCol, hs);

            // Player bio stats
            auto pbv = registry.view<PlayerComponent, BiologyComponent>();
            for (Entity pe : pbv) {
                auto& bio = registry.get<BiologyComponent>(pe);
                SDL_Color bioCol = {120, 220, 120, 220}; // green
                if (bio.hunger < 20.0f || bio.thirst < 20.0f || bio.health < 20.0f)
                    bioCol = {220, 60, 60, 220};          // red
                else if (bio.hunger < 50.0f || bio.thirst < 50.0f || bio.fatigue < 50.0f)
                    bioCol = {220, 200, 60, 220};          // yellow
                std::snprintf(hudBuf, sizeof(hudBuf), "HP:%d HNG:%d THR:%d FAT:%d HR:%d O2:%d%%",
                    static_cast<int>(bio.health), static_cast<int>(bio.hunger),
                    static_cast<int>(bio.thirst), static_cast<int>(bio.fatigue),
                    static_cast<int>(bio.vitals.heart_rate), static_cast<int>(bio.vitals.oxygen_sat * 100.0f));
                drawText(renderer, fontTex, hudBuf, 4, 52, bioCol, hs);
            }

            auto invView = registry.view<PlayerComponent, SurvivalInventoryComponent>();
            for (Entity pe : invView) {
                auto& inv = registry.get<SurvivalInventoryComponent>(pe);
                SDL_Color invCol = {230, 230, 210, 220};
                std::snprintf(hudBuf, sizeof(hudBuf), "INV 1F:%d 2W:%d 3M:%d",
                    inv.food_count, inv.water_count, inv.medical_count);
                drawText(renderer, fontTex, hudBuf, 4, 64, invCol, hs);
            }

            auto equipView = registry.view<PlayerComponent, EquipmentComponent>();
            for (Entity pe : equipView) {
                auto& equipment = registry.get<EquipmentComponent>(pe);
                SDL_Color equipCol = {120, 245, 220, 220};
                std::snprintf(hudBuf, sizeof(hudBuf), "EQ:%s RNG:%d",
                    equipmentName(equipment.equipped),
                    static_cast<int>(equipmentRange(equipment.equipped)));
                drawText(renderer, fontTex, hudBuf, 4, 76, equipCol, hs);
            }

            auto discreteInvView = registry.view<PlayerComponent, DiscreteInventoryComponent>();
            for (Entity pe : discreteInvView) {
                auto& inventory = registry.get<DiscreteInventoryComponent>(pe);
                InventoryInspection inspected = inspectSelectedInventoryItem(inventory);
                SDL_Color invCol = {210, 180, 245, 220};
                if (inspected.present) {
                    EquipmentSlot inspectedEquipment = equipmentFromInventoryType(inspected.type);
                    if (isScanEquipment(inspectedEquipment)) {
                        std::snprintf(hudBuf, sizeof(hudBuf), "BAG %zu/%zu:%s RNG:%d%s",
                            inventory.selected + 1,
                            DiscreteInventoryComponent::CAPACITY,
                            itemTypeName(inspected.type),
                            static_cast<int>(equipmentRange(inspectedEquipment)),
                            itemProvenanceTracked(inspected.flags, inspected.provenance) ? " PROV" : "");
                    } else {
                        std::snprintf(hudBuf, sizeof(hudBuf), "BAG %zu/%zu:%s R:%d%s",
                            inventory.selected + 1,
                            DiscreteInventoryComponent::CAPACITY,
                            itemTypeName(inspected.type),
                            static_cast<int>(inspected.restore_value),
                            itemProvenanceTracked(inspected.flags, inspected.provenance) ? " PROV" : "");
                    }
                } else {
                    std::snprintf(hudBuf, sizeof(hudBuf), "BAG %zu/%zu:EMPTY",
                        inventory.selected + 1,
                        DiscreteInventoryComponent::CAPACITY);
                }
                drawText(renderer, fontTex, hudBuf, 4, 88, invCol, hs);
            }

            if (tradePreview.active) {
                SDL_Color tradeCol = tradePreview.available
                    ? SDL_Color{120, 245, 180, 230}
                    : SDL_Color{245, 100, 90, 230};
                std::snprintf(hudBuf, sizeof(hudBuf), "TRD %s %s P:%d B:%d G:%d R:%d CR:%d S:%d %s",
                    tradeModeName(tradePreview.mode),
                    itemTypeName(tradePreview.type),
                    static_cast<int>(tradePreview.price),
                    static_cast<int>(tradePreview.base_price),
                    static_cast<int>(tradePreview.greed_margin * 100.0f),
                    static_cast<int>(tradePreview.reputation * 100.0f),
                    static_cast<int>(tradePreview.credits),
                    static_cast<int>(tradePreview.stock),
                    tradePreview.reason);
                drawText(renderer, fontTex, hudBuf, 4, 100, tradeCol, hs);
            }

            auto visibleAlerts = alertStack.visible(4);
            int alertY = tradePreview.active ? 116 : 104;
            for (const auto& alert : visibleAlerts) {
                drawText(renderer, fontTex, alert.message.c_str(), 4, alertY,
                         alertColor(alert.severity), 0.65f);
                alertY += 12;
            }
        }

        drawScanPanel(renderer, fontTex, registry, scanTarget, player, scanMode,
                      static_cast<int>(screenW), static_cast<int>(screenH));

        SDL_RenderPresent(renderer);
    }

    if (fontTex) SDL_DestroyTexture(fontTex);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
