#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include "components.h"
#include "ecs.h"
#include "world_builder.h"

struct InteriorWall {
    TransformComponent bounds{};
};

struct InteriorLayout {
    MicroZoneRole role = MicroZoneRole::HOUSING;
    float width = 96.0f;
    float height = 72.0f;
    TransformComponent spawn{0.0f, 20.0f, 12.0f, 12.0f};
    std::vector<InteriorWall> walls{};
};

inline InteriorLayout interiorLayoutForRole(MicroZoneRole role) {
    InteriorLayout layout;
    layout.role = role;
    switch (role) {
        case MicroZoneRole::HOUSING:
            layout.width = 96.0f;
            layout.height = 72.0f;
            layout.spawn = TransformComponent{0.0f, 20.0f, 12.0f, 12.0f};
            break;
        case MicroZoneRole::WORKPLACE:
            layout.width = 120.0f;
            layout.height = 84.0f;
            layout.spawn = TransformComponent{0.0f, 26.0f, 12.0f, 12.0f};
            break;
        case MicroZoneRole::SUPPLY:
            layout.width = 108.0f;
            layout.height = 72.0f;
            layout.spawn = TransformComponent{0.0f, 20.0f, 12.0f, 12.0f};
            break;
        case MicroZoneRole::MARKET:
            layout.width = 112.0f;
            layout.height = 78.0f;
            layout.spawn = TransformComponent{0.0f, 20.0f, 12.0f, 12.0f};
            break;
        case MicroZoneRole::CLINIC:
            layout.width = 100.0f;
            layout.height = 72.0f;
            layout.spawn = TransformComponent{0.0f, 20.0f, 12.0f, 12.0f};
            break;
    }
    return layout;
}

inline InteriorLayout interiorLayoutForBuilding(Registry& registry, Entity building) {
    if (registry.alive(building) && registry.has<BuildingUseComponent>(building)) {
        return interiorLayoutForRole(registry.get<BuildingUseComponent>(building).role);
    }
    return interiorLayoutForRole(MicroZoneRole::HOUSING);
}

inline bool interiorPositionWithinLayout(const InteriorLayout& layout,
                                         const TransformComponent& position) {
    const float half_w = position.width * 0.5f;
    const float half_h = position.height * 0.5f;
    return position.x - half_w >= -layout.width * 0.5f &&
           position.x + half_w <= layout.width * 0.5f &&
           position.y - half_h >= -layout.height * 0.5f &&
           position.y + half_h <= layout.height * 0.5f;
}

inline bool interiorPositionOverlapsWall(const InteriorLayout& layout,
                                         const TransformComponent& position) {
    const AabbRect box = aabbFromTransform(position);
    for (const auto& wall : layout.walls) {
        if (aabbOverlap(box, aabbFromTransform(wall.bounds))) {
            return true;
        }
    }
    return false;
}

inline TransformComponent clampInteriorPosition(const InteriorLayout& layout,
                                                TransformComponent position) {
    const float half_w = position.width * 0.5f;
    const float half_h = position.height * 0.5f;
    position.x = std::clamp(position.x,
                            -layout.width * 0.5f + half_w,
                            layout.width * 0.5f - half_w);
    position.y = std::clamp(position.y,
                            -layout.height * 0.5f + half_h,
                            layout.height * 0.5f - half_h);
    return position;
}

inline TransformComponent movedInteriorPosition(const InteriorLayout& layout,
                                                const TransformComponent& current,
                                                float dx,
                                                float dy,
                                                float speed_wu,
                                                float dt) {
    if (dx == 0.0f && dy == 0.0f) return current;

    const float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0f) return current;

    TransformComponent proposed = current;
    proposed.x += (dx / len) * speed_wu * dt;
    proposed.y += (dy / len) * speed_wu * dt;
    proposed = clampInteriorPosition(layout, proposed);

    if (interiorPositionOverlapsWall(layout, proposed)) {
        return current;
    }
    return proposed;
}

inline TransformComponent fallbackExitPosition(Registry& registry,
                                               const BuildingInteractionComponent& interaction,
                                               const TransformComponent& player_size) {
    TransformComponent candidate = interaction.exterior_position;
    if (!transformOverlapsSolid(registry, candidate)) {
        return candidate;
    }

    if (!registry.alive(interaction.building_entity) ||
        !registry.has<TransformComponent>(interaction.building_entity)) {
        return candidate;
    }

    const auto& building = registry.get<TransformComponent>(interaction.building_entity);
    const float gap = 2.0f;
    const TransformComponent candidates[] = {
        TransformComponent{building.x,
                           building.y + (building.height + player_size.height) * 0.5f + gap,
                           player_size.width,
                           player_size.height},
        TransformComponent{building.x,
                           building.y - (building.height + player_size.height) * 0.5f - gap,
                           player_size.width,
                           player_size.height},
        TransformComponent{building.x + (building.width + player_size.width) * 0.5f + gap,
                           building.y,
                           player_size.width,
                           player_size.height},
        TransformComponent{building.x - (building.width + player_size.width) * 0.5f - gap,
                           building.y,
                           player_size.width,
                           player_size.height}
    };

    for (const auto& exit_candidate : candidates) {
        if (!transformOverlapsSolid(registry, exit_candidate)) {
            return exit_candidate;
        }
    }
    return candidate;
}

inline bool enterBuildingInterior(Registry& registry, Entity player, Entity building) {
    if (!registry.alive(player) || !registry.alive(building) ||
        !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player) ||
        !registry.has<BuildingComponent>(building) ||
        !registry.has<BuildingUseComponent>(building)) {
        return false;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    const auto& use = registry.get<BuildingUseComponent>(building);
    const bool clinic_boundary_open =
        use.role == MicroZoneRole::CLINIC && clinicAccessSpoofed(registry, building);
    if (!registry.get<BuildingComponent>(building).is_enterable && !clinic_boundary_open) {
        return false;
    }
    const auto layout = interiorLayoutForRole(use.role);

    interaction.building_entity = building;
    interaction.building_role = use.role;
    interaction.exterior_position = registry.get<TransformComponent>(player);
    interaction.interior_position = layout.spawn;
    interaction.inside_building = true;
    return true;
}

inline Entity nearestClinicRestrictedBoundaryInRange(Registry& registry,
                                                     const TransformComponent& player_transform,
                                                     float range_wu) {
    return nearestBuildingInRange(registry, player_transform, range_wu, MicroZoneRole::CLINIC);
}

inline bool playerCanAttemptClinicRestrictedBoundary(Registry& registry,
                                                     Entity player,
                                                     float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        playerInsideTransitInterior(registry, player)) {
        return false;
    }
    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        return false;
    }
    if (nearestInteractableBuildingInRange(registry,
                                           registry.get<TransformComponent>(player),
                                           range_wu) != MAX_ENTITIES) {
        return false;
    }
    return nearestClinicRestrictedBoundaryInRange(registry,
                                                 registry.get<TransformComponent>(player),
                                                 range_wu) != MAX_ENTITIES;
}

inline std::string clinicRestrictedBoundaryActionReadout(Registry& registry, Entity clinic) {
    return clinicAccessSpoofed(registry, clinic) ?
        "CLINIC RECORDS BOUNDARY OPEN: GHOST CLEARANCE ACCEPTED" :
        "CLINIC ACCESS DENIED: RECORDS STAFF ONLY";
}

inline bool useClinicRestrictedBoundary(Registry& registry,
                                        Entity player,
                                        float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return false;
    }

    const Entity clinic = nearestClinicRestrictedBoundaryInRange(
        registry,
        registry.get<TransformComponent>(player),
        range_wu);
    if (clinic == MAX_ENTITIES) {
        return false;
    }

    if (registry.has<InheritedGadgetComponent>(player)) {
        auto& gadget = registry.get<InheritedGadgetComponent>(player);
        gadget.last_result_kind = InheritedGadgetResultKind::ACTION;
        gadget.last_result_target_entity = clinic;
        gadget.last_result_target_type = InspectionTargetType::CLINIC;
        gadget.last_result = clinicRestrictedBoundaryActionReadout(registry, clinic);
    }

    if (!clinicAccessSpoofed(registry, clinic)) {
        return true;
    }
    enterBuildingInterior(registry, player, clinic);
    return true;
}

inline Entity nearestMarketBuildingInRange(Registry& registry,
                                           const TransformComponent& player_transform,
                                           float range_wu) {
    return nearestBuildingInRange(registry, player_transform, range_wu, MicroZoneRole::MARKET);
}

inline bool playerCanExchangeAtMarket(Registry& registry,
                                      Entity player,
                                      float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        playerInsideTransitInterior(registry, player)) {
        return false;
    }
    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        return false;
    }
    if (nearestInteractableBuildingInRange(registry,
                                           registry.get<TransformComponent>(player),
                                           range_wu) != MAX_ENTITIES) {
        return false;
    }
    return nearestMarketBuildingInRange(registry,
                                        registry.get<TransformComponent>(player),
                                        range_wu) != MAX_ENTITIES;
}

inline bool exchangeAtMarket(Registry& registry,
                             Entity player,
                             float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return false;
    }

    const Entity market = nearestMarketBuildingInRange(registry,
                                                       registry.get<TransformComponent>(player),
                                                       range_wu);
    if (market == MAX_ENTITIES) {
        return false;
    }

    if (!registry.has<MarketLedgerComponent>(market)) {
        return false;
    }

    auto& ledger = registry.get<MarketLedgerComponent>(market);
    std::string result_label;

    const Entity carried = registry.alive(player) && registry.has<PlayerComponent>(player)
        ? registry.get<PlayerComponent>(player).carried_object
        : MAX_ENTITIES;

    if (carried == MAX_ENTITIES) {
        result_label = "NO ITEM: CLAIM DEFERRED";
    } else if (carryableObjectIsKind(registry, carried, ItemKind::PART)) {
        registry.get<CarryableComponent>(carried).kind = ItemKind::SUPPLY;
        result_label = "PART -> SUPPLY";
    } else if (carryableObjectIsKind(registry, carried, ItemKind::SUPPLY)) {
        ledger.exchange_claimed = true;
        result_label = "SUPPLY: RATION CLAIMED";
    } else {
        result_label = "WRONG ITEM";
    }

    ledger.last_exchange_result = result_label;

    if (registry.has<InheritedGadgetComponent>(player)) {
        auto& gadget = registry.get<InheritedGadgetComponent>(player);
        gadget.last_result_kind = InheritedGadgetResultKind::ACTION;
        gadget.last_result_target_entity = market;
        gadget.last_result_target_type = InspectionTargetType::MARKET;
        gadget.last_result = std::string("MARKET EXCHANGE: ") + result_label;
    }

    return true;
}

inline bool exitBuildingInterior(Registry& registry, Entity player) {
    if (!registry.alive(player) ||
        !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player)) {
        return false;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (!interaction.inside_building) return false;

    const auto player_size = registry.get<TransformComponent>(player);
    registry.get<TransformComponent>(player) =
        fallbackExitPosition(registry, interaction, player_size);
    interaction.inside_building = false;
    return true;
}
