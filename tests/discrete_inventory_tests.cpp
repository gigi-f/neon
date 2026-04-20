#include <cassert>
#include <cmath>
#include <string>
#include "ecs.h"
#include "components.h"
#include "inventory.h"

static Entity makePlayer(Registry& registry) {
    Entity player = registry.create();
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 8.0f, 8.0f);
    registry.assign<BiologyComponent>(player);
    registry.assign<SurvivalInventoryComponent>(player);
    registry.assign<DiscreteInventoryComponent>(player);
    registry.assign<EquipmentComponent>(player);
    return player;
}

static Entity makeItem(Registry& registry, ItemComponent::Type type, float x, float y,
                       float restore = 40.0f, uint32_t flags = ITEM_FLAG_LEGAL) {
    Entity item = registry.create();
    registry.assign<TransformComponent>(item, x, y, 8.0f, 8.0f);
    registry.assign<ItemComponent>(item, type, restore, flags);
    return item;
}

static void testPickupStoresNearestItemAndIncrementsCounter() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity farWater = makeItem(registry, ItemComponent::WATER, 24.0f, 0.0f);
    Entity nearFood = makeItem(registry, ItemComponent::FOOD, 8.0f, 0.0f, 35.0f,
                               ITEM_FLAG_ILLEGAL | ITEM_FLAG_HIGH_VALUE);

    auto result = collectNearestInventoryItem(registry, player);

    assert(result.picked_up);
    assert(result.item == nearFood);
    assert(result.slot == 0);
    assert(result.type == ItemComponent::FOOD);
    assert(!registry.alive(nearFood));
    assert(registry.alive(farWater));
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 1);

    auto inspected = inspectSelectedInventoryItem(registry.get<DiscreteInventoryComponent>(player));
    assert(inspected.present);
    assert(inspected.type == ItemComponent::FOOD);
    assert(std::fabs(inspected.restore_value - 35.0f) < 0.001f);
    assert((inspected.flags & ITEM_FLAG_ILLEGAL) != 0);
    assert((inspected.flags & ITEM_FLAG_HIGH_VALUE) != 0);
}

static void testBaselineScanToolsOccupyInventoryEntries() {
    DiscreteInventoryComponent inventory;

    bool added = addBaselineScanTools(inventory);

    assert(added);
    assert(occupiedInventorySlots(inventory) == 5);
    assert(inventory.slots[0].type == ItemComponent::SURFACE_SCAN_TOOL);
    assert(inventory.slots[4].type == ItemComponent::STRUCTURAL_ANALYSIS_TOOL);
    assert(isScanToolItemType(inventory.slots[2].type));
}

static void testScanToolPickupDoesNotIncrementSurvivalCounters() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity tool = makeItem(registry, ItemComponent::SURFACE_SCAN_TOOL, 4.0f, 0.0f, 0.0f,
                           ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE);

    auto result = collectNearestInventoryItem(registry, player);

    assert(result.picked_up);
    assert(result.item == tool);
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 0);
    assert(registry.get<SurvivalInventoryComponent>(player).water_count == 0);
    assert(registry.get<SurvivalInventoryComponent>(player).medical_count == 0);
    assert(hasInventoryItemForEquipment(registry, player, EquipmentSlot::SURFACE_SCAN));
}

static void testSelectedScanToolEquipsWithoutConsumingSlot() {
    Registry registry;
    Entity player = makePlayer(registry);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    assert(storeInventoryItem(inventory, ItemComponent::COGNITIVE_PROFILE_TOOL, 0.0f,
                              ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE));

    EquipmentSlot equipped = equipSelectedInventoryTool(registry, player);

    assert(equipped == EquipmentSlot::COGNITIVE_PROFILE);
    assert(registry.get<EquipmentComponent>(player).equipped == EquipmentSlot::COGNITIVE_PROFILE);
    assert(inventory.slots[0].occupied);
    assert(inventory.slots[0].type == ItemComponent::COGNITIVE_PROFILE_TOOL);
    assert(!useSelectedInventoryItem(registry, player));
    assert(inventory.slots[0].occupied);
}

static void testPickupFailsWhenInventoryIsFull() {
    Registry registry;
    Entity player = makePlayer(registry);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    for (auto& slot : inventory.slots) {
        slot.occupied = true;
        slot.type = ItemComponent::FOOD;
    }
    Entity water = makeItem(registry, ItemComponent::WATER, 4.0f, 0.0f);

    auto result = collectNearestInventoryItem(registry, player);

    assert(!result.picked_up);
    assert(registry.alive(water));
    assert(registry.get<SurvivalInventoryComponent>(player).water_count == 0);
}

static void testUseSelectedItemRestoresBiologyAndClearsSlot() {
    Registry registry;
    Entity player = makePlayer(registry);
    registry.get<BiologyComponent>(player).thirst = 25.0f;
    makeItem(registry, ItemComponent::WATER, 6.0f, 0.0f);
    assert(collectNearestInventoryItem(registry, player).picked_up);

    bool used = useSelectedInventoryItem(registry, player);

    assert(used);
    assert(registry.get<BiologyComponent>(player).thirst == 65.0f);
    assert(registry.get<SurvivalInventoryComponent>(player).water_count == 0);
    assert(!inspectSelectedInventoryItem(registry.get<DiscreteInventoryComponent>(player)).present);
}

static void testQuickUseConsumesFirstMatchingDiscreteItem() {
    Registry registry;
    Entity player = makePlayer(registry);
    registry.get<BiologyComponent>(player).hunger = 20.0f;
    makeItem(registry, ItemComponent::WATER, 6.0f, 0.0f);
    assert(collectNearestInventoryItem(registry, player).picked_up);
    makeItem(registry, ItemComponent::FOOD, 6.0f, 0.0f, 30.0f);
    assert(collectNearestInventoryItem(registry, player).picked_up);

    bool used = useFirstInventoryItemOfType(registry, player, ItemComponent::FOOD);

    assert(used);
    assert(registry.get<BiologyComponent>(player).hunger == 50.0f);
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 0);
    assert(registry.get<SurvivalInventoryComponent>(player).water_count == 1);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    assert(inventory.selected == 1);
    assert(!inventory.slots[1].occupied);
    assert(inventory.slots[0].occupied);
    assert(inventory.slots[0].type == ItemComponent::WATER);
}

static void testDropSelectedItemCreatesWorldItemAndClearsSlot() {
    Registry registry;
    Entity player = makePlayer(registry);
    makeItem(registry, ItemComponent::MEDICAL, 5.0f, 0.0f, 50.0f,
             ITEM_FLAG_UNIQUE | ITEM_FLAG_FACTION_RELEVANT);
    assert(collectNearestInventoryItem(registry, player).picked_up);

    Entity dropped = dropSelectedInventoryItem(registry, player);

    assert(dropped != MAX_ENTITIES);
    assert(registry.alive(dropped));
    assert(registry.get<ItemComponent>(dropped).type == ItemComponent::MEDICAL);
    assert(std::fabs(registry.get<ItemComponent>(dropped).restore_value - 50.0f) < 0.001f);
    assert((registry.get<ItemComponent>(dropped).flags & ITEM_FLAG_UNIQUE) != 0);
    assert((registry.get<ItemComponent>(dropped).flags & ITEM_FLAG_FACTION_RELEVANT) != 0);
    assert(registry.get<SurvivalInventoryComponent>(player).medical_count == 0);
    assert(!inspectSelectedInventoryItem(registry.get<DiscreteInventoryComponent>(player)).present);
}

static void testItemFlagSummaryLabelsRiskAndValue() {
    std::string summary = itemFlagSummary(
        ITEM_FLAG_ILLEGAL | ITEM_FLAG_UNIQUE | ITEM_FLAG_HIGH_VALUE | ITEM_FLAG_FACTION_RELEVANT);

    assert(summary.find("ILLEGAL") != std::string::npos);
    assert(summary.find("UNIQUE") != std::string::npos);
    assert(summary.find("HIGH") != std::string::npos);
    assert(summary.find("FACTION") != std::string::npos);
}

int main() {
    testPickupStoresNearestItemAndIncrementsCounter();
    testBaselineScanToolsOccupyInventoryEntries();
    testScanToolPickupDoesNotIncrementSurvivalCounters();
    testSelectedScanToolEquipsWithoutConsumingSlot();
    testPickupFailsWhenInventoryIsFull();
    testUseSelectedItemRestoresBiologyAndClearsSlot();
    testQuickUseConsumesFirstMatchingDiscreteItem();
    testDropSelectedItemCreatesWorldItemAndClearsSlot();
    testItemFlagSummaryLabelsRiskAndValue();
    return 0;
}
