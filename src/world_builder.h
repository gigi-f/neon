#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include "components.h"
#include "ecs.h"
#include "fixed_actor_system.h"
#include "world_config.h"
#include "world_generation.h"

inline int floorsForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return 4;
        case MicroZoneRole::WORKPLACE: return 3;
        case MicroZoneRole::SUPPLY: return 1;
        case MicroZoneRole::MARKET: return 2;
        case MicroZoneRole::CLINIC: return 2;
    }
    return 1;
}

inline char glyphForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return 'h';
        case MicroZoneRole::WORKPLACE: return 'w';
        case MicroZoneRole::SUPPLY: return 's';
        case MicroZoneRole::MARKET: return 'm';
        case MicroZoneRole::CLINIC: return '+';
    }
    return '?';
}

inline const char* roleDisplayName(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return "HOUSING";
        case MicroZoneRole::WORKPLACE: return "WORKPLACE";
        case MicroZoneRole::SUPPLY: return "SUPPLY";
        case MicroZoneRole::MARKET: return "MARKET";
        case MicroZoneRole::CLINIC: return "CLINIC";
    }
    return "UNKNOWN";
}

struct BuildingPurposeInfo {
    MicroZoneRole role = MicroZoneRole::HOUSING;
    const char* label = "DWELLING";
    const char* function = "BUILDING RECOVERY";
    const char* scan_detail = "HOUSING PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY";
};

inline BuildingPurposeInfo buildingPurposeForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING:
            return BuildingPurposeInfo{
                MicroZoneRole::HOUSING,
                "DWELLING",
                "BUILDING RECOVERY",
                "HOUSING PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY"
            };
        case MicroZoneRole::WORKPLACE:
            return BuildingPurposeInfo{
                MicroZoneRole::WORKPLACE,
                "PRODUCTION",
                "CONVERT SUPPLY TO PART",
                "WORKPLACE PURPOSE: PRODUCTION; FUNCTION: CONVERT SUPPLY TO PART"
            };
        case MicroZoneRole::SUPPLY:
            return BuildingPurposeInfo{
                MicroZoneRole::SUPPLY,
                "MATERIAL SOURCE",
                "SOURCE LOOP MATERIAL",
                "SUPPLY PURPOSE: MATERIAL SOURCE; FUNCTION: SOURCE LOOP MATERIAL"
            };
        case MicroZoneRole::MARKET:
            return BuildingPurposeInfo{
                MicroZoneRole::MARKET,
                "EXCHANGE",
                "EXCHANGE SITE",
                "MARKET PURPOSE: EXCHANGE; ACCESS: RESTRICTED"
            };
        case MicroZoneRole::CLINIC:
            return BuildingPurposeInfo{
                MicroZoneRole::CLINIC,
                "PUBLIC HEALTH",
                "MEDICAL SERVICE",
                "CLINIC PURPOSE: PUBLIC HEALTH; SERVICE: RATIONED; AUTHORITY: MUNICIPAL"
            };
    }
    return BuildingPurposeInfo{};
}

inline std::string buildingPurposeReadoutForRole(MicroZoneRole role) {
    const BuildingPurposeInfo purpose = buildingPurposeForRole(role);
    return std::string("PURPOSE: ") + purpose.label + "; FUNCTION: " + purpose.function;
}

inline std::string buildingPurposeScanReadoutForRole(MicroZoneRole role) {
    return buildingPurposeForRole(role).scan_detail;
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
        case MicroZoneRole::SUPPLY:
            r = 110;
            g = 230;
            b = 150;
            return;
        case MicroZoneRole::MARKET:
            r = 210;
            g = 120;
            b = 235;
            return;
        case MicroZoneRole::CLINIC:
            r = 255;
            g = 90;
            b = 90;
            return;
    }
    r = 255;
    g = 255;
    b = 255;
}

inline bool roleIsEnterable(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING:
        case MicroZoneRole::WORKPLACE:
        case MicroZoneRole::SUPPLY:
            return true;
        case MicroZoneRole::MARKET:
        case MicroZoneRole::CLINIC:
            return false;
    }
    return false;
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

    registry.assign<BuildingComponent>(building,
        stable_id,
        floorsForRole(role),
        roleIsEnterable(role));
    registry.assign<BuildingUseComponent>(building, role);
    if (role == MicroZoneRole::HOUSING) {
        registry.assign<ShelterStockComponent>(building);
        registry.assign<BuildingImprovementComponent>(building);
    } else if (role == MicroZoneRole::WORKPLACE) {
        registry.assign<WorkplaceBenchComponent>(building);
    }
    registry.assign<GlyphComponent>(building, std::string(1, glyphForRole(role)),
        r, g, b, static_cast<uint8_t>(255), 1.0f, true, true);
    return building;
}

inline Entity firstWorldBuilderBuildingByRole(Registry& registry, MicroZoneRole role) {
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role == role) {
            return building;
        }
    }
    return MAX_ENTITIES;
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
    const int supply_micro_count = std::max(0, config.supply_micro_zone_count);
    const int market_micro_count = std::max(0, config.market_micro_zone_count);
    const int clinic_micro_count = std::max(0, config.clinic_micro_zone_count);
    const int total_micro_count = housing_micro_count + workplace_micro_count + supply_micro_count +
        market_micro_count + clinic_micro_count;
    const float inset = 40.0f;
    const float gap = 16.0f;
    const float content_x0 = x0 + inset;
    const float content_y0 = y0 + inset;
    const float content_x1 = x1 - inset;
    const float content_y1 = y1 - inset;
    const int grid_cols = total_micro_count > 2
        ? std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<float>(total_micro_count)))))
        : 1;
    const int grid_rows = total_micro_count > 0
        ? std::max(1, (total_micro_count + grid_cols - 1) / grid_cols)
        : 0;
    const float total_w = std::max(0.0f, (content_x1 - content_x0) - gap * std::max(0, grid_cols - 1));
    const float total_h = std::max(0.0f, (content_y1 - content_y0) - gap * std::max(0, grid_rows - 1));
    const float micro_w = grid_cols > 0 ? total_w / static_cast<float>(grid_cols) : 0.0f;
    const float micro_h = grid_rows > 0 ? total_h / static_cast<float>(grid_rows) : 0.0f;

    int global_micro_index = 0;
    auto build_role_micros = [&](MicroZoneRole role, int micro_count, int building_count) {
        for (int i = 0; i < micro_count; ++i) {
            const int base = micro_count > 0 ? building_count / micro_count : 0;
            const int extra = i < (micro_count > 0 ? building_count % micro_count : 0) ? 1 : 0;
            int layout_index = global_micro_index;
            if (total_micro_count == 3 &&
                housing_micro_count == 1 &&
                workplace_micro_count == 1 &&
                supply_micro_count == 1) {
                if (role == MicroZoneRole::WORKPLACE) {
                    layout_index = 2;
                } else if (role == MicroZoneRole::SUPPLY) {
                    layout_index = 3;
                }
            } else if (total_micro_count == 4 &&
                       housing_micro_count == 1 &&
                       workplace_micro_count == 1 &&
                       supply_micro_count == 1 &&
                       market_micro_count == 1) {
                if (role == MicroZoneRole::MARKET) {
                    layout_index = 1;
                } else if (role == MicroZoneRole::WORKPLACE) {
                    layout_index = 2;
                } else if (role == MicroZoneRole::SUPPLY) {
                    layout_index = 3;
                }
            } else if (total_micro_count == 5 &&
                       housing_micro_count == 1 &&
                       workplace_micro_count == 1 &&
                       supply_micro_count == 1 &&
                       market_micro_count == 1 &&
                       clinic_micro_count == 1) {
                if (role == MicroZoneRole::WORKPLACE) {
                    layout_index = 1;
                } else if (role == MicroZoneRole::MARKET) {
                    layout_index = 2;
                } else if (role == MicroZoneRole::SUPPLY) {
                    layout_index = 4;
                } else if (role == MicroZoneRole::CLINIC) {
                    layout_index = 3;
                }
            }
            const int col = grid_cols > 0 ? layout_index % grid_cols : 0;
            const int row = grid_cols > 0 ? layout_index / grid_cols : 0;
            const float mx0 = content_x0 + col * (micro_w + gap);
            const float my0 = content_y0 + row * (micro_h + gap);
            const float mx1 = mx0 + micro_w;
            const float my1 = my0 + micro_h;
            buildMicroZone(registry, macro, role, mx0, my0, mx1, my1, base + extra);
            ++global_micro_index;
        }
    };

    build_role_micros(MicroZoneRole::HOUSING,
                      housing_micro_count,
                      std::max(0, config.housing_building_count));
    build_role_micros(MicroZoneRole::WORKPLACE,
                      workplace_micro_count,
                      std::max(0, config.workplace_building_count));
    build_role_micros(MicroZoneRole::SUPPLY,
                      supply_micro_count,
                      std::max(0, config.supply_building_count));
    build_role_micros(MicroZoneRole::MARKET,
                      market_micro_count,
                      std::max(0, config.market_building_count));
    build_role_micros(MicroZoneRole::CLINIC,
                      clinic_micro_count,
                      std::max(0, config.clinic_building_count));

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

    if (config.carryable_object_count > 0) {
        const Entity supply = firstWorldBuilderBuildingByRole(registry, MicroZoneRole::SUPPLY);
        TransformComponent supply_transform{};
        const bool seed_at_supply =
            supply != MAX_ENTITIES && registry.has<TransformComponent>(supply);
        if (seed_at_supply) {
            supply_transform = registry.get<TransformComponent>(supply);
        }
        for (int i = 0; i < config.carryable_object_count; ++i) {
            Entity item = registry.create();
            const float item_x = seed_at_supply ? supply_transform.x + i * 10.0f
                                                : -30.0f + i * 20.0f;
            const float item_y = seed_at_supply ? supply_transform.y : -115.0f;
            registry.assign<TransformComponent>(item, item_x, item_y, 8.0f, 8.0f);
            registry.assign<CarryableComponent>(item);
            registry.assign<GlyphComponent>(item, std::string("*"),
                static_cast<uint8_t>(200), static_cast<uint8_t>(200), static_cast<uint8_t>(200),
                static_cast<uint8_t>(255), 0.8f, true, false);
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

inline bool carryableObjectIsKind(Registry& registry, Entity object, ItemKind kind);

inline bool supplyObjectAtSupplyBuilding(Registry& registry, Entity object) {
    if (!registry.alive(object) ||
        !registry.has<CarryableComponent>(object) ||
        !registry.has<TransformComponent>(object) ||
        !carryableObjectIsKind(registry, object, ItemKind::SUPPLY)) {
        return false;
    }

    const Entity supply = firstWorldBuilderBuildingByRole(registry, MicroZoneRole::SUPPLY);
    if (supply == MAX_ENTITIES || !registry.has<TransformComponent>(supply)) {
        return false;
    }

    constexpr float supply_pickup_range_wu = 18.0f;
    return aabbDistance(registry.get<TransformComponent>(object),
                        registry.get<TransformComponent>(supply)) <= supply_pickup_range_wu;
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
        if (!registry.get<BuildingComponent>(building).is_enterable) continue;

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

inline Entity nearestCarryableObjectInRange(Registry& registry,
                                            const TransformComponent& player_transform,
                                            float range_wu) {
    Entity nearest = MAX_ENTITIES;
    float nearest_distance = range_wu;
    auto objects = registry.view<CarryableComponent, TransformComponent>();
    for (Entity obj : objects) {
        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(obj));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            nearest = obj;
        }
    }
    return nearest;
}

inline Entity firstCarryableObject(Registry& registry) {
    auto objects = registry.view<CarryableComponent, TransformComponent>();
    return objects.empty() ? MAX_ENTITIES : objects.front();
}

inline void hideCarryableObject(Registry& registry, Entity object) {
    auto& object_transform = registry.get<TransformComponent>(object);
    object_transform.x = 99999.0f;
    object_transform.y = 99999.0f;
}

inline bool playerInsideSupplyInterior(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<BuildingInteractionComponent>(player)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    return interaction.inside_building && interaction.building_role == MicroZoneRole::SUPPLY &&
           registry.alive(interaction.building_entity);
}

inline bool playerInsideHousingInterior(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<BuildingInteractionComponent>(player)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    return interaction.inside_building && interaction.building_role == MicroZoneRole::HOUSING &&
           registry.alive(interaction.building_entity);
}

inline bool playerInsideWorkplaceInterior(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<BuildingInteractionComponent>(player)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    return interaction.inside_building && interaction.building_role == MicroZoneRole::WORKPLACE &&
           registry.alive(interaction.building_entity);
}

inline bool carryableObjectIsHeld(Registry& registry, Entity object) {
    auto players = registry.view<PlayerComponent>();
    for (Entity player : players) {
        if (registry.get<PlayerComponent>(player).carried_object == object) {
            return true;
        }
    }
    auto actors = registry.view<FixedActorComponent>();
    for (Entity actor : actors) {
        if (registry.get<FixedActorComponent>(actor).carried_object == object) {
            return true;
        }
    }
    return false;
}

inline bool carryableObjectIsKind(Registry& registry, Entity object, ItemKind kind) {
    return registry.alive(object) &&
           registry.has<CarryableComponent>(object) &&
           registry.get<CarryableComponent>(object).kind == kind;
}

inline const char* carryableObjectLabel(Registry& registry, Entity object) {
    if (!registry.alive(object) || !registry.has<CarryableComponent>(object)) {
        return "UNKNOWN";
    }
    return itemKindDisplayName(registry.get<CarryableComponent>(object).kind);
}

inline std::string carryableObjectReadout(Registry& registry, Entity object) {
    return std::string(carryableObjectLabel(registry, object)) + ": Carryable object.";
}

inline bool playerInsideAnyBuilding(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<BuildingInteractionComponent>(player)) {
        return false;
    }
    return registry.get<BuildingInteractionComponent>(player).inside_building;
}

inline Entity firstShelterStockBuilding(Registry& registry) {
    auto shelters = registry.view<ShelterStockComponent, BuildingUseComponent>();
    for (Entity shelter : shelters) {
        if (registry.get<BuildingUseComponent>(shelter).role == MicroZoneRole::HOUSING) {
            return shelter;
        }
    }
    return MAX_ENTITIES;
}

inline int shelterSupplyCount(Registry& registry) {
    const Entity shelter = firstShelterStockBuilding(registry);
    if (shelter == MAX_ENTITIES) return 0;
    return registry.get<ShelterStockComponent>(shelter).current_supply;
}

inline int shelterSupplyCapacity(Registry& registry) {
    const Entity shelter = firstShelterStockBuilding(registry);
    if (shelter == MAX_ENTITIES) return 0;
    return registry.get<ShelterStockComponent>(shelter).capacity;
}

inline std::string buildingSupplyReadout(Registry& registry) {
    return "BUILDING SUPPLY: " + std::to_string(shelterSupplyCount(registry)) + "/" +
           std::to_string(shelterSupplyCapacity(registry));
}

inline std::string shelterSupplyReadout(Registry& registry) {
    return buildingSupplyReadout(registry);
}

inline bool shelterHasStoredSupply(Registry& registry) {
    return shelterSupplyCount(registry) > 0;
}

inline Entity firstWorkplaceBenchBuilding(Registry& registry) {
    auto benches = registry.view<WorkplaceBenchComponent, BuildingUseComponent>();
    for (Entity workplace : benches) {
        if (registry.get<BuildingUseComponent>(workplace).role == MicroZoneRole::WORKPLACE) {
            return workplace;
        }
    }
    return MAX_ENTITIES;
}

inline bool workplaceBenchStocked(Registry& registry) {
    const Entity workplace = firstWorkplaceBenchBuilding(registry);
    if (workplace == MAX_ENTITIES) return false;
    return registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::STOCKED;
}

inline bool workplaceBenchOutputReady(Registry& registry) {
    const Entity workplace = firstWorkplaceBenchBuilding(registry);
    if (workplace == MAX_ENTITIES) return false;
    return registry.get<WorkplaceBenchComponent>(workplace).state ==
           WorkplaceBenchState::OUTPUT_READY;
}

inline std::string workplaceBenchReadout(Registry& registry) {
    const Entity workplace = firstWorkplaceBenchBuilding(registry);
    if (workplace == MAX_ENTITIES) return "WORK BENCH: EMPTY";

    switch (registry.get<WorkplaceBenchComponent>(workplace).state) {
        case WorkplaceBenchState::EMPTY: return "WORK BENCH: EMPTY";
        case WorkplaceBenchState::STOCKED: return "WORK BENCH: STOCKED";
        case WorkplaceBenchState::OUTPUT_READY: return "WORK BENCH: OUTPUT READY";
    }
    return "WORK BENCH: EMPTY";
}

inline bool workplaceOutputBlockedByCarrier(Registry& registry) {
    if (!workplaceBenchOutputReady(registry)) {
        return false;
    }

    const Entity object = firstCarryableObject(registry);
    return object != MAX_ENTITIES && carryableObjectIsHeld(registry, object);
}

inline std::string workplaceBenchLoopReadout(Registry& registry) {
    const Entity workplace = firstWorkplaceBenchBuilding(registry);
    if (workplace == MAX_ENTITIES) {
        return "WORK BENCH: EMPTY; LOOP: NEEDS SUPPLY";
    }

    switch (registry.get<WorkplaceBenchComponent>(workplace).state) {
        case WorkplaceBenchState::EMPTY:
            return "WORK BENCH: EMPTY; LOOP: NEEDS SUPPLY";
        case WorkplaceBenchState::STOCKED:
            return "WORK BENCH: STOCKED; LOOP: BENCH OCCUPIED";
        case WorkplaceBenchState::OUTPUT_READY:
            if (workplaceOutputBlockedByCarrier(registry)) {
                return "WORK BENCH: OUTPUT WAITING; BLOCKED: BLOCKED BY CARRIER";
            }
            return "WORK BENCH: OUTPUT WAITING; LOOP: READY FOR PICKUP";
    }
    return "WORK BENCH: EMPTY; LOOP: NEEDS SUPPLY";
}

inline const char* workplaceBenchStateSaveName(WorkplaceBenchState state) {
    switch (state) {
        case WorkplaceBenchState::EMPTY: return "EMPTY";
        case WorkplaceBenchState::STOCKED: return "STOCKED";
        case WorkplaceBenchState::OUTPUT_READY: return "OUTPUT_READY";
    }
    return "EMPTY";
}

inline bool workplaceBenchStateFromSaveName(const std::string& name,
                                            WorkplaceBenchState& state) {
    if (name == "EMPTY") {
        state = WorkplaceBenchState::EMPTY;
        return true;
    }
    if (name == "STOCKED") {
        state = WorkplaceBenchState::STOCKED;
        return true;
    }
    if (name == "OUTPUT_READY") {
        state = WorkplaceBenchState::OUTPUT_READY;
        return true;
    }
    return false;
}

inline Entity firstBuildingImprovementBuilding(Registry& registry) {
    auto buildings = registry.view<BuildingImprovementComponent, BuildingUseComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role == MicroZoneRole::HOUSING) {
            return building;
        }
    }
    return MAX_ENTITIES;
}

inline bool buildingImproved(Registry& registry) {
    const Entity building = firstBuildingImprovementBuilding(registry);
    if (building == MAX_ENTITIES) return false;
    return registry.get<BuildingImprovementComponent>(building).improved;
}

inline std::string buildingImprovementReadout(Registry& registry) {
    return std::string("BUILDING IMPROVED: ") + (buildingImproved(registry) ? "YES" : "NO");
}

inline bool finishedPartInCirculation(Registry& registry) {
    auto objects = registry.view<CarryableComponent, TransformComponent>();
    for (Entity object : objects) {
        if (carryableObjectIsKind(registry, object, ItemKind::PART)) {
            const auto& transform = registry.get<TransformComponent>(object);
            if (carryableObjectIsHeld(registry, object) ||
                transform.x != 99999.0f ||
                transform.y != 99999.0f) {
                return true;
            }
        }
    }
    return false;
}

inline std::string buildingImprovementLoopReadout(Registry& registry) {
    if (!buildingImproved(registry)) {
        return "BUILDING: NEEDS PART; " + buildingImprovementReadout(registry);
    }
    if (finishedPartInCirculation(registry)) {
        return "BUILDING: ALREADY COMPLETE; " + buildingImprovementReadout(registry);
    }
    return "BUILDING: IMPROVED; " + buildingImprovementReadout(registry);
}

inline std::string housingInteriorReadout(Registry& registry) {
    return buildingPurposeReadoutForRole(MicroZoneRole::HOUSING) + "; " +
           buildingSupplyReadout(registry) + "; " +
           buildingImprovementLoopReadout(registry);
}

inline const char* siteContextTagForRole(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return "HOUSEHOLD";
        case MicroZoneRole::WORKPLACE: return "PRIVATE";
        case MicroZoneRole::SUPPLY: return "PRIVATE";
        case MicroZoneRole::MARKET: return "COMMERCIAL";
        case MicroZoneRole::CLINIC: return "MUNICIPAL";
    }
    return "UNKNOWN";
}

inline bool roleIsDependencyDependent(MicroZoneRole role,
                                      const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    return role == dependency.dependent_role;
}

inline bool roleIsDependencyProvider(MicroZoneRole role,
                                     const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    return role == dependency.provider_role;
}

inline bool roleInDependencyEdge(MicroZoneRole role,
                                 const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    return roleIsDependencyDependent(role, dependency) ||
           roleIsDependencyProvider(role, dependency);
}

inline Entity dependencyEndpointForRole(Registry& registry,
                                        MicroZoneRole role,
                                        const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    if (!roleInDependencyEdge(role, dependency)) {
        return MAX_ENTITIES;
    }
    return firstWorldBuilderBuildingByRole(registry, role);
}

inline bool dependencyEdgeResolved(Registry& registry,
                                   const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    return dependencyEndpointForRole(registry, dependency.dependent_role, dependency) != MAX_ENTITIES &&
           dependencyEndpointForRole(registry, dependency.provider_role, dependency) != MAX_ENTITIES;
}

inline Entity dependencyDisruptionStateEntity(Registry& registry,
                                              const DependencySpec& dependency = kWorkplaceDependsOnSupply,
                                              bool create_if_missing = false) {
    auto disruptions = registry.view<DependencyDisruptionComponent>();
    for (Entity state : disruptions) {
        const auto& component = registry.get<DependencyDisruptionComponent>(state);
        if (component.dependent_role == dependency.dependent_role &&
            component.provider_role == dependency.provider_role) {
            return state;
        }
    }

    if (!create_if_missing) {
        return MAX_ENTITIES;
    }

    Entity state = registry.create();
    registry.assign<DependencyDisruptionComponent>(state,
        dependency.dependent_role,
        dependency.provider_role,
        false,
        false);
    return state;
}

inline bool dependencyDisrupted(Registry& registry,
                                const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    const Entity state = dependencyDisruptionStateEntity(registry, dependency);
    return state != MAX_ENTITIES &&
           registry.get<DependencyDisruptionComponent>(state).disrupted;
}

inline bool dependencyRecovered(Registry& registry,
                                const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    const Entity state = dependencyDisruptionStateEntity(registry, dependency);
    return state != MAX_ENTITIES &&
           registry.get<DependencyDisruptionComponent>(state).recovered;
}

inline bool toggleDependencyDisruption(Registry& registry,
                                       const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    if (!dependencyEdgeResolved(registry, dependency)) {
        return false;
    }

    const Entity state = dependencyDisruptionStateEntity(registry, dependency, true);
    auto& component = registry.get<DependencyDisruptionComponent>(state);
    component.disrupted = !component.disrupted;
    component.recovered = !component.disrupted;
    return true;
}

inline std::string dependencyInspectionReadout(Registry& registry,
                                               MicroZoneRole role,
                                               const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    const bool disrupted = dependencyDisrupted(registry, dependency);
    const bool recovered = dependencyRecovered(registry, dependency);
    const std::string status = disrupted ? "; DEPENDENCY: DISRUPTED" :
        (recovered ? "; DEPENDENCY: RESTORED" : "");

    if (roleIsDependencyDependent(role, dependency)) {
        return std::string("DEPENDS ON: ") +
               (dependencyEndpointForRole(registry, dependency.provider_role, dependency) != MAX_ENTITIES
                    ? roleDisplayName(dependency.provider_role)
                    : std::string("MISSING ") + roleDisplayName(dependency.provider_role)) +
               status;
    }
    if (roleIsDependencyProvider(role, dependency)) {
        return std::string("SUPPORTS: ") +
               (dependencyEndpointForRole(registry, dependency.dependent_role, dependency) != MAX_ENTITIES
                    ? roleDisplayName(dependency.dependent_role)
                    : std::string("NO ") + roleDisplayName(dependency.dependent_role)) +
               status;
    }
    return "DEPENDENCY: NONE";
}

inline std::string dependencyScanReadout(Registry& registry,
                                         MicroZoneRole role,
                                         const DependencySpec& dependency = kWorkplaceDependsOnSupply) {
    if (!roleInDependencyEdge(role, dependency)) {
        return "";
    }

    std::string readout = std::string("FLOW: ") + dependency.flow_label +
        "; REQUIRED FOR: " + dependency.required_for;
    if (!dependencyEdgeResolved(registry, dependency)) {
        readout += "; TARGET: MISSING";
    } else if (dependencyDisrupted(registry, dependency)) {
        readout += "; DEPENDENCY: DISRUPTED; FLOW STATUS: CONFUSED";
    } else if (dependencyRecovered(registry, dependency)) {
        readout += "; DEPENDENCY: RESTORED; FLOW STATUS: CLEAR";
    } else if (roleIsDependencyProvider(role, dependency)) {
        readout += std::string("; SUPPORTS: ") + roleDisplayName(dependency.dependent_role);
    } else {
        readout += std::string("; SOURCE: ") + roleDisplayName(dependency.provider_role);
    }
    return readout;
}

inline bool pathConnectsReadoutRoles(Registry& registry,
                                     Entity path_entity,
                                     MicroZoneRole a,
                                     MicroZoneRole b) {
    if (!registry.alive(path_entity) || !registry.has<PathComponent>(path_entity)) {
        return false;
    }

    const auto& path = registry.get<PathComponent>(path_entity);
    if (path.kind != PathKind::PEDESTRIAN ||
        !registry.alive(path.from) ||
        !registry.alive(path.to) ||
        !registry.has<BuildingUseComponent>(path.from) ||
        !registry.has<BuildingUseComponent>(path.to)) {
        return false;
    }

    const MicroZoneRole from = registry.get<BuildingUseComponent>(path.from).role;
    const MicroZoneRole to = registry.get<BuildingUseComponent>(path.to).role;
    return (from == a && to == b) || (from == b && to == a);
}

inline bool routeFlowBlockedBySpoofedSignpost(Registry& registry, Entity path_entity) {
    return pathHasSpoofedRouteSignpost(registry, path_entity);
}

inline bool pathHasRecoveredRouteSignpost(Registry& registry, Entity path_entity) {
    if (!registry.alive(path_entity)) {
        return false;
    }
    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        const auto& signpost = registry.get<RouteSignpostComponent>(marker);
        if (signpost.path_entity == path_entity &&
            signpost.signal_recovered &&
            !signpost.spoofed) {
            return true;
        }
    }
    return false;
}

inline bool routeFlowRecoveredByRestoredSignpost(Registry& registry, Entity path_entity) {
    return !routeFlowBlockedBySpoofedSignpost(registry, path_entity) &&
           pathHasRecoveredRouteSignpost(registry, path_entity);
}

inline bool routeBetweenRolesHasSpoofedSignpost(Registry& registry,
                                                MicroZoneRole a,
                                                MicroZoneRole b) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        if (pathConnectsReadoutRoles(registry, path_entity, a, b) &&
            pathHasSpoofedRouteSignpost(registry, path_entity)) {
            return true;
        }
    }
    return false;
}

inline bool routeBetweenRolesHasRecoveredSignpost(Registry& registry,
                                                 MicroZoneRole a,
                                                 MicroZoneRole b) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        if (pathConnectsReadoutRoles(registry, path_entity, a, b) &&
            routeFlowRecoveredByRestoredSignpost(registry, path_entity)) {
            return true;
        }
    }
    return false;
}

inline bool workplaceSupplyFlowDisruptedBySpoofedRoute(Registry& registry) {
    return routeBetweenRolesHasSpoofedSignpost(registry,
                                               MicroZoneRole::WORKPLACE,
                                               MicroZoneRole::SUPPLY);
}

inline bool workplaceSupplyFlowRecoveredByRoute(Registry& registry) {
    return !workplaceSupplyFlowDisruptedBySpoofedRoute(registry) &&
           routeBetweenRolesHasRecoveredSignpost(registry,
                                                MicroZoneRole::WORKPLACE,
                                                MicroZoneRole::SUPPLY);
}

inline std::string buildingInspectionReadout(Registry& registry, Entity building) {
    if (!registry.alive(building) || !registry.has<BuildingUseComponent>(building)) {
        return "BUILDING; PURPOSE: UNKNOWN";
    }

    const MicroZoneRole role = registry.get<BuildingUseComponent>(building).role;
    std::string readout = std::string(roleDisplayName(role)) + "; " +
        buildingPurposeReadoutForRole(role);
    switch (role) {
        case MicroZoneRole::HOUSING:
            if (registry.has<BuildingImprovementComponent>(building)) {
                readout += "; " + buildingImprovementLoopReadout(registry);
            }
            break;
        case MicroZoneRole::WORKPLACE:
            if (registry.has<WorkplaceBenchComponent>(building)) {
                readout += "; " + workplaceBenchLoopReadout(registry);
            }
            if (workplaceSupplyFlowDisruptedBySpoofedRoute(registry)) {
                readout += "; SUPPLY FLOW: DISRUPTED";
            } else if (workplaceSupplyFlowRecoveredByRoute(registry)) {
                readout += "; SUPPLY FLOW: CLEAR";
            }
            break;
        case MicroZoneRole::SUPPLY:
            readout += "; SITE STATUS: STOCK POINT";
            break;
        case MicroZoneRole::MARKET:
            readout += "; SITE STATUS: OBSERVATION ONLY";
            break;
        case MicroZoneRole::CLINIC:
            readout += "; SITE STATUS: OBSERVATION ONLY";
            break;
    }
    if (roleInDependencyEdge(role)) {
        readout += "; " + dependencyInspectionReadout(registry, role);
    }
    readout += std::string("; CONTEXT: ") + siteContextTagForRole(role);
    return readout;
}

struct RoutePurposeInfo {
    const char* purpose = "PUBLIC ACCESS";
    const char* carries = "ACCESS";
    const char* expected_cargo = "PUBLIC";
    const char* access = "OPEN";
};

inline bool pathEndpointRoles(Registry& registry,
                              Entity path_entity,
                              MicroZoneRole& from_role,
                              MicroZoneRole& to_role) {
    if (!registry.alive(path_entity) || !registry.has<PathComponent>(path_entity)) {
        return false;
    }

    const auto& path = registry.get<PathComponent>(path_entity);
    if (!registry.alive(path.from) ||
        !registry.alive(path.to) ||
        !registry.has<BuildingUseComponent>(path.from) ||
        !registry.has<BuildingUseComponent>(path.to)) {
        return false;
    }

    from_role = registry.get<BuildingUseComponent>(path.from).role;
    to_role = registry.get<BuildingUseComponent>(path.to).role;
    return true;
}

inline RoutePurposeInfo routePurposeForRoles(MicroZoneRole a, MicroZoneRole b) {
    const bool housing_workplace =
        (a == MicroZoneRole::HOUSING && b == MicroZoneRole::WORKPLACE) ||
        (a == MicroZoneRole::WORKPLACE && b == MicroZoneRole::HOUSING);
    if (housing_workplace) {
        return RoutePurposeInfo{
            "LABOR ROUTE",
            "LABOR",
            "WORKER",
            "WORKER ROUTE"
        };
    }

    const bool workplace_supply =
        (a == MicroZoneRole::WORKPLACE && b == MicroZoneRole::SUPPLY) ||
        (a == MicroZoneRole::SUPPLY && b == MicroZoneRole::WORKPLACE);
    if (workplace_supply) {
        return RoutePurposeInfo{
            "SUPPLY ROUTE",
            "MATERIAL",
            "SUPPLY/PART",
            "WORKER ONLY"
        };
    }

    return RoutePurposeInfo{};
}

inline RoutePurposeInfo routePurposeForPath(Registry& registry, Entity path_entity) {
    MicroZoneRole from_role = MicroZoneRole::HOUSING;
    MicroZoneRole to_role = MicroZoneRole::HOUSING;
    if (!pathEndpointRoles(registry, path_entity, from_role, to_role)) {
        return RoutePurposeInfo{
            "UNKNOWN ROUTE",
            "UNKNOWN",
            "UNKNOWN",
            "UNKNOWN"
        };
    }
    return routePurposeForRoles(from_role, to_role);
}

inline std::string pathInspectionReadout(Registry& registry, Entity path_entity) {
    std::string readout = "Foot path. Non-solid access between buildings.";
    if (registry.alive(path_entity) && registry.has<PathStateComponent>(path_entity)) {
        switch (registry.get<PathStateComponent>(path_entity).state) {
            case PathState::LIT:
                readout = "Foot path. LIT: low amber markers make the route easier to follow.";
                break;
        }
    }

    const RoutePurposeInfo purpose = routePurposeForPath(registry, path_entity);
    readout += std::string(" ROUTE: ") + purpose.purpose +
        "; CARRIES: " + purpose.carries;
    if (routeFlowBlockedBySpoofedSignpost(registry, path_entity)) {
        readout += "; FLOW: BLOCKED";
    } else if (routeFlowRecoveredByRestoredSignpost(registry, path_entity)) {
        readout += "; FLOW: CLEAR";
    }
    return readout;
}

inline std::string routePurposeDebugReadout(Registry& registry,
                                            Entity path_entity,
                                            const char* prefix) {
    const RoutePurposeInfo purpose = routePurposeForPath(registry, path_entity);
    return std::string(prefix) + " ROUTE: " + purpose.purpose +
        "; EXPECTED CARGO: " + purpose.expected_cargo +
        "; ACCESS: " + purpose.access;
}

inline bool routeSignpostSpoofed(Registry& registry, Entity marker) {
    return registry.alive(marker) &&
           registry.has<RouteSignpostComponent>(marker) &&
           registry.get<RouteSignpostComponent>(marker).spoofed;
}

inline bool anyRouteSignpostSpoofed(Registry& registry) {
    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        if (registry.get<RouteSignpostComponent>(marker).spoofed) {
            return true;
        }
    }
    return false;
}

inline std::string routeSignpostReadout(Registry& registry, Entity marker) {
    if (!registry.alive(marker) || !registry.has<RouteSignpostComponent>(marker)) {
        return "TO UNKNOWN";
    }
    const auto& signpost = registry.get<RouteSignpostComponent>(marker);
    const RoutePurposeInfo purpose = routePurposeForPath(registry, signpost.path_entity);
    std::string readout = std::string("TO ") +
        roleDisplayName(signpost.target_role) +
        "; ROUTE: " +
        purpose.purpose +
        "; CARRIES: " +
        (signpost.spoofed ? "???" : purpose.carries);
    if (signpost.spoofed) {
        readout += "; FLOW: BLOCKED";
    } else if (signpost.signal_recovered) {
        readout += "; FLOW: CLEAR";
    }
    readout += "; SIGNAL: ";
    if (signpost.spoofed) {
        readout += "CORRUPTED; SPOOFED: ROUTE SIGNAL CONFUSED; CONSEQUENCE: ROUTE DELAY";
    } else if (signpost.signal_recovered) {
        readout += "CLEAR; RECOVERY: ROUTE SIGNAL CLEAR; CONSEQUENCE: NONE";
    } else {
        readout += "CLEAR; CONSEQUENCE: NONE";
    }
    return readout;
}

inline bool carryableObjectUnavailableFromSupply(Registry& registry) {
    return shelterHasStoredSupply(registry) ||
           workplaceBenchStocked(registry) ||
           workplaceBenchOutputReady(registry);
}

inline bool workerReturningToSupply(Registry& registry, Entity worker);
inline bool workerCarryingSupplyObject(Registry& registry, Entity worker);
inline bool workerCarryingPartObject(Registry& registry, Entity worker);
inline bool workerAtSupplyEndpoint(Registry& registry, Entity worker);
inline bool workerAtWorkplaceEndpoint(Registry& registry, Entity worker);
inline bool workerCanWorkWorkplaceBench(Registry& registry, Entity worker);
inline bool workerCanTakeWorkplaceOutput(Registry& registry, Entity worker);
inline Entity availableSupplyCarryableObject(Registry& registry);
inline constexpr float kLocalWitnessRangeWu = 22.0f;

inline bool playerCarryingItemKind(Registry& registry, ItemKind kind) {
    auto players = registry.view<PlayerComponent>();
    for (Entity player : players) {
        const Entity object = registry.get<PlayerComponent>(player).carried_object;
        if (object != MAX_ENTITIES && carryableObjectIsKind(registry, object, kind)) {
            return true;
        }
    }
    return false;
}

inline const char* workerLaborReasonTag(Registry& registry, Entity worker) {
    (void)registry;
    (void)worker;
    return "WAGE ROUTE";
}

inline const char* workerCarriedItemName(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return "NONE";
    }

    const Entity carried = registry.get<FixedActorComponent>(worker).carried_object;
    if (carried == MAX_ENTITIES) {
        return "NONE";
    }
    return carryableObjectLabel(registry, carried);
}

inline std::string workerRoutineState(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return "UNKNOWN";
    }
    if (buildingImproved(registry)) {
        return "DONE";
    }
    if (workerCarryingPartObject(registry, worker)) {
        return "DELIVERING PART";
    }
    if (workerCarryingSupplyObject(registry, worker)) {
        return "DELIVERING SUPPLY";
    }
    if (workerCanWorkWorkplaceBench(registry, worker)) {
        return "WORKING BENCH";
    }
    if (workerCanTakeWorkplaceOutput(registry, worker)) {
        return "TAKING PART";
    }
    if (workerReturningToSupply(registry, worker) || !workerAtSupplyEndpoint(registry, worker)) {
        return "GOING TO SUPPLY";
    }
    return "PICKING UP SUPPLY";
}

inline bool playerCarryingExpectedSupplyForWorker(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }
    if (buildingImproved(registry) ||
        workplaceBenchStocked(registry) ||
        workplaceBenchOutputReady(registry) ||
        workerCarryingSupplyObject(registry, worker) ||
        workerCarryingPartObject(registry, worker)) {
        return false;
    }

    const std::string routine = workerRoutineState(registry, worker);
    if (routine != "PICKING UP SUPPLY" && routine != "GOING TO SUPPLY") {
        return false;
    }

    return playerCarryingItemKind(registry, ItemKind::SUPPLY) &&
           availableSupplyCarryableObject(registry) == MAX_ENTITIES;
}

inline bool productionInterruptedByPlayer(Registry& registry) {
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (playerCarryingExpectedSupplyForWorker(registry, worker)) {
            return true;
        }
    }
    return false;
}

inline std::string productionConsequenceReadout(Registry& registry) {
    return productionInterruptedByPlayer(registry) ?
        "CONSEQUENCE: SHIFT STALLED" :
        "CONSEQUENCE: NONE";
}

inline std::string workerBlockedReason(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return "";
    }
    if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
        return "ROUTE SIGNAL CONFUSED";
    }
    if (workerCurrentPathHasDisruptedDependency(registry, worker)) {
        return "DEPENDENCY DISRUPTED";
    }
    if (buildingImproved(registry)) {
        return "BUILDING ALREADY IMPROVED";
    }
    if (playerCarryingExpectedSupplyForWorker(registry, worker)) {
        return "WAITING FOR SUPPLY";
    }
    if (workerCarryingSupplyObject(registry, worker) && workerAtWorkplaceEndpoint(registry, worker)) {
        if (workplaceBenchOutputReady(registry)) {
            return "OUTPUT WAITING";
        }
        if (workplaceBenchStocked(registry)) {
            return "BENCH OCCUPIED";
        }
    }
    if (workplaceOutputBlockedByCarrier(registry)) {
        return "OUTPUT WAITING";
    }
    if (workerAtSupplyEndpoint(registry, worker) &&
        !workerCarryingSupplyObject(registry, worker) &&
        !workerCarryingPartObject(registry, worker)) {
        if (workplaceBenchOutputReady(registry)) {
            return "OUTPUT WAITING";
        }
        if (workplaceBenchStocked(registry)) {
            return "BENCH OCCUPIED";
        }
        if (availableSupplyCarryableObject(registry) == MAX_ENTITIES) {
            return "NO SUPPLY";
        }
    }
    return "";
}

inline std::string workerConsequenceSourceReadout(Registry& registry, Entity worker) {
    if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
        return "SOURCE: CORRUPTED ROUTE SIGNAL";
    }
    if (workerCurrentPathHasDisruptedDependency(registry, worker)) {
        return "SOURCE: DISRUPTED DEPENDENCY";
    }
    return "";
}

inline std::string workerRouteConsequenceReadout(Registry& registry, Entity worker) {
    if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
        return "WAITING ON ROUTE SIGNAL";
    }
    if (workerCurrentPathHasDisruptedDependency(registry, worker)) {
        return "WAITING ON SUPPLY FLOW";
    }
    return "";
}

inline std::string workerCarryReadout(Registry& registry, Entity worker) {
    std::string readout = std::string("WORKER ROUTINE: ") +
        workerRoutineState(registry, worker) +
        "; REASON: " +
        workerLaborReasonTag(registry, worker) +
        "; CARRYING: " +
        workerCarriedItemName(registry, worker);
    if (workplaceBenchStocked(registry) || workplaceBenchOutputReady(registry)) {
        readout += "; " + workplaceBenchReadout(registry);
    }
    if (buildingImproved(registry)) {
        readout += "; " + buildingImprovementReadout(registry);
    }
    const std::string blocked_reason = workerBlockedReason(registry, worker);
    if (!blocked_reason.empty()) {
        readout += "; BLOCKED: " + blocked_reason;
    }
    const std::string consequence_source = workerConsequenceSourceReadout(registry, worker);
    if (!consequence_source.empty()) {
        readout += "; " + consequence_source;
    }
    const std::string route_consequence = workerRouteConsequenceReadout(registry, worker);
    if (!route_consequence.empty()) {
        readout += "; " + route_consequence;
    }
    if (playerCarryingExpectedSupplyForWorker(registry, worker)) {
        readout += "; " + productionConsequenceReadout(registry);
    }
    return readout;
}

inline std::string productionLoopSummaryReadout(Registry& registry) {
    if (dependencyDisrupted(registry)) {
        return "LOOP: DISRUPTED; INTERFERENCE: DEPENDENCY; CONSEQUENCE: SUPPLY FLOW CONFUSED";
    }
    if (anyRouteSignpostSpoofed(registry)) {
        return "LOOP: SPOOFED; INTERFERENCE: ROUTE; CONSEQUENCE: ROUTE SIGNAL CONFUSED";
    }
    if (buildingImproved(registry)) {
        return "LOOP: COMPLETE";
    }
    if (productionInterruptedByPlayer(registry)) {
        return "LOOP: BLOCKED; " + productionConsequenceReadout(registry);
    }

    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (!workerBlockedReason(registry, worker).empty()) {
            return "LOOP: BLOCKED";
        }
    }
    return "LOOP: RUNNING";
}

inline const char* localSuspicionCauseLabel(LocalSuspicionCause cause) {
    switch (cause) {
        case LocalSuspicionCause::MISSING_PART:
            return "MISSING PART";
        case LocalSuspicionCause::ROUTE_TAMPERING:
            return "ROUTE TAMPERING";
        case LocalSuspicionCause::NONE:
            return "NONE";
    }
    return "NONE";
}

inline void recordLocalSuspicion(Registry& registry,
                                 Entity worker,
                                 LocalSuspicionCause cause,
                                 Entity workplace,
                                 Entity target,
                                 Entity path) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker) ||
        cause == LocalSuspicionCause::NONE) {
        return;
    }

    if (!registry.has<LocalSuspicionComponent>(worker)) {
        registry.assign<LocalSuspicionComponent>(worker);
    }
    auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
    suspicion.active = true;
    suspicion.cause = cause;
    suspicion.workplace_entity = workplace;
    suspicion.target_entity = target;
    suspicion.path_entity = path;
}

inline Entity firstLocalSuspicionWorker(Registry& registry) {
    auto suspicions = registry.view<LocalSuspicionComponent>();
    for (Entity worker : suspicions) {
        const auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
        if (suspicion.active && suspicion.cause != LocalSuspicionCause::NONE) {
            return worker;
        }
    }
    return MAX_ENTITIES;
}

inline bool localSuspicionActive(Registry& registry) {
    return firstLocalSuspicionWorker(registry) != MAX_ENTITIES;
}

inline std::string localSuspicionHudReadout(Registry& registry) {
    const Entity worker = firstLocalSuspicionWorker(registry);
    if (worker == MAX_ENTITIES) {
        return "";
    }
    const auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
    return std::string("LOCAL NOTICE: WORKER SAW ") +
           localSuspicionCauseLabel(suspicion.cause);
}

inline bool workerNearTransform(Registry& registry,
                                Entity worker,
                                const TransformComponent& transform,
                                float range_wu) {
    return registry.alive(worker) &&
           registry.has<TransformComponent>(worker) &&
           aabbDistance(registry.get<TransformComponent>(worker), transform) <= range_wu;
}

inline Entity workerWitnessingWorkplaceOutputTake(Registry& registry,
                                                  Entity player,
                                                  Entity workplace,
                                                  float range_wu = kLocalWitnessRangeWu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        !registry.alive(workplace) || !registry.has<TransformComponent>(workplace)) {
        return MAX_ENTITIES;
    }

    const auto& player_transform = registry.get<TransformComponent>(player);
    const auto& workplace_transform = registry.get<TransformComponent>(workplace);
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (workerCanTakeWorkplaceOutput(registry, worker) ||
            workerNearTransform(registry, worker, player_transform, range_wu) ||
            workerNearTransform(registry, worker, workplace_transform, range_wu)) {
            return worker;
        }
    }
    return MAX_ENTITIES;
}

inline Entity workerWitnessingRouteTampering(Registry& registry,
                                             Entity signpost_entity,
                                             float range_wu = kLocalWitnessRangeWu) {
    if (!registry.alive(signpost_entity) ||
        !registry.has<RouteSignpostComponent>(signpost_entity) ||
        !registry.has<TransformComponent>(signpost_entity)) {
        return MAX_ENTITIES;
    }

    const auto& signpost = registry.get<RouteSignpostComponent>(signpost_entity);
    const auto& signpost_transform = registry.get<TransformComponent>(signpost_entity);
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        const auto& actor = registry.get<FixedActorComponent>(worker);
        if (actor.path_entity == signpost.path_entity &&
            workerNearTransform(registry, worker, signpost_transform, range_wu)) {
            return worker;
        }
    }
    return MAX_ENTITIES;
}

inline bool workerCarryingSupplyObject(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    const Entity object = registry.get<FixedActorComponent>(worker).carried_object;
    return object != MAX_ENTITIES &&
           registry.alive(object) &&
           carryableObjectIsKind(registry, object, ItemKind::SUPPLY);
}

inline bool workerCarryingPartObject(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    const Entity object = registry.get<FixedActorComponent>(worker).carried_object;
    return object != MAX_ENTITIES &&
           registry.alive(object) &&
           carryableObjectIsKind(registry, object, ItemKind::PART);
}

inline void refreshWorkerCarryVisual(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker) ||
        !registry.has<GlyphComponent>(worker)) {
        return;
    }

    auto& glyph = registry.get<GlyphComponent>(worker);
    if (workerCarryingSupplyObject(registry, worker)) {
        glyph.chars = "A";
        glyph.r = static_cast<uint8_t>(255);
        glyph.g = static_cast<uint8_t>(205);
        glyph.b = static_cast<uint8_t>(90);
        glyph.a = static_cast<uint8_t>(255);
        glyph.scale = 1.12f;
        return;
    }

    if (workerCarryingPartObject(registry, worker)) {
        glyph.chars = "P";
        glyph.r = static_cast<uint8_t>(170);
        glyph.g = static_cast<uint8_t>(220);
        glyph.b = static_cast<uint8_t>(255);
        glyph.a = static_cast<uint8_t>(255);
        glyph.scale = 1.12f;
        return;
    }

    glyph.chars = "a";
    glyph.r = static_cast<uint8_t>(245);
    glyph.g = static_cast<uint8_t>(235);
    glyph.b = static_cast<uint8_t>(130);
    glyph.a = static_cast<uint8_t>(255);
    glyph.scale = 1.0f;
}

inline Entity pathEndpointWithRole(Registry& registry, Entity path_entity, MicroZoneRole role) {
    if (!registry.alive(path_entity) || !registry.has<PathComponent>(path_entity)) {
        return MAX_ENTITIES;
    }

    const auto& path = registry.get<PathComponent>(path_entity);
    if (registry.alive(path.from) &&
        registry.has<BuildingUseComponent>(path.from) &&
        registry.get<BuildingUseComponent>(path.from).role == role) {
        return path.from;
    }
    if (registry.alive(path.to) &&
        registry.has<BuildingUseComponent>(path.to) &&
        registry.get<BuildingUseComponent>(path.to).role == role) {
        return path.to;
    }
    return MAX_ENTITIES;
}

inline bool workerAtEndpointRole(Registry& registry, Entity worker, MicroZoneRole role) {
    if (!registry.alive(worker) ||
        !registry.has<FixedActorComponent>(worker) ||
        !registry.has<TransformComponent>(worker)) {
        return false;
    }

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.kind != FixedActorKind::WORKER ||
        !registry.alive(worker_component.path_entity) ||
        !registry.has<PathComponent>(worker_component.path_entity)) {
        return false;
    }

    constexpr float endpoint_epsilon = 0.001f;
    const bool at_endpoint = worker_component.route_t <= endpoint_epsilon ||
                             worker_component.route_t >= 1.0f - endpoint_epsilon;
    if (!at_endpoint) {
        return false;
    }

    const Entity endpoint = pathEndpointAtRouteT(registry,
                                                 worker_component.path_entity,
                                                 worker_component.route_t);
    return endpoint != MAX_ENTITIES &&
           registry.has<BuildingUseComponent>(endpoint) &&
           registry.get<BuildingUseComponent>(endpoint).role == role;
}

inline bool workerAtSupplyEndpoint(Registry& registry, Entity worker) {
    return workerAtEndpointRole(registry, worker, MicroZoneRole::SUPPLY);
}

inline bool workerAtWorkplaceEndpoint(Registry& registry, Entity worker) {
    return workerAtEndpointRole(registry, worker, MicroZoneRole::WORKPLACE);
}

inline bool workerAtHousingEndpoint(Registry& registry, Entity worker) {
    return workerAtEndpointRole(registry, worker, MicroZoneRole::HOUSING);
}

inline Entity firstPedestrianPathBetweenRoles(Registry& registry,
                                              MicroZoneRole a,
                                              MicroZoneRole b) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        const auto& path = registry.get<PathComponent>(path_entity);
        if (path.kind != PathKind::PEDESTRIAN ||
            !registry.alive(path.from) ||
            !registry.alive(path.to) ||
            !registry.has<BuildingUseComponent>(path.from) ||
            !registry.has<BuildingUseComponent>(path.to)) {
            continue;
        }

        const MicroZoneRole from = registry.get<BuildingUseComponent>(path.from).role;
        const MicroZoneRole to = registry.get<BuildingUseComponent>(path.to).role;
        if ((from == a && to == b) || (from == b && to == a)) {
            return path_entity;
        }
    }
    return MAX_ENTITIES;
}

inline bool workerReturningToSupply(Registry& registry, Entity worker) {
    if (!registry.alive(worker) ||
        !registry.has<FixedActorComponent>(worker) ||
        !registry.has<TransformComponent>(worker) ||
        !workplaceBenchStocked(registry)) {
        return false;
    }

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.kind != FixedActorKind::WORKER ||
        worker_component.carried_object != MAX_ENTITIES ||
        !registry.alive(worker_component.path_entity) ||
        !registry.has<PathComponent>(worker_component.path_entity) ||
        !registry.has<TransformComponent>(worker_component.path_entity)) {
        return false;
    }

    const Entity workplace =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::WORKPLACE);
    const Entity supply =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::SUPPLY);
    if (workplace == MAX_ENTITIES || supply == MAX_ENTITIES) {
        return false;
    }
    if (dependencyDisrupted(registry)) {
        return false;
    }

    const float supply_t = routeTForPathEndpoint(registry, worker_component.path_entity, supply);
    return std::fabs(worker_component.route_t - supply_t) > 0.001f;
}

inline bool routeWorkerReturningToSupply(Registry& registry, Entity worker) {
    if (!workerReturningToSupply(registry, worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity supply =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::SUPPLY);
    if (supply == MAX_ENTITIES) {
        worker_component.direction = 0.0f;
        return false;
    }

    const float target_t =
        routeTForPathEndpoint(registry, worker_component.path_entity, supply);
    if (std::fabs(worker_component.route_t - target_t) <= 0.001f) {
        worker_component.route_t = target_t;
        worker_component.direction = 0.0f;
        registry.get<TransformComponent>(worker) =
            transformOnPath(registry.get<TransformComponent>(worker_component.path_entity),
                            worker_component.route_t);
        return true;
    }

    worker_component.direction = worker_component.route_t < target_t ? 1.0f : -1.0f;
    return true;
}

inline size_t updateWorkerReturnRoutes(Registry& registry, float dt) {
    size_t routed = 0;
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity worker : workers) {
        if (!routeWorkerReturningToSupply(registry, worker)) {
            continue;
        }

        auto& worker_component = registry.get<FixedActorComponent>(worker);
        if (worker_component.direction == 0.0f) {
            ++routed;
            continue;
        }
        if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
            ++routed;
            continue;
        }

        const auto& path_transform = registry.get<TransformComponent>(worker_component.path_entity);
        const float span = std::max(path_transform.width, path_transform.height);
        if (span <= 0.0f) {
            continue;
        }

        const Entity supply =
            pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::SUPPLY);
        const float target_t =
            routeTForPathEndpoint(registry, worker_component.path_entity, supply);
        worker_component.route_t += worker_component.direction *
                                    (worker_component.speed_wu / span) * dt;
        if (worker_component.direction > 0.0f) {
            worker_component.route_t = std::min(worker_component.route_t, target_t);
        } else {
            worker_component.route_t = std::max(worker_component.route_t, target_t);
        }

        registry.get<TransformComponent>(worker) =
            transformOnPath(path_transform, worker_component.route_t);
        if (std::fabs(worker_component.route_t - target_t) <= 0.001f) {
            worker_component.route_t = target_t;
            worker_component.direction = 0.0f;
            registry.get<TransformComponent>(worker) =
                transformOnPath(path_transform, worker_component.route_t);
        }
        ++routed;
    }
    return routed;
}

inline Entity availableSupplyCarryableObject(Registry& registry) {
    if (carryableObjectUnavailableFromSupply(registry)) {
        return MAX_ENTITIES;
    }

    auto objects = registry.view<CarryableComponent, TransformComponent>();
    for (Entity object : objects) {
        if (!carryableObjectIsHeld(registry, object) &&
            supplyObjectAtSupplyBuilding(registry, object)) {
            return object;
        }
    }
    return MAX_ENTITIES;
}

inline bool playerCanTakeSupplyObject(Registry& registry, Entity player) {
    if (!playerInsideSupplyInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    if (registry.get<PlayerComponent>(player).carried_object != MAX_ENTITIES) {
        return false;
    }

    return availableSupplyCarryableObject(registry) != MAX_ENTITIES;
}

inline bool workerCanTakeSupplyObject(Registry& registry, Entity worker) {
    if (!workerAtSupplyEndpoint(registry, worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    if (dependencyDisrupted(registry)) {
        return false;
    }

    if (registry.get<FixedActorComponent>(worker).carried_object != MAX_ENTITIES) {
        return false;
    }

    return availableSupplyCarryableObject(registry) != MAX_ENTITIES;
}

inline bool takeSupplyObjectForWorker(Registry& registry, Entity worker) {
    if (!workerCanTakeSupplyObject(registry, worker)) {
        return false;
    }

    const Entity object = availableSupplyCarryableObject(registry);
    if (object == MAX_ENTITIES) {
        return false;
    }

    registry.get<FixedActorComponent>(worker).carried_object = object;
    hideCarryableObject(registry, object);
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerSupplyPickups(Registry& registry) {
    size_t picked_up = 0;
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (takeSupplyObjectForWorker(registry, worker)) {
            ++picked_up;
        }
    }
    return picked_up;
}

inline bool routeWorkerCarryingSupplyTowardWorkplace(Registry& registry, Entity worker) {
    if (!workerCarryingSupplyObject(registry, worker) ||
        !registry.has<TransformComponent>(worker)) {
        refreshWorkerCarryVisual(registry, worker);
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.kind != FixedActorKind::WORKER ||
        !registry.alive(worker_component.path_entity) ||
        !registry.has<PathComponent>(worker_component.path_entity) ||
        !registry.has<TransformComponent>(worker_component.path_entity)) {
        refreshWorkerCarryVisual(registry, worker);
        return false;
    }

    const Entity workplace =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::WORKPLACE);
    const Entity supply =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::SUPPLY);
    if (workplace == MAX_ENTITIES || supply == MAX_ENTITIES) {
        refreshWorkerCarryVisual(registry, worker);
        return false;
    }
    if (dependencyDisrupted(registry)) {
        worker_component.direction = 0.0f;
        refreshWorkerCarryVisual(registry, worker);
        return true;
    }

    const float target_t =
        routeTForPathEndpoint(registry, worker_component.path_entity, workplace);
    constexpr float endpoint_epsilon = 0.001f;
    if (std::fabs(worker_component.route_t - target_t) <= endpoint_epsilon) {
        worker_component.route_t = target_t;
        worker_component.direction = 0.0f;
        registry.get<TransformComponent>(worker) =
            transformOnPath(registry.get<TransformComponent>(worker_component.path_entity),
                            worker_component.route_t);
        refreshWorkerCarryVisual(registry, worker);
        return true;
    }

    worker_component.direction = worker_component.route_t < target_t ? 1.0f : -1.0f;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerSupplyDeliveryRoutes(Registry& registry, float dt) {
    size_t routed = 0;
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity worker : workers) {
        if (!routeWorkerCarryingSupplyTowardWorkplace(registry, worker)) {
            continue;
        }

        auto& worker_component = registry.get<FixedActorComponent>(worker);
        if (worker_component.direction == 0.0f) {
            ++routed;
            continue;
        }
        if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
            ++routed;
            continue;
        }

        const auto& path_transform = registry.get<TransformComponent>(worker_component.path_entity);
        const float span = std::max(path_transform.width, path_transform.height);
        if (span <= 0.0f) {
            continue;
        }

        const Entity workplace =
            pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::WORKPLACE);
        const float target_t =
            routeTForPathEndpoint(registry, worker_component.path_entity, workplace);
        worker_component.route_t += worker_component.direction *
                                    (worker_component.speed_wu / span) * dt;
        if (worker_component.direction > 0.0f) {
            worker_component.route_t = std::min(worker_component.route_t, target_t);
        } else {
            worker_component.route_t = std::max(worker_component.route_t, target_t);
        }

        registry.get<TransformComponent>(worker) =
            transformOnPath(path_transform, worker_component.route_t);
        if (std::fabs(worker_component.route_t - target_t) <= 0.001f) {
            worker_component.route_t = target_t;
            worker_component.direction = 0.0f;
            registry.get<TransformComponent>(worker) =
                transformOnPath(path_transform, worker_component.route_t);
        }
        ++routed;
    }
    return routed;
}

inline bool workerCanStockWorkplaceBench(Registry& registry, Entity worker) {
    if (!workerAtWorkplaceEndpoint(registry, worker) ||
        !workerCarryingSupplyObject(registry, worker)) {
        return false;
    }
    if (dependencyDisrupted(registry)) {
        return false;
    }

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    if (workplace == MAX_ENTITIES ||
        !registry.has<WorkplaceBenchComponent>(workplace)) {
        return false;
    }

    return registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::EMPTY;
}

inline bool stockWorkplaceBenchForWorker(Registry& registry, Entity worker) {
    if (!workerCanStockWorkplaceBench(registry, worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    auto& bench = registry.get<WorkplaceBenchComponent>(workplace);

    bench.state = WorkplaceBenchState::STOCKED;
    hideCarryableObject(registry, worker_component.carried_object);
    worker_component.carried_object = MAX_ENTITIES;
    worker_component.direction = 0.0f;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerWorkplaceBenchDropOffs(Registry& registry) {
    size_t stocked = 0;
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (stockWorkplaceBenchForWorker(registry, worker)) {
            ++stocked;
        }
    }
    return stocked;
}

inline bool workerCanWorkWorkplaceBench(Registry& registry, Entity worker) {
    if (!workerAtWorkplaceEndpoint(registry, worker) ||
        !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.carried_object != MAX_ENTITIES) {
        return false;
    }

    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    if (workplace == MAX_ENTITIES ||
        !registry.has<WorkplaceBenchComponent>(workplace)) {
        return false;
    }

    return registry.get<WorkplaceBenchComponent>(workplace).state ==
           WorkplaceBenchState::STOCKED;
}

inline bool workWorkplaceBenchForWorker(Registry& registry, Entity worker) {
    if (!workerCanWorkWorkplaceBench(registry, worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    registry.get<WorkplaceBenchComponent>(workplace).state =
        WorkplaceBenchState::OUTPUT_READY;
    worker_component.direction = 0.0f;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerWorkplaceBenchWork(Registry& registry) {
    size_t worked = 0;
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (workWorkplaceBenchForWorker(registry, worker)) {
            ++worked;
        }
    }
    return worked;
}

inline bool workerCanTakeWorkplaceOutput(Registry& registry, Entity worker) {
    if (!workerAtWorkplaceEndpoint(registry, worker) ||
        !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.carried_object != MAX_ENTITIES) {
        return false;
    }

    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    if (workplace == MAX_ENTITIES ||
        !registry.has<WorkplaceBenchComponent>(workplace) ||
        registry.get<WorkplaceBenchComponent>(workplace).state !=
            WorkplaceBenchState::OUTPUT_READY) {
        return false;
    }

    const Entity object = firstCarryableObject(registry);
    return object != MAX_ENTITIES && !carryableObjectIsHeld(registry, object);
}

inline bool takeWorkplaceOutputForWorker(Registry& registry, Entity worker) {
    if (!workerCanTakeWorkplaceOutput(registry, worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity workplace = pathEndpointAtRouteT(registry,
                                                  worker_component.path_entity,
                                                  worker_component.route_t);
    const Entity object = firstCarryableObject(registry);

    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    hideCarryableObject(registry, object);
    worker_component.carried_object = object;
    worker_component.direction = 0.0f;
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::EMPTY;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerWorkplaceOutputPickups(Registry& registry) {
    size_t picked_up = 0;
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (takeWorkplaceOutputForWorker(registry, worker)) {
            ++picked_up;
        }
    }
    return picked_up;
}

inline bool moveWorkerToPathEndpointRole(Registry& registry,
                                         Entity worker,
                                         Entity path,
                                         MicroZoneRole endpoint_role) {
    if (!registry.alive(worker) ||
        !registry.has<FixedActorComponent>(worker) ||
        !registry.has<TransformComponent>(worker) ||
        !registry.alive(path) ||
        !registry.has<PathComponent>(path) ||
        !registry.has<TransformComponent>(path)) {
        return false;
    }

    const Entity endpoint = pathEndpointWithRole(registry, path, endpoint_role);
    if (endpoint == MAX_ENTITIES) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = path;
    worker_component.route_t = routeTForPathEndpoint(registry, path, endpoint);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(path), worker_component.route_t);
    return true;
}

inline bool routeWorkerCarryingPartTowardHousing(Registry& registry, Entity worker) {
    if (!workerCarryingPartObject(registry, worker) ||
        !registry.has<TransformComponent>(worker)) {
        refreshWorkerCarryVisual(registry, worker);
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    if (worker_component.kind != FixedActorKind::WORKER ||
        !registry.alive(worker_component.path_entity) ||
        !registry.has<PathComponent>(worker_component.path_entity) ||
        !registry.has<TransformComponent>(worker_component.path_entity)) {
        refreshWorkerCarryVisual(registry, worker);
        return false;
    }

    Entity housing =
        pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::HOUSING);
    if (housing == MAX_ENTITIES) {
        const Entity housing_workplace_path =
            firstPedestrianPathBetweenRoles(registry,
                                            MicroZoneRole::HOUSING,
                                            MicroZoneRole::WORKPLACE);
        if (housing_workplace_path == MAX_ENTITIES ||
            !workerAtWorkplaceEndpoint(registry, worker) ||
            !moveWorkerToPathEndpointRole(registry,
                                          worker,
                                          housing_workplace_path,
                                          MicroZoneRole::WORKPLACE)) {
            refreshWorkerCarryVisual(registry, worker);
            return false;
        }
        housing =
            pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::HOUSING);
    }

    const float target_t =
        routeTForPathEndpoint(registry, worker_component.path_entity, housing);
    constexpr float endpoint_epsilon = 0.001f;
    if (std::fabs(worker_component.route_t - target_t) <= endpoint_epsilon) {
        worker_component.route_t = target_t;
        worker_component.direction = 0.0f;
        registry.get<TransformComponent>(worker) =
            transformOnPath(registry.get<TransformComponent>(worker_component.path_entity),
                            worker_component.route_t);
        refreshWorkerCarryVisual(registry, worker);
        return true;
    }

    worker_component.direction = worker_component.route_t < target_t ? 1.0f : -1.0f;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerFinishedItemDeliveryRoutes(Registry& registry, float dt) {
    size_t routed = 0;
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity worker : workers) {
        if (!routeWorkerCarryingPartTowardHousing(registry, worker)) {
            continue;
        }

        auto& worker_component = registry.get<FixedActorComponent>(worker);
        if (worker_component.direction == 0.0f) {
            ++routed;
            continue;
        }
        if (workerCurrentPathHasSpoofedRouteSignpost(registry, worker)) {
            ++routed;
            continue;
        }

        const auto& path_transform = registry.get<TransformComponent>(worker_component.path_entity);
        const float span = std::max(path_transform.width, path_transform.height);
        if (span <= 0.0f) {
            continue;
        }

        const Entity housing =
            pathEndpointWithRole(registry, worker_component.path_entity, MicroZoneRole::HOUSING);
        if (housing == MAX_ENTITIES) {
            continue;
        }
        const float target_t =
            routeTForPathEndpoint(registry, worker_component.path_entity, housing);
        worker_component.route_t += worker_component.direction *
                                    (worker_component.speed_wu / span) * dt;
        if (worker_component.direction > 0.0f) {
            worker_component.route_t = std::min(worker_component.route_t, target_t);
        } else {
            worker_component.route_t = std::max(worker_component.route_t, target_t);
        }

        registry.get<TransformComponent>(worker) =
            transformOnPath(path_transform, worker_component.route_t);
        if (std::fabs(worker_component.route_t - target_t) <= 0.001f) {
            worker_component.route_t = target_t;
            worker_component.direction = 0.0f;
            registry.get<TransformComponent>(worker) =
                transformOnPath(path_transform, worker_component.route_t);
        }
        ++routed;
    }
    return routed;
}

inline bool workerCanImproveBuilding(Registry& registry, Entity worker) {
    if (!workerAtHousingEndpoint(registry, worker) ||
        !workerCarryingPartObject(registry, worker) ||
        !registry.has<FixedActorComponent>(worker)) {
        return false;
    }

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity housing = pathEndpointAtRouteT(registry,
                                                worker_component.path_entity,
                                                worker_component.route_t);
    if (housing == MAX_ENTITIES ||
        !registry.has<BuildingImprovementComponent>(housing)) {
        return false;
    }

    return !registry.get<BuildingImprovementComponent>(housing).improved;
}

inline bool improveBuildingForWorker(Registry& registry, Entity worker) {
    if (!workerCanImproveBuilding(registry, worker)) {
        return false;
    }

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    const Entity housing = pathEndpointAtRouteT(registry,
                                                worker_component.path_entity,
                                                worker_component.route_t);
    registry.get<BuildingImprovementComponent>(housing).improved = true;
    hideCarryableObject(registry, worker_component.carried_object);
    worker_component.carried_object = MAX_ENTITIES;
    worker_component.direction = 0.0f;
    refreshWorkerCarryVisual(registry, worker);
    return true;
}

inline size_t updateWorkerBuildingDeliveries(Registry& registry) {
    size_t delivered = 0;
    auto workers = registry.view<FixedActorComponent>();
    for (Entity worker : workers) {
        if (improveBuildingForWorker(registry, worker)) {
            ++delivered;
        }
    }
    return delivered;
}

inline bool playerCanTakeNearbyCarryableObject(Registry& registry, Entity player, float range_wu) {
    if (!registry.alive(player) ||
        !registry.has<PlayerComponent>(player) ||
        !registry.has<TransformComponent>(player) ||
        playerInsideAnyBuilding(registry, player)) {
        return false;
    }

    if (registry.get<PlayerComponent>(player).carried_object != MAX_ENTITIES) {
        return false;
    }

    return nearestCarryableObjectInRange(registry,
                                         registry.get<TransformComponent>(player),
                                         range_wu) != MAX_ENTITIES;
}

inline bool takeNearbyCarryableObject(Registry& registry, Entity player, float range_wu) {
    if (!playerCanTakeNearbyCarryableObject(registry, player, range_wu)) {
        return false;
    }

    const Entity object = nearestCarryableObjectInRange(registry,
                                                        registry.get<TransformComponent>(player),
                                                        range_wu);
    if (object == MAX_ENTITIES) {
        return false;
    }

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    return true;
}

inline bool takeSupplyObjectFromInterior(Registry& registry, Entity player) {
    if (!playerCanTakeSupplyObject(registry, player)) {
        return false;
    }

    const Entity object = availableSupplyCarryableObject(registry);
    if (object == MAX_ENTITIES) {
        return false;
    }

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    return true;
}

inline bool playerCanStoreSupplyAtShelter(Registry& registry, Entity player) {
    if (!playerInsideHousingInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    const auto& player_component = registry.get<PlayerComponent>(player);
    if (player_component.carried_object == MAX_ENTITIES ||
        !registry.alive(player_component.carried_object) ||
        !registry.has<CarryableComponent>(player_component.carried_object) ||
        !registry.has<TransformComponent>(player_component.carried_object) ||
        !carryableObjectIsKind(registry, player_component.carried_object, ItemKind::SUPPLY)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!registry.has<ShelterStockComponent>(interaction.building_entity)) {
        return false;
    }

    const auto& shelter = registry.get<ShelterStockComponent>(interaction.building_entity);
    return shelter.current_supply < shelter.capacity;
}

inline bool storeSupplyAtShelter(Registry& registry, Entity player) {
    if (!playerCanStoreSupplyAtShelter(registry, player)) {
        return false;
    }

    auto& player_component = registry.get<PlayerComponent>(player);
    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    auto& shelter = registry.get<ShelterStockComponent>(interaction.building_entity);

    shelter.current_supply = std::min(shelter.capacity, shelter.current_supply + 1);
    hideCarryableObject(registry, player_component.carried_object);
    player_component.carried_object = MAX_ENTITIES;
    return true;
}

inline bool playerCanStockWorkplaceBench(Registry& registry, Entity player) {
    if (!playerInsideWorkplaceInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    const auto& player_component = registry.get<PlayerComponent>(player);
    if (player_component.carried_object == MAX_ENTITIES ||
        !registry.alive(player_component.carried_object) ||
        !registry.has<CarryableComponent>(player_component.carried_object) ||
        !registry.has<TransformComponent>(player_component.carried_object) ||
        !carryableObjectIsKind(registry, player_component.carried_object, ItemKind::SUPPLY)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!registry.has<WorkplaceBenchComponent>(interaction.building_entity)) {
        return false;
    }

    return registry.get<WorkplaceBenchComponent>(interaction.building_entity).state ==
           WorkplaceBenchState::EMPTY;
}

inline bool stockWorkplaceBench(Registry& registry, Entity player) {
    if (!playerCanStockWorkplaceBench(registry, player)) {
        return false;
    }

    auto& player_component = registry.get<PlayerComponent>(player);
    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    auto& bench = registry.get<WorkplaceBenchComponent>(interaction.building_entity);

    bench.state = WorkplaceBenchState::STOCKED;
    hideCarryableObject(registry, player_component.carried_object);
    player_component.carried_object = MAX_ENTITIES;
    return true;
}

inline bool playerCanWorkWorkplaceBench(Registry& registry, Entity player) {
    if (!playerInsideWorkplaceInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!registry.has<WorkplaceBenchComponent>(interaction.building_entity)) {
        return false;
    }

    return registry.get<WorkplaceBenchComponent>(interaction.building_entity).state ==
           WorkplaceBenchState::STOCKED;
}

inline bool workWorkplaceBench(Registry& registry, Entity player) {
    if (!playerCanWorkWorkplaceBench(registry, player)) {
        return false;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    registry.get<WorkplaceBenchComponent>(interaction.building_entity).state =
        WorkplaceBenchState::OUTPUT_READY;
    return true;
}

inline bool playerCanTakeWorkplaceOutput(Registry& registry, Entity player) {
    if (!playerInsideWorkplaceInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    if (registry.get<PlayerComponent>(player).carried_object != MAX_ENTITIES) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!registry.has<WorkplaceBenchComponent>(interaction.building_entity) ||
        registry.get<WorkplaceBenchComponent>(interaction.building_entity).state !=
            WorkplaceBenchState::OUTPUT_READY) {
        return false;
    }

    const Entity object = firstCarryableObject(registry);
    return object != MAX_ENTITIES && !carryableObjectIsHeld(registry, object);
}

inline bool takeWorkplaceOutput(Registry& registry,
                                Entity player,
                                float witness_range_wu = kLocalWitnessRangeWu) {
    if (!playerCanTakeWorkplaceOutput(registry, player)) {
        return false;
    }

    const Entity object = firstCarryableObject(registry);
    auto& player_component = registry.get<PlayerComponent>(player);
    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    const Entity workplace = interaction.building_entity;
    const Entity witness = workerWitnessingWorkplaceOutputTake(registry,
                                                               player,
                                                               workplace,
                                                               witness_range_wu);

    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    hideCarryableObject(registry, object);
    player_component.carried_object = object;
    registry.get<WorkplaceBenchComponent>(workplace).state =
        WorkplaceBenchState::EMPTY;
    if (witness != MAX_ENTITIES) {
        recordLocalSuspicion(registry,
                             witness,
                             LocalSuspicionCause::MISSING_PART,
                             workplace,
                             object,
                             MAX_ENTITIES);
    }
    return true;
}

inline bool playerCanImproveBuilding(Registry& registry, Entity player) {
    if (!playerInsideHousingInterior(registry, player) || !registry.has<PlayerComponent>(player)) {
        return false;
    }

    const auto& player_component = registry.get<PlayerComponent>(player);
    if (player_component.carried_object == MAX_ENTITIES ||
        !registry.alive(player_component.carried_object) ||
        !registry.has<CarryableComponent>(player_component.carried_object) ||
        !carryableObjectIsKind(registry, player_component.carried_object, ItemKind::PART)) {
        return false;
    }

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!registry.has<BuildingImprovementComponent>(interaction.building_entity)) {
        return false;
    }

    return !registry.get<BuildingImprovementComponent>(interaction.building_entity).improved;
}

inline bool improveBuilding(Registry& registry, Entity player) {
    if (!playerCanImproveBuilding(registry, player)) {
        return false;
    }

    auto& player_component = registry.get<PlayerComponent>(player);
    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    registry.get<BuildingImprovementComponent>(interaction.building_entity).improved = true;
    hideCarryableObject(registry, player_component.carried_object);
    player_component.carried_object = MAX_ENTITIES;
    return true;
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
        case MicroZoneRole::SUPPLY: return InspectionTargetType::SUPPLY;
        case MicroZoneRole::MARKET: return InspectionTargetType::MARKET;
        case MicroZoneRole::CLINIC: return InspectionTargetType::CLINIC;
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

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    for (Entity marker : signposts) {
        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(marker));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            target.entity = marker;
            target.type = InspectionTargetType::ROUTE_SIGNPOST;
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

    auto carryables = registry.view<CarryableComponent, TransformComponent>();
    for (Entity obj : carryables) {
        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(obj));
        if (distance <= nearest_distance) {
            nearest_distance = distance;
            target.entity = obj;
            target.type = InspectionTargetType::CARRYABLE_OBJECT;
        }
    }

    return target;
}

inline bool playerCanInspect(Registry& registry,
                             const TransformComponent& player_transform,
                             float range_wu) {
    return nearestInspectionTargetInRange(registry, player_transform, range_wu).entity != MAX_ENTITIES;
}

inline InspectionTarget playerInspectionTarget(Registry& registry,
                                               Entity player,
                                               float range_wu) {
    InspectionTarget target;
    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return target;
    }

    if (registry.has<BuildingInteractionComponent>(player)) {
        const auto& interaction = registry.get<BuildingInteractionComponent>(player);
        if (interaction.inside_building && registry.alive(interaction.building_entity)) {
            target.entity = interaction.building_entity;
            if (interaction.building_role == MicroZoneRole::HOUSING) {
                target.type = InspectionTargetType::HOUSING_INTERIOR;
            } else if (interaction.building_role == MicroZoneRole::WORKPLACE) {
                target.type = InspectionTargetType::WORKPLACE_INTERIOR;
            } else if (interaction.building_role == MicroZoneRole::SUPPLY) {
                target.type = InspectionTargetType::SUPPLY_INTERIOR;
            } else {
                target.type = inspectionTypeForRole(interaction.building_role);
            }
            return target;
        }
    }

    return nearestInspectionTargetInRange(registry, registry.get<TransformComponent>(player), range_wu);
}

inline bool playerHasInheritedGadget(Registry& registry, Entity player) {
    return registry.alive(player) &&
           registry.has<InheritedGadgetComponent>(player) &&
           registry.get<InheritedGadgetComponent>(player).available;
}

inline std::string inheritedGadgetLabel(Registry& registry, Entity player) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "NONE";
    }
    return registry.get<InheritedGadgetComponent>(player).label;
}

inline std::string inheritedGadgetReadout(Registry& registry, Entity player) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "DEBUGGER:NONE";
    }
    return "DEBUGGER:" + inheritedGadgetLabel(registry, player) + " READY";
}

inline std::string inheritedGadgetResultReadout(Registry& registry, Entity player) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "DEBUGGER RESULT: UNAVAILABLE";
    }

    const auto& gadget = registry.get<InheritedGadgetComponent>(player);
    if (gadget.last_result.empty()) {
        return "DEBUGGER RESULT: IDLE";
    }
    const char* label = gadget.last_result_kind == InheritedGadgetResultKind::INTERFERENCE_TORCH ?
        "INTERFERENCE TORCH RESULT: " :
        "DEBUGGER RESULT: ";
    return std::string(label) + gadget.last_result;
}

inline const char* inheritedGadgetTargetLabel(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::HOUSING:
            return "HOUSING";
        case InspectionTargetType::WORKPLACE:
            return "WORKPLACE";
        case InspectionTargetType::SUPPLY:
            return "SUPPLY";
        case InspectionTargetType::MARKET:
            return "MARKET";
        case InspectionTargetType::CLINIC:
            return "CLINIC";
        case InspectionTargetType::PEDESTRIAN_PATH:
            return "PATH";
        case InspectionTargetType::ROUTE_SIGNPOST:
            return "SIGNPOST";
        case InspectionTargetType::WORKER:
            return "WORKER";
        case InspectionTargetType::HOUSING_INTERIOR:
            return "HOUSING INTERIOR";
        case InspectionTargetType::WORKPLACE_INTERIOR:
            return "WORKPLACE INTERIOR";
        case InspectionTargetType::SUPPLY_INTERIOR:
            return "SUPPLY INTERIOR";
        case InspectionTargetType::CARRYABLE_OBJECT:
            return "CARRYABLE OBJECT";
        case InspectionTargetType::NONE:
            return "NO TARGET";
    }
    return "NO TARGET";
}

inline std::string inheritedGadgetWorkerScan(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return "NO SIGNAL";
    }

    return "WORKER SIGNAL: DEBT WORK; PAY DOCKED IF STALLED; ROUTE QUOTA: 1";
}

inline std::string inheritedGadgetSiteMetadataScan(Registry& registry,
                                                   const InspectionTarget& target) {
    auto building_scan = [&](MicroZoneRole role) {
        std::string readout = buildingPurposeScanReadoutForRole(role);
        const std::string dependency = dependencyScanReadout(registry, role);
        if (!dependency.empty()) {
            readout += "; " + dependency;
        }
        return readout;
    };

    switch (target.type) {
        case InspectionTargetType::HOUSING:
        case InspectionTargetType::HOUSING_INTERIOR:
            return building_scan(MicroZoneRole::HOUSING);
        case InspectionTargetType::WORKPLACE:
        case InspectionTargetType::WORKPLACE_INTERIOR:
            return building_scan(MicroZoneRole::WORKPLACE);
        case InspectionTargetType::SUPPLY:
        case InspectionTargetType::SUPPLY_INTERIOR:
            return building_scan(MicroZoneRole::SUPPLY);
        case InspectionTargetType::MARKET:
            return building_scan(MicroZoneRole::MARKET);
        case InspectionTargetType::CLINIC:
            return building_scan(MicroZoneRole::CLINIC);
        case InspectionTargetType::ROUTE_SIGNPOST:
            if (registry.alive(target.entity) && registry.has<RouteSignpostComponent>(target.entity)) {
                const auto& signpost = registry.get<RouteSignpostComponent>(target.entity);
                return routePurposeDebugReadout(registry,
                                                signpost.path_entity,
                                                "SIGNPOST") +
                       "; POINTS TO " +
                       roleDisplayName(signpost.target_role);
            }
            return "SIGNPOST ROUTE: UNKNOWN ROUTE; EXPECTED CARGO: UNKNOWN; ACCESS: UNKNOWN";
        case InspectionTargetType::PEDESTRIAN_PATH:
            if (registry.alive(target.entity) && registry.has<PathComponent>(target.entity)) {
                return routePurposeDebugReadout(registry, target.entity, "PATH");
            }
            return "PATH ROUTE: UNKNOWN ROUTE; EXPECTED CARGO: UNKNOWN; ACCESS: UNKNOWN";
        case InspectionTargetType::CARRYABLE_OBJECT:
            return "OBJECT PURPOSE: LOOP MATERIAL";
        case InspectionTargetType::WORKER:
        case InspectionTargetType::NONE:
            break;
    }
    return std::string(inheritedGadgetTargetLabel(target.type)) + " SIGNAL DETECTED";
}

inline std::string inheritedGadgetScanResult(Registry& registry, const InspectionTarget& target) {
    if (target.entity == MAX_ENTITIES) {
        return "NO SIGNAL";
    }
    if (target.type == InspectionTargetType::WORKER) {
        return inheritedGadgetWorkerScan(registry, target.entity);
    }
    return inheritedGadgetSiteMetadataScan(registry, target);
}

inline bool inspectionTargetIsDependencyTarget(const InspectionTarget& target) {
    switch (target.type) {
        case InspectionTargetType::WORKPLACE:
        case InspectionTargetType::WORKPLACE_INTERIOR:
        case InspectionTargetType::SUPPLY:
        case InspectionTargetType::SUPPLY_INTERIOR:
            return target.entity != MAX_ENTITIES;
        case InspectionTargetType::NONE:
        case InspectionTargetType::HOUSING:
        case InspectionTargetType::MARKET:
        case InspectionTargetType::CLINIC:
        case InspectionTargetType::PEDESTRIAN_PATH:
        case InspectionTargetType::ROUTE_SIGNPOST:
        case InspectionTargetType::WORKER:
        case InspectionTargetType::HOUSING_INTERIOR:
        case InspectionTargetType::CARRYABLE_OBJECT:
            return false;
    }
    return false;
}

inline bool inheritedGadgetCanSpoofTarget(const InspectionTarget& target) {
    return target.entity != MAX_ENTITIES &&
           (target.type == InspectionTargetType::ROUTE_SIGNPOST ||
            inspectionTargetIsDependencyTarget(target));
}

inline bool playerCanUseInheritedGadget(Registry& registry, Entity player) {
    return playerHasInheritedGadget(registry, player);
}

inline std::string inheritedGadgetPromptReadout(Registry& registry,
                                                Entity player,
                                                float range_wu) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "DEBUGGER:NONE";
    }

    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    if (target.entity == MAX_ENTITIES) {
        return "SPACE DEBUGGER: NO SIGNAL";
    }
    return std::string("SPACE DEBUGGER ON ") + inheritedGadgetTargetLabel(target.type);
}

inline std::string inheritedGadgetSpoofPromptReadout(Registry& registry,
                                                     Entity player,
                                                     float range_wu) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "TORCH:NONE";
    }

    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    if (!inheritedGadgetCanSpoofTarget(target)) {
        return "G INTERFERENCE TORCH:N/A";
    }
    if (inspectionTargetIsDependencyTarget(target)) {
        return dependencyDisrupted(registry) ?
            "G INTERFERENCE TORCH RESTORE DEPENDENCY" :
            "G INTERFERENCE TORCH DISRUPT DEPENDENCY";
    }
    return routeSignpostSpoofed(registry, target.entity) ?
        "G INTERFERENCE TORCH RESTORE SIGNPOST" :
        "G INTERFERENCE TORCH SPOOF SIGNPOST";
}

inline bool useInheritedGadget(Registry& registry, Entity player, float range_wu) {
    if (!playerCanUseInheritedGadget(registry, player)) {
        return false;
    }

    auto& gadget = registry.get<InheritedGadgetComponent>(player);
    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    if (target.entity == MAX_ENTITIES) {
        gadget.last_result_kind = InheritedGadgetResultKind::DEBUGGER;
        gadget.last_result = "NO SIGNAL";
        return false;
    }

    gadget.last_result_kind = InheritedGadgetResultKind::DEBUGGER;
    gadget.last_result = inheritedGadgetScanResult(registry, target);
    return true;
}

inline bool useInheritedGadgetSpoof(Registry& registry, Entity player, float range_wu) {
    if (!playerCanUseInheritedGadget(registry, player)) {
        return false;
    }

    auto& gadget = registry.get<InheritedGadgetComponent>(player);
    gadget.last_result_kind = InheritedGadgetResultKind::INTERFERENCE_TORCH;
    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    if (target.entity == MAX_ENTITIES) {
        gadget.last_result = "FAILED: NO SIGNAL";
        return false;
    }
    if (!inheritedGadgetCanSpoofTarget(target)) {
        gadget.last_result = "FAILED: SIGNPOST OR DEPENDENCY REQUIRED";
        return false;
    }

    if (inspectionTargetIsDependencyTarget(target)) {
        if (!toggleDependencyDisruption(registry)) {
            gadget.last_result = "FAILED: DEPENDENCY UNRESOLVED";
            return false;
        }
        gadget.last_result = dependencyDisrupted(registry) ?
            "DISRUPTED DEPENDENCY: SUPPLY FLOW CONFUSED" :
            "RESTORED DEPENDENCY: SUPPLY FLOW CLEAR";
        return true;
    }

    if (!registry.alive(target.entity) ||
        !registry.has<RouteSignpostComponent>(target.entity)) {
        gadget.last_result = "FAILED: SIGNPOST REQUIRED";
        return false;
    }

    auto& signpost = registry.get<RouteSignpostComponent>(target.entity);
    const bool was_spoofed = signpost.spoofed;
    const Entity witness = workerWitnessingRouteTampering(registry,
                                                          target.entity,
                                                          range_wu);
    signpost.spoofed = !signpost.spoofed;
    signpost.signal_recovered = !signpost.spoofed;
    gadget.last_result = signpost.spoofed ?
        "SPOOFED SIGNPOST: ROUTE SIGNAL CONFUSED" :
        "RESTORED SIGNPOST: ROUTE SIGNAL CLEAR";
    if (!was_spoofed && signpost.spoofed && witness != MAX_ENTITIES) {
        recordLocalSuspicion(registry,
                             witness,
                             LocalSuspicionCause::ROUTE_TAMPERING,
                             pathEndpointWithRole(registry,
                                                  signpost.path_entity,
                                                  MicroZoneRole::WORKPLACE),
                             target.entity,
                             signpost.path_entity);
    }
    return true;
}

inline PlayerLocationState locationStateForRole(MicroZoneRole role, bool inside) {
    switch (role) {
        case MicroZoneRole::HOUSING:
            return inside ? PlayerLocationState::INSIDE_HOUSING : PlayerLocationState::NEAR_HOUSING;
        case MicroZoneRole::WORKPLACE:
            return inside ? PlayerLocationState::INSIDE_WORKPLACE : PlayerLocationState::NEAR_WORKPLACE;
        case MicroZoneRole::SUPPLY:
            return inside ? PlayerLocationState::INSIDE_SUPPLY : PlayerLocationState::NEAR_SUPPLY;
        case MicroZoneRole::MARKET:
        case MicroZoneRole::CLINIC:
            return PlayerLocationState::OUTSIDE;
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
            std::max(0, config.workplace_micro_zone_count) +
            std::max(0, config.supply_micro_zone_count) +
            std::max(0, config.market_micro_zone_count) +
            std::max(0, config.clinic_micro_zone_count));
    const size_t expected_buildings =
        expected_macros * static_cast<size_t>(
            std::max(0, config.housing_building_count) +
            std::max(0, config.workplace_building_count) +
            std::max(0, config.supply_building_count) +
            std::max(0, config.market_building_count) +
            std::max(0, config.clinic_building_count));

    if (macros.size() != expected_macros) return false;
    if (micros.size() != expected_micros) return false;
    if (buildings.size() != expected_buildings) return false;

    for (Entity macro : macros) {
        if (registry.get<MacroZoneComponent>(macro).type != config.macro_type) return false;
    }

    size_t housing_micros = 0;
    size_t workplace_micros = 0;
    size_t supply_micros = 0;
    size_t market_micros = 0;
    size_t clinic_micros = 0;
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
            case MicroZoneRole::SUPPLY:
                ++supply_micros;
                break;
            case MicroZoneRole::MARKET:
                ++market_micros;
                break;
            case MicroZoneRole::CLINIC:
                ++clinic_micros;
                break;
        }
    }

    if (housing_micros != expected_macros * static_cast<size_t>(std::max(0, config.housing_micro_zone_count))) return false;
    if (workplace_micros != expected_macros * static_cast<size_t>(std::max(0, config.workplace_micro_zone_count))) return false;
    if (supply_micros != expected_macros * static_cast<size_t>(std::max(0, config.supply_micro_zone_count))) return false;
    if (market_micros != expected_macros * static_cast<size_t>(std::max(0, config.market_micro_zone_count))) return false;
    if (clinic_micros != expected_macros * static_cast<size_t>(std::max(0, config.clinic_micro_zone_count))) return false;

    size_t housing_buildings = 0;
    size_t workplace_buildings = 0;
    size_t supply_buildings = 0;
    size_t market_buildings = 0;
    size_t clinic_buildings = 0;
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
            case MicroZoneRole::SUPPLY:
                ++supply_buildings;
                break;
            case MicroZoneRole::MARKET:
                ++market_buildings;
                break;
            case MicroZoneRole::CLINIC:
                ++clinic_buildings;
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
    if (supply_buildings != expected_macros * static_cast<size_t>(std::max(0, config.supply_building_count))) return false;
    if (market_buildings != expected_macros * static_cast<size_t>(std::max(0, config.market_building_count))) return false;
    if (clinic_buildings != expected_macros * static_cast<size_t>(std::max(0, config.clinic_building_count))) return false;

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
