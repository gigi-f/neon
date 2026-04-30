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
    const bool x_overlap_or_touch = a.left <= b.right && a.right >= b.left;
    const bool y_overlap_or_touch = a.top <= b.bottom && a.bottom >= b.top;

    if ((a.bottom <= b.top || b.bottom <= a.top) && x_overlap_or_touch) {
        const bool a_above_b = a.bottom <= b.top;
        const float start = a_above_b ? a.bottom : b.bottom;
        const float end = a_above_b ? b.top : a.top;
        const float overlap_start = std::max(a.left, b.left);
        const float overlap_end = std::min(a.right, b.right);
        return TransformComponent{
            (overlap_start + overlap_end) * 0.5f,
            (start + end) * 0.5f,
            thickness,
            std::max(thickness, end - start)
        };
    }

    if ((a.right <= b.left || b.right <= a.left) && y_overlap_or_touch) {
        const bool a_left_of_b = a.right <= b.left;
        const float start = a_left_of_b ? a.right : b.right;
        const float end = a_left_of_b ? b.left : a.left;
        const float overlap_start = std::max(a.top, b.top);
        const float overlap_end = std::min(a.bottom, b.bottom);
        return TransformComponent{
            (start + end) * 0.5f,
            (overlap_start + overlap_end) * 0.5f,
            std::max(thickness, end - start),
            thickness
        };
    }

    if (!x_overlap_or_touch || !y_overlap_or_touch) {
        return TransformComponent{(from.x + to.x) * 0.5f,
                                  (from.y + to.y) * 0.5f,
                                  0.0f,
                                  0.0f};
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

    const TransformComponent path_transform =
        pedestrianPathTransformBetween(registry.get<TransformComponent>(from),
                                       registry.get<TransformComponent>(to));
    if (path_transform.width <= 0.0f || path_transform.height <= 0.0f) {
        return MAX_ENTITIES;
    }

    Entity path = registry.create();
    registry.assign<TransformComponent>(path, path_transform);
    registry.assign<PathComponent>(path, PathKind::PEDESTRIAN, from, to);
    registry.assign<PathStateComponent>(path, PathState::LIT);
    registry.assign<GlyphComponent>(path, std::string("."),
        static_cast<uint8_t>(120), static_cast<uint8_t>(210), static_cast<uint8_t>(170),
        static_cast<uint8_t>(220), 0.75f, true, true);
    return path;
}

inline TransformComponent routeSignpostTransform(const TransformComponent& path,
                                                 const TransformComponent& endpoint) {
    constexpr float marker_size = 8.0f;
    constexpr float marker_offset = marker_size * 0.5f + 2.0f;
    const AabbRect endpoint_box = aabbFromTransform(endpoint);
    const bool vertical = path.height >= path.width;
    if (vertical) {
        return TransformComponent{
            path.x,
            endpoint.y <= path.y ? endpoint_box.bottom + marker_offset
                                  : endpoint_box.top - marker_offset,
            marker_size,
            marker_size
        };
    }

    return TransformComponent{
        endpoint.x <= path.x ? endpoint_box.right + marker_offset
                             : endpoint_box.left - marker_offset,
        path.y,
        marker_size,
        marker_size
    };
}

inline char routeSignpostGlyphForTarget(const TransformComponent& marker,
                                        const TransformComponent& target) {
    const float dx = target.x - marker.x;
    const float dy = target.y - marker.y;
    if (std::fabs(dx) >= std::fabs(dy)) {
        return dx < 0.0f ? '<' : '>';
    }
    return dy < 0.0f ? '^' : 'v';
}

inline Entity createRouteSignpost(Registry& registry,
                                  Entity path,
                                  Entity endpoint,
                                  Entity target) {
    if (!registry.alive(path) ||
        !registry.alive(endpoint) ||
        !registry.alive(target) ||
        !registry.has<TransformComponent>(path) ||
        !registry.has<TransformComponent>(endpoint) ||
        !registry.has<TransformComponent>(target) ||
        !registry.has<BuildingUseComponent>(target)) {
        return MAX_ENTITIES;
    }

    Entity marker = registry.create();
    registry.assign<TransformComponent>(marker,
        routeSignpostTransform(registry.get<TransformComponent>(path),
                               registry.get<TransformComponent>(endpoint)));
    registry.assign<RouteSignpostComponent>(marker,
        path,
        endpoint,
        registry.get<BuildingUseComponent>(target).role);
    const char glyph = routeSignpostGlyphForTarget(registry.get<TransformComponent>(marker),
                                                   registry.get<TransformComponent>(target));
    registry.assign<GlyphComponent>(marker, std::string(1, glyph),
        static_cast<uint8_t>(255), static_cast<uint8_t>(225), static_cast<uint8_t>(120),
        static_cast<uint8_t>(255), 0.9f, true, false);
    return marker;
}

inline const char* pathStateName(PathState state) {
    switch (state) {
        case PathState::LIT: return "LIT";
    }
    return "";
}

inline const char* pathStateInspectionDetail(PathState state) {
    switch (state) {
        case PathState::LIT:
            return "Foot path. LIT: low amber markers make the route easier to follow.";
    }
    return "Foot path. Non-solid access between buildings.";
}

inline size_t derivePedestrianPaths(Registry& registry, const ConnectionSpec& spec) {
    if (spec.path_kind != PathKind::PEDESTRIAN) return 0;

    const Entity from = firstBuildingByRole(registry, spec.from_role);
    const Entity to = firstBuildingByRole(registry, spec.to_role);
    if (from == MAX_ENTITIES || to == MAX_ENTITIES) return 0;
    if (pathExistsBetween(registry, from, to, spec.path_kind)) return 0;

    const Entity path = createPedestrianPath(registry, from, to);
    if (path == MAX_ENTITIES) return 0;

    createRouteSignpost(registry, path, from, to);
    createRouteSignpost(registry, path, to, from);
    return 1;
}

inline bool infrastructurePointInMacro(const TransformComponent& transform,
                                       const MacroZoneComponent& macro) {
    return transform.x >= macro.x0 && transform.x <= macro.x1 &&
           transform.y >= macro.y0 && transform.y <= macro.y1;
}

inline Entity infrastructureFirstBuildingByRoleInMacro(Registry& registry,
                                                       Entity macro,
                                                       MicroZoneRole role) {
    if (!registry.alive(macro) || !registry.has<MacroZoneComponent>(macro)) {
        return MAX_ENTITIES;
    }
    const auto& macro_component = registry.get<MacroZoneComponent>(macro);
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role == role &&
            infrastructurePointInMacro(registry.get<TransformComponent>(building),
                                       macro_component)) {
            return building;
        }
    }
    return MAX_ENTITIES;
}

inline size_t derivePedestrianPathsInMacro(Registry& registry,
                                           Entity macro,
                                           const ConnectionSpec& spec) {
    if (spec.path_kind != PathKind::PEDESTRIAN) return 0;

    const Entity from = infrastructureFirstBuildingByRoleInMacro(registry, macro, spec.from_role);
    const Entity to = infrastructureFirstBuildingByRoleInMacro(registry, macro, spec.to_role);
    if (from == MAX_ENTITIES || to == MAX_ENTITIES) return 0;
    if (pathExistsBetween(registry, from, to, spec.path_kind)) return 0;

    const Entity path = createPedestrianPath(registry, from, to);
    if (path == MAX_ENTITIES) return 0;

    createRouteSignpost(registry, path, from, to);
    createRouteSignpost(registry, path, to, from);
    return 1;
}

inline size_t deriveInfrastructure(Registry& registry, const WorldConfig& config) {
    (void)config;
    size_t created = 0;
    auto macros = registry.view<MacroZoneComponent>();
    if (macros.empty()) {
        created += derivePedestrianPaths(registry, kHousingToWorkplacePedestrianAccess);
        created += derivePedestrianPaths(registry, kWorkplaceToSupplyPedestrianAccess);
        return created;
    }
    for (Entity macro : macros) {
        created += derivePedestrianPathsInMacro(registry,
                                                macro,
                                                kHousingToWorkplacePedestrianAccess);
    }
    for (Entity macro : macros) {
        created += derivePedestrianPathsInMacro(registry,
                                                macro,
                                                kWorkplaceToSupplyPedestrianAccess);
    }
    return created;
}
