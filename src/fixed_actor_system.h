#pragma once

#include <algorithm>
#include "components.h"
#include "ecs.h"
#include "world_config.h"

inline int configuredFixedWorkerCount(const WorldConfig& config) {
    return std::clamp(config.fixed_worker_count, 0, 1);
}

inline Entity firstPedestrianPath(Registry& registry) {
    auto paths = registry.view<PathComponent>();
    for (Entity path : paths) {
        if (registry.get<PathComponent>(path).kind == PathKind::PEDESTRIAN) {
            return path;
        }
    }
    return MAX_ENTITIES;
}

inline TransformComponent transformOnPath(const TransformComponent& path, float route_t) {
    const float t = std::clamp(route_t, 0.0f, 1.0f);
    const bool vertical = path.height >= path.width;
    const float half_span = (vertical ? path.height : path.width) * 0.5f;
    const float offset = -half_span + half_span * 2.0f * t;

    TransformComponent transform;
    transform.x = vertical ? path.x : path.x + offset;
    transform.y = vertical ? path.y + offset : path.y;
    transform.width = 10.0f;
    transform.height = 10.0f;
    return transform;
}

inline Entity spawnFixedWorker(Registry& registry, Entity path_entity) {
    if (!registry.alive(path_entity) || !registry.has<TransformComponent>(path_entity)) {
        return MAX_ENTITIES;
    }

    Entity actor = registry.create();
    registry.assign<TransformComponent>(actor,
        transformOnPath(registry.get<TransformComponent>(path_entity), 0.0f));
    registry.assign<FixedActorComponent>(actor,
        FixedActorKind::WORKER, path_entity, 0.0f, 1.0f, 24.0f);
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
        if (!registry.alive(component.path_entity) ||
            !registry.has<TransformComponent>(component.path_entity)) {
            continue;
        }

        const auto& path = registry.get<TransformComponent>(component.path_entity);
        const float span = std::max(path.width, path.height);
        if (span <= 0.0f) continue;

        component.route_t += component.direction * (component.speed_wu / span) * dt;
        if (component.route_t >= 1.0f) {
            component.route_t = 1.0f;
            component.direction = -1.0f;
        } else if (component.route_t <= 0.0f) {
            component.route_t = 0.0f;
            component.direction = 1.0f;
        }

        registry.get<TransformComponent>(actor) = transformOnPath(path, component.route_t);
    }
}
