#include <cassert>
#include <cmath>
#include "ecs.h"
#include "components.h"
#include "simulation_systems.h"

static Entity makePlayer(Registry& registry) {
    Entity player = registry.create();
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 8.0f, 8.0f);
    registry.assign<BiologyComponent>(player);
    registry.assign<SurvivalInventoryComponent>(player);
    return player;
}

static Entity makeItem(Registry& registry, ItemComponent::Type type, float x, float y) {
    Entity item = registry.create();
    registry.assign<TransformComponent>(item, x, y, 8.0f, 8.0f);
    registry.assign<ItemComponent>(item, type, 40.0f);
    return item;
}

static void testCollectFoodIncrementsCounterWithoutRestoringHunger() {
    Registry registry;
    ConsumableSystem consumables;
    Entity player = makePlayer(registry);
    registry.get<BiologyComponent>(player).hunger = 10.0f;
    Entity food = makeItem(registry, ItemComponent::FOOD, 10.0f, 0.0f);

    auto [collected, type] = consumables.playerCollectWithType(registry, player);

    assert(collected);
    assert(type == ItemComponent::FOOD);
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 1);
    assert(registry.get<BiologyComponent>(player).hunger == 10.0f);
    assert(!registry.alive(food));
}

static void testConsumeWaterRestoresThirstAndDecrementsCounter() {
    Registry registry;
    ConsumableSystem consumables;
    Entity player = makePlayer(registry);
    auto& inv = registry.get<SurvivalInventoryComponent>(player);
    inv.water_count = 2;
    registry.get<BiologyComponent>(player).thirst = 30.0f;

    bool consumed = consumables.consumePlayerInventory(registry, player, ItemComponent::WATER);

    assert(consumed);
    assert(inv.water_count == 1);
    assert(registry.get<BiologyComponent>(player).thirst == 70.0f);
}

static void testEmptySlotDoesNothing() {
    Registry registry;
    ConsumableSystem consumables;
    Entity player = makePlayer(registry);
    registry.get<BiologyComponent>(player).hunger = 30.0f;

    bool consumed = consumables.consumePlayerInventory(registry, player, ItemComponent::FOOD);

    assert(!consumed);
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 0);
    assert(registry.get<BiologyComponent>(player).hunger == 30.0f);
}

static void testMedicalInventoryTreatsWorstInjuryBeforeHealth() {
    Registry registry;
    ConsumableSystem consumables;
    Entity player = makePlayer(registry);
    registry.get<SurvivalInventoryComponent>(player).medical_count = 1;
    registry.get<BiologyComponent>(player).health = 40.0f;

    InjuryComponent injuries;
    injuries.slots[0] = {InjuryType::LACERATION, 0.2f};
    injuries.slots[1] = {InjuryType::INTERNAL_BLEEDING, 0.8f};
    registry.assign<InjuryComponent>(player, injuries);

    bool consumed = consumables.consumePlayerInventory(registry, player, ItemComponent::MEDICAL);

    assert(consumed);
    assert(registry.get<SurvivalInventoryComponent>(player).medical_count == 0);
    assert(registry.get<BiologyComponent>(player).health == 40.0f);
    assert(std::fabs(registry.get<InjuryComponent>(player).slots[1].severity - 0.3f) < 0.001f);
}

int main() {
    testCollectFoodIncrementsCounterWithoutRestoringHunger();
    testConsumeWaterRestoresThirstAndDecrementsCounter();
    testEmptySlotDoesNothing();
    testMedicalInventoryTreatsWorstInjuryBeforeHealth();
    return 0;
}
