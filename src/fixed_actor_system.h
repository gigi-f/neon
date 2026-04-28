#pragma once

#include <algorithm>
#include <cmath>
#include "components.h"
#include "ecs.h"
#include "world_config.h"

inline int configuredFixedWorkerCount(const WorldConfig& config) {
    return std::clamp(config.fixed_worker_count, 0, 1);
}

inline Entity firstPedestrianPath(Registry& registry) {
    auto paths = registry.view<PathComponent>();
    for (Entity path : paths) {
        const auto& component = registry.get<PathComponent>(path);
        if (component.kind != PathKind::PEDESTRIAN) continue;
        if (registry.alive(component.from) &&
            registry.alive(component.to) &&
            registry.has<BuildingUseComponent>(component.from) &&
            registry.has<BuildingUseComponent>(component.to) &&
            registry.get<BuildingUseComponent>(component.from).role == MicroZoneRole::HOUSING &&
            registry.get<BuildingUseComponent>(component.to).role == MicroZoneRole::WORKPLACE) {
            return path;
        }
    }
    for (Entity path : paths) {
        if (registry.get<PathComponent>(path).kind == PathKind::PEDESTRIAN) {
            return path;
        }
    }
    return MAX_ENTITIES;
}

inline bool pathHasSpoofedRouteSignpost(Registry& registry, Entity path_entity) {
    if (!registry.alive(path_entity)) {
        return false;
    }

    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        const auto& signpost = registry.get<RouteSignpostComponent>(marker);
        if (signpost.path_entity == path_entity && signpost.spoofed) {
            return true;
        }
    }
    return false;
}

inline bool pathConnectsRoles(Registry& registry,
                              Entity path_entity,
                              MicroZoneRole a,
                              MicroZoneRole b) {
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

    const MicroZoneRole from = registry.get<BuildingUseComponent>(path.from).role;
    const MicroZoneRole to = registry.get<BuildingUseComponent>(path.to).role;
    return (from == a && to == b) || (from == b && to == a);
}

inline bool pathHasDisruptedDependency(Registry& registry, Entity path_entity) {
    auto disruptions = registry.view<DependencyDisruptionComponent>();
    for (Entity disruption : disruptions) {
        const auto& component = registry.get<DependencyDisruptionComponent>(disruption);
        if (component.disrupted &&
            pathConnectsRoles(registry,
                              path_entity,
                              component.dependent_role,
                              component.provider_role)) {
            return true;
        }
    }
    return false;
}

inline bool workerCurrentPathHasSpoofedRouteSignpost(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }
    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    return worker_component.kind == FixedActorKind::WORKER &&
           pathHasSpoofedRouteSignpost(registry, worker_component.path_entity);
}

inline bool workerCurrentPathHasDisruptedDependency(Registry& registry, Entity worker) {
    if (!registry.alive(worker) || !registry.has<FixedActorComponent>(worker)) {
        return false;
    }
    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    return worker_component.kind == FixedActorKind::WORKER &&
           pathHasDisruptedDependency(registry, worker_component.path_entity);
}

inline TransformComponent transformOnPath(const TransformComponent& path, float route_t) {
    const float t = std::clamp(route_t, 0.0f, 1.0f);
    const bool vertical = path.height >= path.width;
    constexpr float actor_size = 10.0f;
    constexpr float actor_half = actor_size * 0.5f;
    const float half_span = (vertical ? path.height : path.width) * 0.5f;
    const float travel_half_span = std::max(0.0f, half_span - actor_half);
    const float offset = -travel_half_span + travel_half_span * 2.0f * t;

    TransformComponent transform;
    transform.x = vertical ? path.x : path.x + offset;
    transform.y = vertical ? path.y + offset : path.y;
    transform.width = actor_size;
    transform.height = actor_size;
    return transform;
}

inline float routeTForPathEndpoint(Registry& registry, Entity path_entity, Entity endpoint) {
    if (!registry.alive(path_entity) || !registry.alive(endpoint) ||
        !registry.has<TransformComponent>(path_entity) ||
        !registry.has<TransformComponent>(endpoint)) {
        return 0.0f;
    }

    const auto& path = registry.get<TransformComponent>(path_entity);
    const auto& building = registry.get<TransformComponent>(endpoint);
    const bool vertical = path.height >= path.width;
    if (vertical) {
        return building.y <= path.y ? 0.0f : 1.0f;
    }
    return building.x <= path.x ? 0.0f : 1.0f;
}

inline Entity pathEndpointAtRouteT(Registry& registry, Entity path_entity, float route_t) {
    if (!registry.alive(path_entity) || !registry.has<PathComponent>(path_entity)) {
        return MAX_ENTITIES;
    }

    const auto& path = registry.get<PathComponent>(path_entity);
    const float from_t = routeTForPathEndpoint(registry, path_entity, path.from);
    return std::fabs(from_t - route_t) < 0.5f ? path.from : path.to;
}

inline Entity nextPedestrianPathFromEndpoint(Registry& registry, Entity current_path, Entity endpoint) {
    auto paths = registry.view<PathComponent>();
    for (Entity path : paths) {
        if (path == current_path) continue;
        const auto& component = registry.get<PathComponent>(path);
        if (component.kind != PathKind::PEDESTRIAN) continue;
        if (component.from == endpoint || component.to == endpoint) {
            return path;
        }
    }
    return MAX_ENTITIES;
}

inline bool moveActorToConnectedPath(Registry& registry,
                                     FixedActorComponent& component,
                                     TransformComponent& transform,
                                     float endpoint_t) {
    const Entity endpoint = pathEndpointAtRouteT(registry, component.path_entity, endpoint_t);
    if (endpoint == MAX_ENTITIES) return false;

    const Entity next_path = nextPedestrianPathFromEndpoint(registry, component.path_entity, endpoint);
    if (next_path == MAX_ENTITIES || !registry.has<TransformComponent>(next_path)) return false;

    component.path_entity = next_path;
    component.route_t = routeTForPathEndpoint(registry, next_path, endpoint);
    component.direction = component.route_t <= 0.5f ? 1.0f : -1.0f;
    transform = transformOnPath(registry.get<TransformComponent>(next_path), component.route_t);
    return true;
}

inline Entity spawnFixedWorker(Registry& registry, Entity path_entity) {
    if (!registry.alive(path_entity) || !registry.has<TransformComponent>(path_entity)) {
        return MAX_ENTITIES;
    }

    Entity actor = registry.create();
    registry.assign<TransformComponent>(actor,
        transformOnPath(registry.get<TransformComponent>(path_entity), 0.0f));
    registry.assign<FixedActorComponent>(actor,
        FixedActorKind::WORKER, path_entity, MAX_ENTITIES, 0.0f, 1.0f, 24.0f);
    registry.assign<GlyphComponent>(actor, "a",
        static_cast<uint8_t>(245), static_cast<uint8_t>(235), static_cast<uint8_t>(130),
        static_cast<uint8_t>(255), 1.0f, true, false);
    return actor;
}

inline size_t spawnFixedActors(Registry& registry, const WorldConfig& config) {
    const int worker_count = configuredFixedWorkerCount(config);
    if (worker_count <= 0) return 0;

    const auto existing_workers = registry.view<FixedActorComponent>();
    if (existing_workers.size() >= static_cast<size_t>(worker_count)) return 0;

    const Entity path = firstPedestrianPath(registry);
    if (path == MAX_ENTITIES) return 0;

    size_t spawned = 0;
    const size_t remaining = static_cast<size_t>(worker_count) - existing_workers.size();
    for (size_t i = 0; i < remaining; ++i) {
        if (spawnFixedWorker(registry, path) != MAX_ENTITIES) {
            ++spawned;
        }
    }
    return spawned;
}

inline void updateFixedActors(Registry& registry, float dt) {
    auto actors = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity actor : actors) {
        auto& component = registry.get<FixedActorComponent>(actor);
        if (component.kind == FixedActorKind::WORKER &&
            component.carried_object != MAX_ENTITIES) {
            continue;
        }
        if (!registry.alive(component.path_entity) ||
            !registry.has<TransformComponent>(component.path_entity)) {
            continue;
        }
        if (pathHasSpoofedRouteSignpost(registry, component.path_entity)) {
            continue;
        }
        if (pathHasDisruptedDependency(registry, component.path_entity)) {
            continue;
        }

        const auto& path = registry.get<TransformComponent>(component.path_entity);
        const float span = std::max(path.width, path.height);
        if (span <= 0.0f) continue;

        component.route_t += component.direction * (component.speed_wu / span) * dt;
        auto& transform = registry.get<TransformComponent>(actor);
        if (component.route_t >= 1.0f) {
            component.route_t = 1.0f;
            if (moveActorToConnectedPath(registry, component, transform, 1.0f)) {
                continue;
            }
            component.direction = -1.0f;
        } else if (component.route_t <= 0.0f) {
            component.route_t = 0.0f;
            if (moveActorToConnectedPath(registry, component, transform, 0.0f)) {
                continue;
            }
            component.direction = 1.0f;
        }

        transform = transformOnPath(path, component.route_t);
    }
}
