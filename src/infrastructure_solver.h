#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include "components.h"
#include "ecs.h"
#include "world_config.h"
#include "world_generation.h"

inline Entity firstBuildingByRole(Registry& registry, MicroZoneRole role) {
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role == role) {
            return building;
        }
    }
    return MAX_ENTITIES;
}

inline bool pathExistsBetween(Registry& registry,
                              Entity from,
                              Entity to,
                              PathKind kind) {
    auto paths = registry.view<PathComponent>();
    for (Entity path : paths) {
        const auto& component = registry.get<PathComponent>(path);
        if (component.kind != kind) continue;
        const bool same_direction = component.from == from && component.to == to;
        const bool reverse_direction = component.from == to && component.to == from;
        if (same_direction || reverse_direction) return true;
    }
    return false;
}

inline TransformComponent pedestrianPathTransformBetween(const TransformComponent& from,
                                                         const TransformComponent& to) {
    const AabbRect a = aabbFromTransform(from);
    const AabbRect b = aabbFromTransform(to);
    constexpr float thickness = 8.0f;

    if (a.bottom <= b.top || b.bottom <= a.top) {
        const bool a_above_b = a.bottom <= b.top;
        const float start = a_above_b ? a.bottom : b.bottom;
        const float end = a_above_b ? b.top : a.top;
        return TransformComponent{
            (from.x + to.x) * 0.5f,
            (start + end) * 0.5f,
            thickness,
            std::max(thickness, end - start)
        };
    }

    if (a.right <= b.left || b.right <= a.left) {
        const bool a_left_of_b = a.right <= b.left;
        const float start = a_left_of_b ? a.right : b.right;
        const float end = a_left_of_b ? b.left : a.left;
        return TransformComponent{
            (start + end) * 0.5f,
            (from.y + to.y) * 0.5f,
            std::max(thickness, end - start),
            thickness
        };
    }

    return TransformComponent{
        (from.x + to.x) * 0.5f,
        (from.y + to.y) * 0.5f,
        std::max(thickness, std::fabs(from.x - to.x)),
        std::max(thickness, std::fabs(from.y - to.y))
    };
}

inline Entity createPedestrianPath(Registry& registry, Entity from, Entity to) {
    if (!registry.alive(from) || !registry.alive(to) ||
        !registry.has<TransformComponent>(from) || !registry.has<TransformComponent>(to)) {
        return MAX_ENTITIES;
    }

    Entity path = registry.create();
    registry.assign<TransformComponent>(path,
        pedestrianPathTransformBetween(registry.get<TransformComponent>(from),
                                       registry.get<TransformComponent>(to)));
    registry.assign<PathComponent>(path, PathKind::PEDESTRIAN, from, to);
    registry.assign<GlyphComponent>(path, std::string("."),
        static_cast<uint8_t>(120), static_cast<uint8_t>(210), static_cast<uint8_t>(170),
        static_cast<uint8_t>(220), 0.75f, true, true);
    return path;
}

inline size_t derivePedestrianPaths(Registry& registry, const ConnectionSpec& spec) {
    if (spec.path_kind != PathKind::PEDESTRIAN) return 0;

    const Entity from = firstBuildingByRole(registry, spec.from_role);
    const Entity to = firstBuildingByRole(registry, spec.to_role);
    if (from == MAX_ENTITIES || to == MAX_ENTITIES) return 0;
    if (pathExistsBetween(registry, from, to, spec.path_kind)) return 0;

    return createPedestrianPath(registry, from, to) == MAX_ENTITIES ? 0 : 1;
}

inline size_t deriveInfrastructure(Registry& registry, const WorldConfig& config) {
    (void)config;
    return derivePedestrianPaths(registry, kHousingToWorkplacePedestrianAccess);
}
