#include <cassert>
#include "target_selection.h"

static Entity makeEntity(Registry& registry, float x, float y) {
    Entity e = registry.create();
    registry.assign<TransformComponent>(e, x, y, 8.0f, 8.0f);
    return e;
}

static Entity makePlayer(Registry& registry, float x, float y, Facing facing) {
    Entity e = makeEntity(registry, x, y);
    auto& player = registry.assign<PlayerComponent>(e);
    player.facing = facing;
    return e;
}

static void testNearestScanTargetPrefersFacingDirection() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity side = makeEntity(registry, 0.0f, 10.0f);
    Entity forward = makeEntity(registry, 40.0f, 0.0f);

    (void)side;
    assert(nearestScanTarget(registry, player, 100.0f) == forward);
}

static void testDirectionalCursorMovesByArrowDirection() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity current = makeEntity(registry, 10.0f, 0.0f);
    Entity up = makeEntity(registry, 10.0f, -30.0f);
    Entity right = makeEntity(registry, 50.0f, 0.0f);

    (void)right;
    assert(directionalScanTarget(registry, player, current, Facing::UP, 100.0f) == up);
}

static void testDirectionalCursorClearsWhenNoDirectionalTargetExists() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity current = makeEntity(registry, 20.0f, 0.0f);
    Entity outOfRange = makeEntity(registry, 200.0f, 0.0f);

    (void)outOfRange;
    assert(directionalScanTarget(registry, player, current, Facing::RIGHT, 100.0f) == MAX_ENTITIES);
}

static void testDirectionalCursorClearsOnlyTargetWhenPressedAgain() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity only = makeEntity(registry, 20.0f, 0.0f);

    assert(directionalScanTarget(registry, player, only, Facing::RIGHT, 100.0f) == MAX_ENTITIES);
}

static void testDirectionalCursorCanRecoverFromEmptySelection() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity right = makeEntity(registry, 35.0f, 0.0f);

    assert(directionalScanTarget(registry, player, MAX_ENTITIES, Facing::RIGHT, 100.0f) == right);
}

static void testScanTargetRangeValidationFollowsMovingTarget() {
    Registry registry;
    Entity player = makePlayer(registry, 0.0f, 0.0f, Facing::RIGHT);
    Entity target = makeEntity(registry, 30.0f, 0.0f);

    assert(scanTargetInRange(registry, player, target, 100.0f));
    registry.get<TransformComponent>(target).x = 130.0f;
    assert(!scanTargetInRange(registry, player, target, 100.0f));
}

int main() {
    testNearestScanTargetPrefersFacingDirection();
    testDirectionalCursorMovesByArrowDirection();
    testDirectionalCursorClearsWhenNoDirectionalTargetExists();
    testDirectionalCursorClearsOnlyTargetWhenPressedAgain();
    testDirectionalCursorCanRecoverFromEmptySelection();
    testScanTargetRangeValidationFollowsMovingTarget();
    return 0;
}
