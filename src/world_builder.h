#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <vector>
#include "components.h"
#include "ecs.h"
#include "world_config.h"
#include "world_generation.h"

inline Entity buildHousingUnit(Registry& registry, float x, float y, float w, float h) {
    Entity building = registry.create();
    registry.assign<TransformComponent>(building, x, y, w, h);
    registry.assign<SolidComponent>(building);

    uint64_t stable_id =
        (static_cast<uint64_t>(static_cast<int>(x) + 4096) << 32) ^
        static_cast<uint64_t>(static_cast<int>(y) + 4096);

    registry.assign<BuildingComponent>(building, stable_id, 4, true);
    registry.assign<BuildingUseComponent>(building, MicroZoneRole::HOUSING);
    registry.assign<GlyphComponent>(building, std::string("h"),
        static_cast<uint8_t>(100), static_cast<uint8_t>(170), static_cast<uint8_t>(255),
        static_cast<uint8_t>(255), 1.0f, true, true);
    return building;
}

inline void buildHousingUnits(Registry& registry,
                              float x0, float y0, float x1, float y1,
                              int building_count) {
    if (building_count <= 0) return;

    const float margin = 32.0f;
    const float gap = 16.0f;
    const int cols = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<float>(building_count)))));
    const int rows = std::max(1, (building_count + cols - 1) / cols);
    const float usable_w = std::max(24.0f, (x1 - x0) - margin * 2.0f - gap * (cols - 1));
    const float usable_h = std::max(24.0f, (y1 - y0) - margin * 2.0f - gap * (rows - 1));
    const float building_w = std::max(24.0f, usable_w / static_cast<float>(cols));
    const float building_h = std::max(24.0f, usable_h / static_cast<float>(rows));

    for (int i = 0; i < building_count; ++i) {
        const int col = i % cols;
        const int row = i / cols;
        const float cx = x0 + margin + building_w * 0.5f +
                         col * (building_w + gap);
        const float cy = y0 + margin + building_h * 0.5f +
                         row * (building_h + gap);
        buildHousingUnit(registry, cx, cy, building_w, building_h);
    }
}

inline Entity buildHousingMicroZone(Registry& registry,
                                    Entity macro,
                                    float x0, float y0, float x1, float y1,
                                    int building_count) {
    Entity micro = registry.create();

    MicroZoneComponent component;
    component.parent_macro = macro;
    component.role = MicroZoneRole::HOUSING;
    component.x0 = x0;
    component.y0 = y0;
    component.x1 = x1;
    component.y1 = y1;
    registry.assign<MicroZoneComponent>(micro, component);

    buildHousingUnits(registry, x0, y0, x1, y1, building_count);
    return micro;
}

inline Entity buildMacroZone(Registry& registry,
                             float x0, float y0, float x1, float y1,
                             int grid_x, int grid_y,
                             uint32_t macro_id,
                             const WorldConfig& config) {
    Entity macro = registry.create();

    MacroZoneComponent component;
    component.macro_id = macro_id;
    component.type = config.macro_type;
    component.grid_x = grid_x;
    component.grid_y = grid_y;
    component.x0 = x0;
    component.y0 = y0;
    component.x1 = x1;
    component.y1 = y1;
    registry.assign<MacroZoneComponent>(macro, component);

    const int micro_count = std::max(0, config.housing_micro_zone_count);
    const int building_count = std::max(0, config.housing_building_count);
    const float inset = 40.0f;
    const float gap = 16.0f;
    const float content_x0 = x0 + inset;
    const float content_y0 = y0 + inset;
    const float content_x1 = x1 - inset;
    const float content_y1 = y1 - inset;
    const float total_h = std::max(0.0f, (content_y1 - content_y0) - gap * std::max(0, micro_count - 1));
    const float micro_h = micro_count > 0 ? total_h / static_cast<float>(micro_count) : 0.0f;

    for (int i = 0; i < micro_count; ++i) {
        const int base = micro_count > 0 ? building_count / micro_count : 0;
        const int extra = i < (micro_count > 0 ? building_count % micro_count : 0) ? 1 : 0;
        const float my0 = content_y0 + i * (micro_h + gap);
        const float my1 = my0 + micro_h;
        buildHousingMicroZone(registry, macro, content_x0, my0, content_x1, my1, base + extra);
    }

    return macro;
}

inline std::vector<Entity> buildWorld(Registry& registry, const WorldConfig& config) {
    (void)config.seed;
    std::vector<Entity> macros;
    const int macro_count_x = std::max(0, config.macro_count_x);
    const int macro_count_y = std::max(0, config.macro_count_y);
    macros.reserve(static_cast<size_t>(macro_count_x * macro_count_y));

    uint32_t macro_id = 0;
    for (int y = 0; y < macro_count_y; ++y) {
        for (int x = 0; x < macro_count_x; ++x) {
            const float x0 = (x - macro_count_x * 0.5f) * MACRO_ZONE_SIZE_WU;
            const float y0 = (y - macro_count_y * 0.5f) * MACRO_ZONE_SIZE_WU;
            macros.push_back(buildMacroZone(registry,
                                            x0, y0,
                                            x0 + MACRO_ZONE_SIZE_WU,
                                            y0 + MACRO_ZONE_SIZE_WU,
                                            x, y, macro_id++,
                                            config));
        }
    }
    return macros;
}

inline std::vector<Entity> buildWorld(Registry& registry,
                                      int macro_count_x = MACRO_ZONE_COUNT_X,
                                      int macro_count_y = MACRO_ZONE_COUNT_Y,
                                      uint32_t seed = kDefaultWorldSeed) {
    WorldConfig config = makeSandboxConfig();
    config.macro_count_x = macro_count_x;
    config.macro_count_y = macro_count_y;
    config.seed = seed;
    return buildWorld(registry, config);
}

inline bool entityInsideMicroZone(const TransformComponent& transform,
                                  const MicroZoneComponent& micro) {
    return transform.x - transform.width * 0.5f >= micro.x0 &&
           transform.x + transform.width * 0.5f <= micro.x1 &&
           transform.y - transform.height * 0.5f >= micro.y0 &&
           transform.y + transform.height * 0.5f <= micro.y1;
}

inline float aabbDistance(const TransformComponent& a, const TransformComponent& b) {
    const float dx = std::max(0.0f, std::fabs(a.x - b.x) - (a.width + b.width) * 0.5f);
    const float dy = std::max(0.0f, std::fabs(a.y - b.y) - (a.height + b.height) * 0.5f);
    return std::sqrt(dx * dx + dy * dy);
}

inline bool transformOverlapsSolid(Registry& registry,
                                   const TransformComponent& transform,
                                   Entity ignore = MAX_ENTITIES) {
    const AabbRect box = aabbFromTransform(transform);
    auto solids = registry.view<SolidComponent, TransformComponent>();
    for (Entity e : solids) {
        if (e == ignore) continue;
        if (!registry.get<SolidComponent>(e).is_solid) continue;
        if (aabbOverlap(box, aabbFromTransform(registry.get<TransformComponent>(e)))) return true;
    }
    return false;
}

inline bool playerSpawnOutsideSolids(Registry& registry, const TransformComponent& spawn) {
    return !transformOverlapsSolid(registry, spawn);
}

inline Entity nearestHousingBuildingInRange(Registry& registry,
                                            const TransformComponent& player_transform,
                                            float range_wu) {
    Entity nearest = MAX_ENTITIES;
    float nearest_distance = range_wu;
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role != MicroZoneRole::HOUSING) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(building));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = building;
        }
    }
    return nearest;
}

inline bool playerCanInteractWithHousing(Registry& registry,
                                         const TransformComponent& player_transform,
                                         float range_wu) {
    return nearestHousingBuildingInRange(registry, player_transform, range_wu) != MAX_ENTITIES;
}

inline PlayerLocationState playerLocationState(Registry& registry,
                                               Entity player,
                                               float housing_range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return PlayerLocationState::OUTSIDE;
    }

    if (registry.has<HousingInteractionComponent>(player) &&
        registry.get<HousingInteractionComponent>(player).inside_housing) {
        return PlayerLocationState::INSIDE_HOUSING;
    }

    return playerCanInteractWithHousing(registry,
                                        registry.get<TransformComponent>(player),
                                        housing_range_wu)
        ? PlayerLocationState::NEAR_HOUSING
        : PlayerLocationState::OUTSIDE;
}

inline bool buildingsDoNotOverlap(Registry& registry) {
    auto buildings = registry.view<BuildingComponent, TransformComponent>();
    for (size_t i = 0; i < buildings.size(); ++i) {
        for (size_t j = i + 1; j < buildings.size(); ++j) {
            if (aabbOverlap(aabbFromTransform(registry.get<TransformComponent>(buildings[i])),
                            aabbFromTransform(registry.get<TransformComponent>(buildings[j])))) {
                return false;
            }
        }
    }
    return true;
}

inline bool validateWorld(Registry& registry, const WorldConfig& config) {
    auto macros = registry.view<MacroZoneComponent>();
    auto micros = registry.view<MicroZoneComponent>();
    auto buildings = registry.view<BuildingComponent, TransformComponent, BuildingUseComponent>();

    const size_t expected_macros =
        static_cast<size_t>(std::max(0, config.macro_count_x) * std::max(0, config.macro_count_y));
    const size_t expected_micros =
        expected_macros * static_cast<size_t>(std::max(0, config.housing_micro_zone_count));
    const size_t expected_buildings =
        expected_macros * static_cast<size_t>(std::max(0, config.housing_building_count));

    if (macros.size() != expected_macros) return false;
    if (micros.size() != expected_micros) return false;
    if (buildings.size() != expected_buildings) return false;

    for (Entity macro : macros) {
        if (registry.get<MacroZoneComponent>(macro).type != config.macro_type) return false;
    }

    for (Entity micro : micros) {
        const auto& micro_component = registry.get<MicroZoneComponent>(micro);
        if (micro_component.parent_macro == MAX_ENTITIES || !registry.alive(micro_component.parent_macro)) return false;
        if (micro_component.role != MicroZoneRole::HOUSING) return false;
    }

    for (Entity building : buildings) {
        const auto& use = registry.get<BuildingUseComponent>(building);
        const auto& transform = registry.get<TransformComponent>(building);

        if (use.role != MicroZoneRole::HOUSING) return false;

        bool inside_any_micro = false;
        for (Entity micro : micros) {
            if (entityInsideMicroZone(transform, registry.get<MicroZoneComponent>(micro))) {
                inside_any_micro = true;
                break;
            }
        }
        if (!inside_any_micro) return false;
    }

    if (!buildingsDoNotOverlap(registry)) return false;

    return true;
}

inline bool validatePlayerSpawn(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<PlayerComponent>(player) ||
        !registry.has<TransformComponent>(player)) {
        return false;
    }
    return playerSpawnOutsideSolids(registry, registry.get<TransformComponent>(player));
}

inline bool validateWorld(Registry& registry) {
    return validateWorld(registry, makeSandboxConfig());
}
