#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <vector>
#include "components.h"
#include "ecs.h"
#include "world_config.h"
#include "world_generation.h"

inline int floorsForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return 4;
        case MicroZoneRole::WORKPLACE: return 3;
    }
    return 1;
}

inline char glyphForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return 'h';
        case MicroZoneRole::WORKPLACE: return 'w';
    }
    return '?';
}

inline void colorForRole(MicroZoneRole role, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (role) {
        case MicroZoneRole::HOUSING:
            r = 100;
            g = 170;
            b = 255;
            return;
        case MicroZoneRole::WORKPLACE:
            r = 245;
            g = 180;
            b = 90;
            return;
    }
    r = 255;
    g = 255;
    b = 255;
}

inline Entity buildBuildingUnit(Registry& registry,
                                MicroZoneRole role,
                                float x, float y, float w, float h) {
    Entity building = registry.create();
    registry.assign<TransformComponent>(building, x, y, w, h);
    registry.assign<SolidComponent>(building);

    uint64_t stable_id =
        (static_cast<uint64_t>(static_cast<int>(x) + 4096) << 32) ^
        static_cast<uint64_t>(static_cast<int>(y) + 4096) ^
        (static_cast<uint64_t>(static_cast<int>(role)) << 48);

    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    colorForRole(role, r, g, b);

    registry.assign<BuildingComponent>(building, stable_id, floorsForRole(role), true);
    registry.assign<BuildingUseComponent>(building, role);
    registry.assign<GlyphComponent>(building, std::string(1, glyphForRole(role)),
        r, g, b, static_cast<uint8_t>(255), 1.0f, true, true);
    return building;
}

inline void buildBuildingUnits(Registry& registry,
                               MicroZoneRole role,
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
        buildBuildingUnit(registry, role, cx, cy, building_w, building_h);
    }
}

inline Entity buildMicroZone(Registry& registry,
                             Entity macro,
                             MicroZoneRole role,
                             float x0, float y0, float x1, float y1,
                             int building_count) {
    Entity micro = registry.create();

    MicroZoneComponent component;
    component.parent_macro = macro;
    component.role = role;
    component.x0 = x0;
    component.y0 = y0;
    component.x1 = x1;
    component.y1 = y1;
    registry.assign<MicroZoneComponent>(micro, component);

    buildBuildingUnits(registry, role, x0, y0, x1, y1, building_count);
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

    const int housing_micro_count = std::max(0, config.housing_micro_zone_count);
    const int workplace_micro_count = std::max(0, config.workplace_micro_zone_count);
    const int total_micro_count = housing_micro_count + workplace_micro_count;
    const float inset = 40.0f;
    const float gap = 16.0f;
    const float content_x0 = x0 + inset;
    const float content_y0 = y0 + inset;
    const float content_x1 = x1 - inset;
    const float content_y1 = y1 - inset;
    const float total_h = std::max(0.0f, (content_y1 - content_y0) - gap * std::max(0, total_micro_count - 1));
    const float micro_h = total_micro_count > 0 ? total_h / static_cast<float>(total_micro_count) : 0.0f;

    int global_micro_index = 0;
    auto build_role_micros = [&](MicroZoneRole role, int micro_count, int building_count) {
        for (int i = 0; i < micro_count; ++i) {
            const int base = micro_count > 0 ? building_count / micro_count : 0;
            const int extra = i < (micro_count > 0 ? building_count % micro_count : 0) ? 1 : 0;
            const float my0 = content_y0 + global_micro_index * (micro_h + gap);
            const float my1 = my0 + micro_h;
            buildMicroZone(registry, macro, role, content_x0, my0, content_x1, my1, base + extra);
            ++global_micro_index;
        }
    };

    build_role_micros(MicroZoneRole::HOUSING,
                      housing_micro_count,
                      std::max(0, config.housing_building_count));
    build_role_micros(MicroZoneRole::WORKPLACE,
                      workplace_micro_count,
                      std::max(0, config.workplace_building_count));

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

inline Entity nearestBuildingInRange(Registry& registry,
                                     const TransformComponent& player_transform,
                                     float range_wu,
                                     MicroZoneRole role) {
    Entity nearest = MAX_ENTITIES;
    float nearest_distance = range_wu;
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role != role) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(building));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = building;
        }
    }
    return nearest;
}

inline Entity nearestInteractableBuildingInRange(Registry& registry,
                                                 const TransformComponent& player_transform,
                                                 float range_wu) {
    Entity nearest = MAX_ENTITIES;
    float nearest_distance = range_wu;
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        const MicroZoneRole role = registry.get<BuildingUseComponent>(building).role;
        if (role != MicroZoneRole::HOUSING && role != MicroZoneRole::WORKPLACE) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(building));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = building;
        }
    }
    return nearest;
}

inline Entity nearestHousingBuildingInRange(Registry& registry,
                                            const TransformComponent& player_transform,
                                            float range_wu) {
    return nearestBuildingInRange(registry, player_transform, range_wu, MicroZoneRole::HOUSING);
}

inline Entity nearestWorkplaceBuildingInRange(Registry& registry,
                                              const TransformComponent& player_transform,
                                              float range_wu) {
    return nearestBuildingInRange(registry, player_transform, range_wu, MicroZoneRole::WORKPLACE);
}

inline Entity nearestWorkerInRange(Registry& registry,
                                   const TransformComponent& player_transform,
                                   float range_wu) {
    Entity nearest = MAX_ENTITIES;
    float nearest_distance = range_wu;
    auto actors = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity actor : actors) {
        if (registry.get<FixedActorComponent>(actor).kind != FixedActorKind::WORKER) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(actor));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = actor;
        }
    }
    return nearest;
}

inline bool playerCanInteractWithHousing(Registry& registry,
                                         const TransformComponent& player_transform,
                                         float range_wu) {
    return nearestHousingBuildingInRange(registry, player_transform, range_wu) != MAX_ENTITIES;
}

inline bool playerCanInteractWithWorkplace(Registry& registry,
                                           const TransformComponent& player_transform,
                                           float range_wu) {
    return nearestWorkplaceBuildingInRange(registry, player_transform, range_wu) != MAX_ENTITIES;
}

inline InspectionTargetType inspectionTypeForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return InspectionTargetType::HOUSING;
        case MicroZoneRole::WORKPLACE: return InspectionTargetType::WORKPLACE;
    }
    return InspectionTargetType::NONE;
}

struct InspectionTarget {
    Entity entity = MAX_ENTITIES;
    InspectionTargetType type = InspectionTargetType::NONE;
};

inline InspectionTarget nearestInspectionTargetInRange(Registry& registry,
                                                       const TransformComponent& player_transform,
                                                       float range_wu) {
    InspectionTarget target;
    float nearest_distance = range_wu;

    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(building));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            target.entity = building;
            target.type = inspectionTypeForRole(registry.get<BuildingUseComponent>(building).role);
        }
    }

    auto paths = registry.view<PathComponent, TransformComponent>();
    for (Entity path : paths) {
        if (registry.get<PathComponent>(path).kind != PathKind::PEDESTRIAN) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(path));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            target.entity = path;
            target.type = InspectionTargetType::PEDESTRIAN_PATH;
        }
    }

    auto actors = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity actor : actors) {
        if (registry.get<FixedActorComponent>(actor).kind != FixedActorKind::WORKER) continue;

        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(actor));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            target.entity = actor;
            target.type = InspectionTargetType::WORKER;
        }
    }

    return target;
}

inline bool playerCanInspect(Registry& registry,
                             const TransformComponent& player_transform,
                             float range_wu) {
    return nearestInspectionTargetInRange(registry, player_transform, range_wu).entity != MAX_ENTITIES;
}

inline PlayerLocationState locationStateForRole(MicroZoneRole role, bool inside) {
    switch (role) {
        case MicroZoneRole::HOUSING:
            return inside ? PlayerLocationState::INSIDE_HOUSING : PlayerLocationState::NEAR_HOUSING;
        case MicroZoneRole::WORKPLACE:
            return inside ? PlayerLocationState::INSIDE_WORKPLACE : PlayerLocationState::NEAR_WORKPLACE;
    }
    return PlayerLocationState::OUTSIDE;
}

inline PlayerLocationState playerLocationState(Registry& registry,
                                               Entity player,
                                               float interaction_range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return PlayerLocationState::OUTSIDE;
    }

    if (registry.has<BuildingInteractionComponent>(player)) {
        const auto& interaction = registry.get<BuildingInteractionComponent>(player);
        if (interaction.inside_building) {
            return locationStateForRole(interaction.building_role, true);
        }
    }

    const Entity nearest = nearestInteractableBuildingInRange(registry,
        registry.get<TransformComponent>(player),
        interaction_range_wu);
    if (nearest == MAX_ENTITIES || !registry.has<BuildingUseComponent>(nearest)) {
        return PlayerLocationState::OUTSIDE;
    }

    return locationStateForRole(registry.get<BuildingUseComponent>(nearest).role, false);
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
        expected_macros * static_cast<size_t>(
            std::max(0, config.housing_micro_zone_count) +
            std::max(0, config.workplace_micro_zone_count));
    const size_t expected_buildings =
        expected_macros * static_cast<size_t>(
            std::max(0, config.housing_building_count) +
            std::max(0, config.workplace_building_count));

    if (macros.size() != expected_macros) return false;
    if (micros.size() != expected_micros) return false;
    if (buildings.size() != expected_buildings) return false;

    for (Entity macro : macros) {
        if (registry.get<MacroZoneComponent>(macro).type != config.macro_type) return false;
    }

    size_t housing_micros = 0;
    size_t workplace_micros = 0;
    for (Entity micro : micros) {
        const auto& micro_component = registry.get<MicroZoneComponent>(micro);
        if (micro_component.parent_macro == MAX_ENTITIES || !registry.alive(micro_component.parent_macro)) return false;
        switch (micro_component.role) {
            case MicroZoneRole::HOUSING:
                ++housing_micros;
                break;
            case MicroZoneRole::WORKPLACE:
                ++workplace_micros;
                break;
        }
    }

    if (housing_micros != expected_macros * static_cast<size_t>(std::max(0, config.housing_micro_zone_count))) return false;
    if (workplace_micros != expected_macros * static_cast<size_t>(std::max(0, config.workplace_micro_zone_count))) return false;

    size_t housing_buildings = 0;
    size_t workplace_buildings = 0;
    for (Entity building : buildings) {
        const auto& use = registry.get<BuildingUseComponent>(building);
        const auto& transform = registry.get<TransformComponent>(building);

        switch (use.role) {
            case MicroZoneRole::HOUSING:
                ++housing_buildings;
                break;
            case MicroZoneRole::WORKPLACE:
                ++workplace_buildings;
                break;
        }

        bool inside_matching_micro = false;
        for (Entity micro : micros) {
            const auto& micro_component = registry.get<MicroZoneComponent>(micro);
            if (micro_component.role == use.role && entityInsideMicroZone(transform, micro_component)) {
                inside_matching_micro = true;
                break;
            }
        }
        if (!inside_matching_micro) return false;
    }

    if (housing_buildings != expected_macros * static_cast<size_t>(std::max(0, config.housing_building_count))) return false;
    if (workplace_buildings != expected_macros * static_cast<size_t>(std::max(0, config.workplace_building_count))) return false;

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
