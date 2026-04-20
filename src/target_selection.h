#pragma once

#include <cmath>
#include "components.h"
#include "ecs.h"

inline void scanFacingVector(Facing facing, float& x, float& y) {
    switch (facing) {
        case Facing::UP:    x =  0.0f; y = -1.0f; return;
        case Facing::DOWN:  x =  0.0f; y =  1.0f; return;
        case Facing::LEFT:  x = -1.0f; y =  0.0f; return;
        case Facing::RIGHT: x =  1.0f; y =  0.0f; return;
    }
    x = 0.0f;
    y = 1.0f;
}

inline Entity nearestScanTarget(Registry& registry, Entity player, float maxDistance) {
    if (!registry.has<TransformComponent>(player)) return MAX_ENTITIES;
    auto& pt = registry.get<TransformComponent>(player);
    const bool preferFacing = registry.has<PlayerComponent>(player);
    float faceX = 0.0f;
    float faceY = 1.0f;
    if (preferFacing)
        scanFacingVector(registry.get<PlayerComponent>(player).facing, faceX, faceY);

    Entity best = MAX_ENTITIES;
    float bestDistSq = maxDistance * maxDistance;
    bool bestInFacing = false;
    constexpr float FORWARD_CONE_COS = 0.35f;

    auto view = registry.view<TransformComponent>();
    for (Entity e : view) {
        if (e == player) continue;
        if (!registry.alive(e)) continue;
        auto& t = registry.get<TransformComponent>(e);
        float dx = t.x - pt.x;
        float dy = t.y - pt.y;
        float d2 = dx * dx + dy * dy;
        if (d2 >= maxDistance * maxDistance) continue;

        bool inFacing = false;
        if (preferFacing && d2 > 0.0001f) {
            float d = std::sqrt(d2);
            float facingDot = (dx * faceX + dy * faceY) / d;
            inFacing = facingDot >= FORWARD_CONE_COS;
        }

        if ((inFacing && !bestInFacing) ||
            (inFacing == bestInFacing && d2 < bestDistSq)) {
            bestDistSq = d2;
            bestInFacing = inFacing;
            best = e;
        }
    }
    return best;
}

inline bool scanTargetInRange(Registry& registry, Entity player, Entity target, float maxDistance) {
    if (target == MAX_ENTITIES || !registry.alive(target)) return false;
    if (!registry.has<TransformComponent>(player) || !registry.has<TransformComponent>(target)) return false;

    auto& pt = registry.get<TransformComponent>(player);
    auto& tt = registry.get<TransformComponent>(target);
    float dx = tt.x - pt.x;
    float dy = tt.y - pt.y;
    return dx * dx + dy * dy < maxDistance * maxDistance;
}

inline int scanTargetCountInRange(Registry& registry, Entity player, float maxDistance) {
    if (!registry.has<TransformComponent>(player)) return 0;

    auto& pt = registry.get<TransformComponent>(player);
    float maxDistanceSq = maxDistance * maxDistance;
    int count = 0;

    auto view = registry.view<TransformComponent>();
    for (Entity e : view) {
        if (e == player || !registry.alive(e)) continue;

        auto& t = registry.get<TransformComponent>(e);
        float dx = t.x - pt.x;
        float dy = t.y - pt.y;
        float d2 = dx * dx + dy * dy;
        if (d2 < maxDistanceSq) ++count;
    }
    return count;
}

inline Entity directionalScanTarget(Registry& registry, Entity player, Entity current,
                                    Facing direction, float maxDistance) {
    if (!registry.has<TransformComponent>(player)) return MAX_ENTITIES;
    if (current != MAX_ENTITIES && scanTargetCountInRange(registry, player, maxDistance) <= 1) {
        return MAX_ENTITIES;
    }

    auto& pt = registry.get<TransformComponent>(player);
    float originX = pt.x;
    float originY = pt.y;
    if (scanTargetInRange(registry, player, current, maxDistance)) {
        auto& ct = registry.get<TransformComponent>(current);
        originX = ct.x;
        originY = ct.y;
    }

    float dirX = 0.0f;
    float dirY = 1.0f;
    scanFacingVector(direction, dirX, dirY);

    Entity best = MAX_ENTITIES;
    float bestScore = 1.0e30f;
    float maxDistanceSq = maxDistance * maxDistance;

    auto view = registry.view<TransformComponent>();
    for (Entity e : view) {
        if (e == player || e == current) continue;
        if (!registry.alive(e)) continue;

        auto& t = registry.get<TransformComponent>(e);
        float playerDx = t.x - pt.x;
        float playerDy = t.y - pt.y;
        float playerDistSq = playerDx * playerDx + playerDy * playerDy;
        if (playerDistSq >= maxDistanceSq) continue;

        float dx = t.x - originX;
        float dy = t.y - originY;
        float projection = dx * dirX + dy * dirY;
        if (projection <= 0.001f) continue;

        float perpendicularX = dx - projection * dirX;
        float perpendicularY = dy - projection * dirY;
        float perpendicularSq = perpendicularX * perpendicularX + perpendicularY * perpendicularY;
        float score = perpendicularSq * 4.0f + projection * projection;
        if (score < bestScore) {
            bestScore = score;
            best = e;
        }
    }

    return best;
}
