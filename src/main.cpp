#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "components.h"
#include "ecs.h"
#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "interior.h"
#include "save_state.h"
#include "stb_image.h"

static constexpr int FONT_GLYPH_W = 16;
static constexpr int FONT_GLYPH_H = 16;
static constexpr int FONT_COLS = 16;
static constexpr int FONT_FIRST_CHAR = 32;
static constexpr float BUILDING_INTERACTION_RANGE_WU = 18.0f;
static constexpr float INSPECTION_RANGE_WU = 22.0f;
static const char* TINY_SAVE_PATH = "neon_tiny_save.txt";

static SDL_Texture* loadFontTexture(SDL_Renderer* renderer, const char* path) {
    int w = 0, h = 0, ch = 0;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        std::cerr << "Failed to load font: " << path << std::endl;
        return nullptr;
    }

    for (int i = 0; i < w * h * 4; i += 4) {
        data[i] = 255 - data[i];
        data[i + 1] = 255 - data[i + 1];
        data[i + 2] = 255 - data[i + 2];
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        data, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surf) {
        stbi_image_free(data);
        return nullptr;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
    SDL_FreeSurface(surf);
    stbi_image_free(data);
    return tex;
}

static SDL_Texture* loadFontTextureFromKnownPaths(SDL_Renderer* renderer) {
    const char* paths[] = {
        "assets/om_large_plain_black_rgba.png",
        "../assets/om_large_plain_black_rgba.png",
        "../../assets/om_large_plain_black_rgba.png",
        "/Users/gm1/Code/neon/assets/om_large_plain_black_rgba.png"
    };

    for (const char* path : paths) {
        if (SDL_Texture* font = loadFontTexture(renderer, path)) {
            return font;
        }
    }

    std::cerr << "Font texture unavailable; using rectangle-only fallback rendering." << std::endl;
    return nullptr;
}

static bool glyphSourceRect(char c, SDL_Rect& src) {
    int idx = static_cast<int>(static_cast<unsigned char>(c)) - FONT_FIRST_CHAR;
    if (idx < 0 || idx >= FONT_COLS * 6) return false;

    src = {(idx % FONT_COLS) * FONT_GLYPH_W,
           (idx / FONT_COLS) * FONT_GLYPH_H,
           FONT_GLYPH_W,
           FONT_GLYPH_H};
    return true;
}

static void drawGlyph(SDL_Renderer* renderer, SDL_Texture* font, char c,
                      int x, int y, SDL_Color color, float scale) {
    if (!font) return;
    SDL_Rect src{};
    if (!glyphSourceRect(c, src)) return;

    SDL_Rect dst = {x, y,
                    static_cast<int>(FONT_GLYPH_W * scale),
                    static_cast<int>(FONT_GLYPH_H * scale)};
    SDL_SetTextureColorMod(font, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font, color.a);
    SDL_RenderCopy(renderer, font, &src, &dst);
}

static void drawGlyphToRect(SDL_Renderer* renderer, SDL_Texture* font, char c,
                            const SDL_Rect& dst, SDL_Color color) {
    if (!font || dst.w <= 0 || dst.h <= 0) return;
    SDL_Rect src{};
    if (!glyphSourceRect(c, src)) return;

    SDL_SetTextureColorMod(font, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font, color.a);
    SDL_RenderCopy(renderer, font, &src, &dst);
}

static void drawGlyphToRectRotated(SDL_Renderer* renderer, SDL_Texture* font, char c,
                                   const SDL_Rect& dst, SDL_Color color, double angle_degrees) {
    if (!font || dst.w <= 0 || dst.h <= 0) return;
    SDL_Rect src{};
    if (!glyphSourceRect(c, src)) return;

    SDL_SetTextureColorMod(font, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font, color.a);
    SDL_RenderCopyEx(renderer, font, &src, &dst, angle_degrees, nullptr, SDL_FLIP_NONE);
}

static void drawText(SDL_Renderer* renderer, SDL_Texture* font, const char* text,
                     int x, int y, SDL_Color color, float scale) {
    int gw = static_cast<int>(FONT_GLYPH_W * scale);
    for (int i = 0; text[i]; ++i) {
        drawGlyph(renderer, font, text[i], x + i * gw, y, color, scale);
    }
}

static void drawTextCentered(SDL_Renderer* renderer, SDL_Texture* font, const char* text,
                             int cx, int cy, SDL_Color color, float scale) {
    int len = static_cast<int>(std::strlen(text));
    int gw = static_cast<int>(FONT_GLYPH_W * scale);
    int gh = static_cast<int>(FONT_GLYPH_H * scale);
    drawText(renderer, font, text, cx - (len * gw) / 2, cy - gh / 2, color, scale);
}

static void updatePlayer(Registry& registry, Entity player, float dt, const uint8_t* keys) {
    if (!registry.has<PlayerComponent>(player) || !registry.has<TransformComponent>(player)) return;

    auto& player_component = registry.get<PlayerComponent>(player);
    float dx = 0.0f;
    float dy = 0.0f;

    if (keys[SDL_SCANCODE_W]) dy -= 1.0f;
    if (keys[SDL_SCANCODE_S]) dy += 1.0f;
    if (keys[SDL_SCANCODE_A]) dx -= 1.0f;
    if (keys[SDL_SCANCODE_D]) dx += 1.0f;

    if (dx == 0.0f && dy == 0.0f) return;

    if (std::fabs(dx) > std::fabs(dy)) {
        player_component.facing = dx < 0.0f ? Facing::LEFT : Facing::RIGHT;
    } else {
        player_component.facing = dy < 0.0f ? Facing::UP : Facing::DOWN;
    }

    float len = std::sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        auto& interaction = registry.get<BuildingInteractionComponent>(player);
        const auto layout = interiorLayoutForRole(interaction.building_role);
        interaction.interior_position = movedInteriorPosition(layout,
                                                              interaction.interior_position,
                                                              dx,
                                                              dy,
                                                              player_component.speed,
                                                              dt);
        return;
    }

    auto current = registry.get<TransformComponent>(player);
    TransformComponent proposed = current;
    proposed.x += dx * player_component.speed * dt;
    proposed.y += dy * player_component.speed * dt;

    if (!transformOverlapsSolid(registry, proposed, player)) {
        registry.get<TransformComponent>(player) = proposed;
    }
}

static void toggleBuildingInteraction(Registry& registry, Entity player, float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player)) {
        return;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (interaction.inside_building) {
        exitBuildingInterior(registry, player);
        return;
    }

    const Entity nearest = nearestInteractableBuildingInRange(registry,
        registry.get<TransformComponent>(player),
        range_wu);
    if (nearest != MAX_ENTITIES) {
        enterBuildingInterior(registry, player, nearest);
    }
}

static void performInspection(Registry& registry, Entity player, float range_wu) {
    if (!registry.alive(player) || !registry.has<InspectionComponent>(player) ||
        !registry.has<TransformComponent>(player)) {
        return;
    }

    auto& inspection = registry.get<InspectionComponent>(player);
    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    
    inspection.target_entity = target.entity;
    inspection.target_type = target.type;
    inspection.has_result = true;
}

static const char* locationStateName(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "OUTSIDE";
        case PlayerLocationState::NEAR_HOUSING: return "NEAR HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "INSIDE HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "NEAR WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "INSIDE WORKPLACE";
        case PlayerLocationState::NEAR_SUPPLY: return "NEAR SUPPLY";
        case PlayerLocationState::INSIDE_SUPPLY: return "INSIDE SUPPLY";
    }
    return "OUTSIDE";
}

static const char* locationPrompt(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "";
        case PlayerLocationState::NEAR_HOUSING: return "E ENTER HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "E EXIT HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "E ENTER WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "E EXIT WORKPLACE";
        case PlayerLocationState::NEAR_SUPPLY: return "E ENTER SUPPLY";
        case PlayerLocationState::INSIDE_SUPPLY: return "E EXIT SUPPLY";
    }
    return "";
}

static const char* inspectionTargetName(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::NONE: return "NO TARGET";
        case InspectionTargetType::HOUSING: return "HOUSING";
        case InspectionTargetType::WORKPLACE: return "WORKPLACE";
        case InspectionTargetType::SUPPLY: return "SUPPLY";
        case InspectionTargetType::MARKET: return "MARKET";
        case InspectionTargetType::CLINIC: return "CLINIC";
        case InspectionTargetType::PEDESTRIAN_PATH: return "PATH";
        case InspectionTargetType::ROUTE_SIGNPOST: return "SIGNPOST";
        case InspectionTargetType::WORKER: return "WORKER";
        case InspectionTargetType::HOUSING_INTERIOR: return "HOUSING INTERIOR";
        case InspectionTargetType::WORKPLACE_INTERIOR: return "WORKPLACE INTERIOR";
        case InspectionTargetType::SUPPLY_INTERIOR: return "SUPPLY INTERIOR";
        case InspectionTargetType::CARRYABLE_OBJECT: return "CARRYABLE OBJECT";
    }
    return "NO TARGET";
}

static const char* inspectionDetail(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::NONE:
            return "Nothing close enough to read.";
        case InspectionTargetType::HOUSING:
            return "Private shelter. Enterable. Solid exterior.";
        case InspectionTargetType::WORKPLACE:
            return "Work site. Enterable. Linked to housing.";
        case InspectionTargetType::SUPPLY:
            return "Supply cache. Visible stock point. Linked through workplace.";
        case InspectionTargetType::MARKET:
            return "Commercial site. Observation only in this pass.";
        case InspectionTargetType::CLINIC:
            return "Public clinic. Municipal authority. Observation only.";
        case InspectionTargetType::PEDESTRIAN_PATH:
            return "Foot path. Non-solid access between buildings.";
        case InspectionTargetType::ROUTE_SIGNPOST:
            return "Route marker. Inspectable path endpoint.";
        case InspectionTargetType::WORKER:
            return "Fixed worker. Path route. Count locked at one.";
        case InspectionTargetType::HOUSING_INTERIOR:
            return "SLEEPING MAT: Tattered synthetic weave.";
        case InspectionTargetType::WORKPLACE_INTERIOR:
            return "WORK BENCH: Heavy machinery array.";
        case InspectionTargetType::SUPPLY_INTERIOR:
            return "STOCK SHELF: Usable cache markers, no inventory system yet.";
        case InspectionTargetType::CARRYABLE_OBJECT:
            return "SUPPLY: Carryable object.";
    }
    return "";
}

static const char* inspectionDetail(Registry& registry, const InspectionComponent& inspection) {
    static std::string dynamic_detail;
    if (inspection.target_type == InspectionTargetType::PEDESTRIAN_PATH &&
        registry.alive(inspection.target_entity) &&
        registry.has<PathComponent>(inspection.target_entity)) {
        dynamic_detail = pathInspectionReadout(registry, inspection.target_entity);
        return dynamic_detail.c_str();
    }

    if ((inspection.target_type == InspectionTargetType::HOUSING ||
         inspection.target_type == InspectionTargetType::WORKPLACE ||
         inspection.target_type == InspectionTargetType::SUPPLY ||
         inspection.target_type == InspectionTargetType::MARKET ||
         inspection.target_type == InspectionTargetType::CLINIC) &&
        registry.alive(inspection.target_entity) &&
        registry.has<BuildingUseComponent>(inspection.target_entity)) {
        dynamic_detail = buildingInspectionReadout(registry, inspection.target_entity);
        return dynamic_detail.c_str();
    }
    if (inspection.target_type == InspectionTargetType::HOUSING_INTERIOR &&
        registry.alive(inspection.target_entity) &&
        registry.has<BuildingImprovementComponent>(inspection.target_entity)) {
        dynamic_detail = housingInteriorReadout(registry);
        return dynamic_detail.c_str();
    }
    if (inspection.target_type == InspectionTargetType::WORKPLACE_INTERIOR &&
        registry.alive(inspection.target_entity) &&
        registry.has<WorkplaceBenchComponent>(inspection.target_entity)) {
        dynamic_detail = workplaceBenchLoopReadout(registry);
        return dynamic_detail.c_str();
    }
    if (inspection.target_type == InspectionTargetType::WORKER &&
        registry.alive(inspection.target_entity) &&
        registry.has<FixedActorComponent>(inspection.target_entity)) {
        dynamic_detail = workerCarryReadout(registry, inspection.target_entity);
        return dynamic_detail.c_str();
    }
    if (inspection.target_type == InspectionTargetType::ROUTE_SIGNPOST &&
        registry.alive(inspection.target_entity) &&
        registry.has<RouteSignpostComponent>(inspection.target_entity)) {
        dynamic_detail = routeSignpostReadout(registry, inspection.target_entity);
        return dynamic_detail.c_str();
    }
    if (inspection.target_type == InspectionTargetType::CARRYABLE_OBJECT &&
        registry.alive(inspection.target_entity) &&
        registry.has<CarryableComponent>(inspection.target_entity)) {
        dynamic_detail = carryableObjectReadout(registry, inspection.target_entity);
        return dynamic_detail.c_str();
    }

    return inspectionDetail(inspection.target_type);
}

static void updateCamera(Registry& registry, Entity camera_entity, float screen_w, float screen_h) {
    if (!registry.has<CameraComponent>(camera_entity)) return;
    auto& camera = registry.get<CameraComponent>(camera_entity);
    camera.screenWidth = screen_w;
    camera.screenHeight = screen_h;

    if (camera.target_entity != MAX_ENTITIES &&
        registry.alive(camera.target_entity) &&
        registry.has<TransformComponent>(camera.target_entity)) {
        const auto& target = registry.get<TransformComponent>(camera.target_entity);
        camera.x = target.x;
        camera.y = target.y;
    }
}

static void renderWorld(SDL_Renderer* renderer, SDL_Texture* font, Registry& registry,
                        const CameraComponent& camera) {
    const float center_x = camera.screenWidth * 0.5f;
    const float center_y = camera.screenHeight * 0.5f;
    const float scale = camera.scale;
    const float view_left = camera.x - center_x / scale;
    const float view_right = camera.x + center_x / scale;
    const float view_top = camera.y - center_y / scale;
    const float view_bottom = camera.y + center_y / scale;

    auto view = registry.view<TransformComponent, GlyphComponent>();
    for (Entity e : view) {
        const auto& t = registry.get<TransformComponent>(e);
        if (t.x + t.width * 0.5f < view_left || t.x - t.width * 0.5f > view_right ||
            t.y + t.height * 0.5f < view_top || t.y - t.height * 0.5f > view_bottom) {
            continue;
        }

        const auto& g = registry.get<GlyphComponent>(e);
        SDL_Color color{g.r, g.g, g.b, g.a};
        SDL_Rect rect = {
            static_cast<int>(center_x + (t.x - t.width * 0.5f - camera.x) * scale),
            static_cast<int>(center_y + (t.y - t.height * 0.5f - camera.y) * scale),
            std::max(2, static_cast<int>(t.width * scale)),
            std::max(2, static_cast<int>(t.height * scale))
        };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        const bool lit_path = registry.has<PathStateComponent>(e) &&
                              registry.get<PathStateComponent>(e).state == PathState::LIT;
        if (lit_path) {
            SDL_SetRenderDrawColor(renderer, 245, 190, 80, 70);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 255, 220, 120, 230);
            if (rect.h >= rect.w) {
                const int x = rect.x + rect.w / 2;
                SDL_RenderDrawLine(renderer, x, rect.y, x, rect.y + rect.h);
            } else {
                const int y = rect.y + rect.h / 2;
                SDL_RenderDrawLine(renderer, rect.x, y, rect.x + rect.w, y);
            }
        }
        if (!font) {
            if (registry.has<SolidComponent>(e)) {
                SDL_SetRenderDrawColor(renderer, 26, 54, 78, 180);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else if (lit_path) {
                SDL_SetRenderDrawColor(renderer, 255, 220, 120, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else {
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 235);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 10, 12, 16, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
            continue;
        }

        if (g.tiled && !g.chars.empty()) {
            const float target_tile_w = std::max(4.0f, FONT_GLYPH_W * scale * g.scale);
            const float target_tile_h = std::max(4.0f, FONT_GLYPH_H * scale * g.scale);
            const int cols = std::max(1, static_cast<int>(std::round(rect.w / target_tile_w)));
            const int rows = std::max(1, static_cast<int>(std::round(rect.h / target_tile_h)));
            for (int row = 0; row < rows; ++row) {
                const int tile_y0 = rect.y + (rect.h * row) / rows;
                const int tile_y1 = rect.y + (rect.h * (row + 1)) / rows;
                for (int col = 0; col < cols; ++col) {
                    const int tile_x0 = rect.x + (rect.w * col) / cols;
                    const int tile_x1 = rect.x + (rect.w * (col + 1)) / cols;
                    SDL_Rect tile{tile_x0, tile_y0,
                                  std::max(1, tile_x1 - tile_x0),
                                  std::max(1, tile_y1 - tile_y0)};
                    drawGlyphToRect(renderer, font, g.chars[0], tile, color);
                }
            }
        } else if (!g.chars.empty()) {
            if (registry.has<RouteSignpostComponent>(e) && g.chars[0] == 'v') {
                drawGlyphToRectRotated(renderer, font, '^', rect, color, 180.0);
            } else {
                drawGlyphToRect(renderer, font, g.chars[0], rect, color);
            }
        }
    }
}

static SDL_Rect interiorLocalRect(const TransformComponent& t,
                                  float room_cx,
                                  float room_cy,
                                  float scale) {
    return SDL_Rect{
        static_cast<int>(room_cx + (t.x - t.width * 0.5f) * scale),
        static_cast<int>(room_cy + (t.y - t.height * 0.5f) * scale),
        std::max(2, static_cast<int>(t.width * scale)),
        std::max(2, static_cast<int>(t.height * scale))
    };
}

static void renderInterior(SDL_Renderer* renderer, SDL_Texture* font, Registry& registry,
                           Entity player, const CameraComponent& camera) {
    if (!registry.alive(player) || !registry.has<BuildingInteractionComponent>(player)) {
        return;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!interaction.inside_building) return;

    const InteriorLayout layout = interiorLayoutForRole(interaction.building_role);
    uint8_t role_r = 255;
    uint8_t role_g = 255;
    uint8_t role_b = 255;
    colorForRole(interaction.building_role, role_r, role_g, role_b);

    const float hud_top = 86.0f;
    const float usable_w = std::max(1.0f, camera.screenWidth - 120.0f);
    const float usable_h = std::max(1.0f, camera.screenHeight - hud_top - 50.0f);
    const float room_scale = std::max(1.0f, std::min(usable_w / layout.width,
                                                     usable_h / layout.height) * 0.78f);
    const float room_cx = camera.screenWidth * 0.5f;
    const float room_cy = hud_top + usable_h * 0.5f;

    const SDL_Rect room_rect{
        static_cast<int>(room_cx - layout.width * room_scale * 0.5f),
        static_cast<int>(room_cy - layout.height * room_scale * 0.5f),
        static_cast<int>(layout.width * room_scale),
        static_cast<int>(layout.height * room_scale)
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 14, 16, 22, 255);
    SDL_RenderFillRect(renderer, &room_rect);

    SDL_SetRenderDrawColor(renderer, 28, 36, 48, 190);
    const float grid_step = 12.0f * room_scale;
    for (float x = static_cast<float>(room_rect.x); x <= room_rect.x + room_rect.w; x += grid_step) {
        SDL_RenderDrawLine(renderer, static_cast<int>(x), room_rect.y,
                           static_cast<int>(x), room_rect.y + room_rect.h);
    }
    for (float y = static_cast<float>(room_rect.y); y <= room_rect.y + room_rect.h; y += grid_step) {
        SDL_RenderDrawLine(renderer, room_rect.x, static_cast<int>(y),
                           room_rect.x + room_rect.w, static_cast<int>(y));
    }

    SDL_SetRenderDrawColor(renderer, role_r, role_g, role_b, 255);
    SDL_RenderDrawRect(renderer, &room_rect);

    const bool housing = interaction.building_role == MicroZoneRole::HOUSING;
    const bool workplace = interaction.building_role == MicroZoneRole::WORKPLACE;
    const TransformComponent fixture = housing
        ? TransformComponent{-24.0f, -16.0f, 34.0f, 14.0f}
        : workplace
            ? TransformComponent{0.0f, -20.0f, 56.0f, 18.0f}
            : TransformComponent{0.0f, -18.0f, 48.0f, 16.0f};
    SDL_Rect fixture_rect = interiorLocalRect(fixture, room_cx, room_cy, room_scale);
    SDL_SetRenderDrawColor(renderer,
                           housing ? 64 : workplace ? 130 : 42,
                           housing ? 112 : workplace ? 94 : 120,
                           housing ? 180 : workplace ? 42 : 72,
                           210);
    SDL_RenderFillRect(renderer, &fixture_rect);
    SDL_SetRenderDrawColor(renderer, role_r, role_g, role_b, 230);
    SDL_RenderDrawRect(renderer, &fixture_rect);

    const SDL_Rect player_rect = interiorLocalRect(interaction.interior_position,
                                                   room_cx,
                                                   room_cy,
                                                   room_scale);
    if (font) {
        drawGlyphToRect(renderer, font, '@', player_rect,
                        SDL_Color{245, 245, 210, 255});
        const char* fixture_label = housing ? "MAT" : workplace ? "BENCH" : "STOCK";
        if (workplace && workplaceBenchOutputReady(registry)) {
            fixture_label = "READY";
        } else if (housing && buildingImproved(registry)) {
            fixture_label = "FIXED";
        }
        const float label_fit = static_cast<float>(fixture_rect.w) /
            (std::strlen(fixture_label) * FONT_GLYPH_W);
        const float label_scale = std::clamp(label_fit * 0.82f, 0.75f, 1.15f);
        drawTextCentered(renderer, font, fixture_label,
                         fixture_rect.x + fixture_rect.w / 2,
                         fixture_rect.y + fixture_rect.h / 2,
                         SDL_Color{245, 245, 210, 220},
                         label_scale);
    } else {
        SDL_SetRenderDrawColor(renderer, 245, 245, 210, 255);
        SDL_RenderFillRect(renderer, &player_rect);
    }
}

int main(int, char**) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Neon Oubliette", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* font = loadFontTextureFromKnownPaths(renderer);

    Registry registry;
    WorldConfig world_config = makeSandboxConfig();
    world_config.workplace_micro_zone_count = 1;
    world_config.workplace_building_count = 1;
    world_config.supply_micro_zone_count = 1;
    world_config.supply_building_count = 1;
    world_config.market_micro_zone_count = 1;
    world_config.market_building_count = 1;
    world_config.clinic_micro_zone_count = 1;
    world_config.clinic_building_count = 1;
    world_config.fixed_worker_count = 1;
    world_config.carryable_object_count = 1;
    buildWorld(registry, world_config);
    if (!validateWorld(registry, world_config)) {
        std::cerr << "Generated world validation failed during startup." << std::endl;
        if (font) SDL_DestroyTexture(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    deriveInfrastructure(registry, world_config);
    spawnFixedActors(registry, world_config);

    Entity player = registry.create();
    registry.assign<TransformComponent>(player, 0.0f, -115.0f, 12.0f, 12.0f);
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<InspectionComponent>(player);
    registry.assign<GlyphComponent>(player, std::string("@"),
        static_cast<uint8_t>(245), static_cast<uint8_t>(245), static_cast<uint8_t>(210),
        static_cast<uint8_t>(255), 1.0f, true, false);
    if (!validatePlayerSpawn(registry, player)) {
        std::cerr << "Player spawn validation failed during startup." << std::endl;
        if (font) SDL_DestroyTexture(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Entity camera_entity = registry.create();
    auto& camera = registry.assign<CameraComponent>(camera_entity);
    camera.target_entity = player;
    camera.scale = 2.0f;

    bool running = true;
    std::string save_status = startupSaveStatusLine(tinySaveFileExists(TINY_SAVE_PATH));
    uint32_t last_ticks = SDL_GetTicks();

    while (running) {
        uint32_t now = SDL_GetTicks();
        float dt = std::min(0.05f, (now - last_ticks) / 1000.0f);
        last_ticks = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F) {
                auto& player_comp = registry.get<PlayerComponent>(player);
                bool inside = registry.has<BuildingInteractionComponent>(player) && registry.get<BuildingInteractionComponent>(player).inside_building;

                if (player_comp.carried_object != MAX_ENTITIES) {
                    if (!inside && !transformOverlapsSolid(registry, registry.get<TransformComponent>(player))) {
                        registry.get<TransformComponent>(player_comp.carried_object) = registry.get<TransformComponent>(player);
                        player_comp.carried_object = MAX_ENTITIES;
                    }
                } else if (playerCanTakeWorkplaceOutput(registry, player)) {
                    takeWorkplaceOutput(registry, player);
                } else if (playerCanTakeSupplyObject(registry, player)) {
                    takeSupplyObjectFromInterior(registry, player);
                } else {
                    takeNearbyCarryableObject(registry, player, BUILDING_INTERACTION_RANGE_WU);
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_E) {
                bool inside = registry.has<BuildingInteractionComponent>(player) && registry.get<BuildingInteractionComponent>(player).inside_building;

                Entity near_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), BUILDING_INTERACTION_RANGE_WU);

                if (playerCanImproveBuilding(registry, player)) {
                    improveBuilding(registry, player);
                } else if (playerCanStoreSupplyAtShelter(registry, player)) {
                    storeSupplyAtShelter(registry, player);
                } else if (playerCanStockWorkplaceBench(registry, player)) {
                    stockWorkplaceBench(registry, player);
                } else if (playerCanWorkWorkplaceBench(registry, player)) {
                    workWorkplaceBench(registry, player);
                } else if (near_worker != MAX_ENTITIES && !inside) {
                    auto& actor_comp = registry.get<FixedActorComponent>(near_worker);
                    actor_comp.acknowledged = !actor_comp.acknowledged;
                } else {
                    toggleBuildingInteraction(registry, player, BUILDING_INTERACTION_RANGE_WU);
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                performInspection(registry, player, INSPECTION_RANGE_WU);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_G) {
                if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
                    useInheritedGadgetSpoof(registry, player, INSPECTION_RANGE_WU);
                } else {
                    useInheritedGadget(registry, player, INSPECTION_RANGE_WU);
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F5) {
                const TinySaveStatus status = saveTinyStateToFile(registry, player, TINY_SAVE_PATH);
                save_status = saveResultStatusLine(status);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F9) {
                const TinySaveStatus status = loadTinyStateFromFile(registry, player, TINY_SAVE_PATH);
                std::string carried_label;
                const auto& loaded_player = registry.get<PlayerComponent>(player);
                if (loaded_player.carried_object != MAX_ENTITIES) {
                    carried_label = carryableObjectLabel(registry, loaded_player.carried_object);
                }
                save_status = loadResultStatusLine(
                    status,
                    playerLocationState(registry, player, BUILDING_INTERACTION_RANGE_WU),
                    carried_label);
            }
            if (event.type == SDL_MOUSEWHEEL && registry.has<CameraComponent>(camera_entity)) {
                auto& c = registry.get<CameraComponent>(camera_entity);
                c.scale = std::clamp(c.scale + event.wheel.y * 0.15f, 0.75f, 4.0f);
            }
        }

        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        updatePlayer(registry, player, dt, keys);
        updateFixedActors(registry, dt);
        updateWorkerSupplyPickups(registry);
        updateWorkerSupplyDeliveryRoutes(registry, dt);
        updateWorkerWorkplaceBenchDropOffs(registry);
        updateWorkerWorkplaceBenchWork(registry);
        updateWorkerWorkplaceOutputPickups(registry);
        updateWorkerFinishedItemDeliveryRoutes(registry, dt);
        updateWorkerBuildingDeliveries(registry);
        updateWorkerReturnRoutes(registry, dt);

        int screen_w = 0;
        int screen_h = 0;
        SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);
        updateCamera(registry, camera_entity, static_cast<float>(screen_w), static_cast<float>(screen_h));
        const auto& active_camera = registry.get<CameraComponent>(camera_entity);

        SDL_SetRenderDrawColor(renderer, 10, 12, 16, 255);
        SDL_RenderClear(renderer);

        const bool inside_render =
            registry.has<BuildingInteractionComponent>(player) &&
            registry.get<BuildingInteractionComponent>(player).inside_building;
        if (inside_render) {
            renderInterior(renderer, font, registry, player, active_camera);
        } else {
            renderWorld(renderer, font, registry, active_camera);
        }

        if (font) {
            SDL_Color hud{140, 230, 180, 230};
            char line[220];
            float fps = dt > 0.0f ? 1.0f / dt : 0.0f;
            std::snprintf(line, sizeof(line), "FPS:%03.0f ENT:%zu BASELINE:PLAYER + HOUSING + WORKPLACE + SUPPLY + MARKET + CLINIC + WORKER", fps, registry.entity_count());
            drawText(renderer, font, line, 6, 6, hud, 0.7f);
            std::snprintf(line, sizeof(line), "WASD MOVE  E ACT/DOORS  F PICK/DROP  SPACE INSPECT  G SCAN  SHIFT+G SPOOF  F5 SAVE  F9 LOAD  CAM:%.0f,%.0f Z:%.2f",
                          active_camera.x, active_camera.y, active_camera.scale);
            drawText(renderer, font, line, 6, 20, SDL_Color{110, 190, 230, 220}, 0.65f);
            const PlayerLocationState location_state =
                playerLocationState(registry, player, BUILDING_INTERACTION_RANGE_WU);
            Entity near_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), BUILDING_INTERACTION_RANGE_WU);
            Entity near_carryable = nearestCarryableObjectInRange(registry, registry.get<TransformComponent>(player), BUILDING_INTERACTION_RANGE_WU);
            bool inside = registry.has<BuildingInteractionComponent>(player) && registry.get<BuildingInteractionComponent>(player).inside_building;
            auto& player_comp = registry.get<PlayerComponent>(player);

            if (playerCanImproveBuilding(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  E IMPROVE BUILDING  %s  [CARRIED: %s]",
                              locationStateName(location_state),
                              buildingImprovementReadout(registry).c_str(),
                              carryableObjectLabel(registry, player_comp.carried_object));
            } else if (playerCanStoreSupplyAtShelter(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  E STORE SUPPLY  %s  [CARRIED: %s]",
                              locationStateName(location_state),
                              shelterSupplyReadout(registry).c_str(),
                              carryableObjectLabel(registry, player_comp.carried_object));
            } else if (playerCanStockWorkplaceBench(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  E STOCK BENCH  %s  [CARRIED: %s]",
                              locationStateName(location_state),
                              workplaceBenchReadout(registry).c_str(),
                              carryableObjectLabel(registry, player_comp.carried_object));
            } else if (playerCanWorkWorkplaceBench(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  E WORK BENCH  %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              workplaceBenchReadout(registry).c_str());
            } else if (player_comp.carried_object != MAX_ENTITIES) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s  F DROP %s  [CARRIED: %s]",
                              locationStateName(location_state),
                              locationPrompt(location_state),
                              carryableObjectLabel(registry, player_comp.carried_object),
                              carryableObjectLabel(registry, player_comp.carried_object));
            } else if (playerCanTakeSupplyObject(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s  F TAKE %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              locationPrompt(location_state),
                              itemKindDisplayName(ItemKind::SUPPLY));
            } else if (playerCanTakeWorkplaceOutput(registry, player)) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s  F TAKE %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              locationPrompt(location_state),
                              itemKindDisplayName(ItemKind::PART));
            } else if (near_carryable != MAX_ENTITIES && !inside) {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s  F PICK UP %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              locationPrompt(location_state),
                              carryableObjectLabel(registry, near_carryable));
            } else if (near_worker != MAX_ENTITIES && !inside) {
                bool ack = registry.get<FixedActorComponent>(near_worker).acknowledged;
                std::snprintf(line, sizeof(line), "LOCATION:%s  E %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              ack ? "DISMISS WORKER [WORKER ACKNOWLEDGED]" : "ACKNOWLEDGE WORKER");
            } else {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s  [CARRIED: NONE]",
                              locationStateName(location_state),
                              locationPrompt(location_state));
            }
            drawText(renderer, font, line, 6, 34, SDL_Color{245, 205, 120, 230}, 0.65f);
            std::snprintf(line, sizeof(line), "%s  %s  %s",
                          inheritedGadgetReadout(registry, player).c_str(),
                          inheritedGadgetPromptReadout(registry, player, INSPECTION_RANGE_WU).c_str(),
                          inheritedGadgetSpoofPromptReadout(registry, player, INSPECTION_RANGE_WU).c_str());
            drawText(renderer, font, line, 6, 48, SDL_Color{190, 205, 255, 225}, 0.65f);
            const bool can_inspect = playerInspectionTarget(registry, player, INSPECTION_RANGE_WU).entity != MAX_ENTITIES;
            std::snprintf(line, sizeof(line), "INSPECT:%s",
                          can_inspect ? "SPACE READ NEARBY" : "NO NEARBY TARGET");
            drawText(renderer, font, line, 6, 62, SDL_Color{180, 220, 190, 220}, 0.65f);
            if (registry.has<InspectionComponent>(player) &&
                registry.get<InspectionComponent>(player).has_result) {
                const auto& inspection = registry.get<InspectionComponent>(player);
                std::snprintf(line, sizeof(line), "READOUT:%s - %s",
                              inspectionTargetName(inspection.target_type),
                              inspectionDetail(registry, inspection));
                drawText(renderer, font, line, 6, 76, SDL_Color{245, 230, 150, 230}, 0.65f);
            }
            std::snprintf(line, sizeof(line), "STATE:%s", save_status.c_str());
            drawText(renderer, font, line, 6, 90, SDL_Color{150, 215, 245, 220}, 0.65f);
            if (firstShelterStockBuilding(registry) != MAX_ENTITIES) {
                drawText(renderer, font, housingInteriorReadout(registry).c_str(), 6, 104, SDL_Color{245, 215, 160, 220}, 0.65f);
            }
            if (firstWorkplaceBenchBuilding(registry) != MAX_ENTITIES) {
                drawText(renderer, font, workplaceBenchReadout(registry).c_str(), 6, 118, SDL_Color{245, 185, 120, 220}, 0.65f);
            }
            drawText(renderer, font, productionLoopSummaryReadout(registry).c_str(), 6, 132, SDL_Color{235, 170, 210, 220}, 0.65f);
            drawText(renderer, font, inheritedGadgetResultReadout(registry, player).c_str(), 6, 146, SDL_Color{205, 215, 255, 220}, 0.65f);
        }

        SDL_RenderPresent(renderer);
    }

    if (font) SDL_DestroyTexture(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
