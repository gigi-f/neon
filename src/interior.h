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
        !registry.has<BuildingUseComponent>(building) ||
        !registry.get<BuildingComponent>(building).is_enterable) {
        return false;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    const auto& use = registry.get<BuildingUseComponent>(building);
    const auto layout = interiorLayoutForRole(use.role);

    interaction.building_entity = building;
    interaction.building_role = use.role;
    interaction.exterior_position = registry.get<TransformComponent>(player);
    interaction.interior_position = layout.spawn;
    interaction.inside_building = true;
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
