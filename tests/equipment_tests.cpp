#include <cassert>
#include "equipment.h"

static void testDefaultHotkeysEquipToolsAndSupplies() {
    EquipmentComponent equipment;

    assert(equipment.hotkeys[0] == EquipmentSlot::NONE);
    assert(equipment.hotkeys[1] == EquipmentSlot::SURFACE_SCAN);
    assert(equipment.hotkeys[2] == EquipmentSlot::BIOLOGY_AUDIT);
    assert(equipment.hotkeys[3] == EquipmentSlot::COGNITIVE_PROFILE);
    assert(equipment.hotkeys[4] == EquipmentSlot::FINANCIAL_FORENSICS);
    assert(equipment.hotkeys[5] == EquipmentSlot::STRUCTURAL_ANALYSIS);
    assert(equipment.hotkeys[6] == EquipmentSlot::FOOD);
    assert(equipment.hotkeys[7] == EquipmentSlot::WATER);
    assert(equipment.hotkeys[8] == EquipmentSlot::MEDICAL);
    assert(equipment.hotkeys[9] == EquipmentSlot::NONE);
}

static void testConsumableSlotsMapToItemTypes() {
    assert(isConsumableEquipment(EquipmentSlot::FOOD));
    assert(isConsumableEquipment(EquipmentSlot::WATER));
    assert(isConsumableEquipment(EquipmentSlot::MEDICAL));
    assert(equipmentItemType(EquipmentSlot::FOOD) == ItemComponent::FOOD);
    assert(equipmentItemType(EquipmentSlot::WATER) == ItemComponent::WATER);
    assert(equipmentItemType(EquipmentSlot::MEDICAL) == ItemComponent::MEDICAL);
}

static void testScanSlotsMapToInventoryToolTypes() {
    assert(equipmentInventoryType(EquipmentSlot::SURFACE_SCAN) == ItemComponent::SURFACE_SCAN_TOOL);
    assert(equipmentInventoryType(EquipmentSlot::BIOLOGY_AUDIT) == ItemComponent::BIOLOGY_AUDIT_TOOL);
    assert(equipmentFromInventoryType(ItemComponent::COGNITIVE_PROFILE_TOOL) == EquipmentSlot::COGNITIVE_PROFILE);
    assert(equipmentFromInventoryType(ItemComponent::FINANCIAL_FORENSICS_TOOL) == EquipmentSlot::FINANCIAL_FORENSICS);
    assert(equipmentFromInventoryType(ItemComponent::STRUCTURAL_ANALYSIS_TOOL) == EquipmentSlot::STRUCTURAL_ANALYSIS);
}

static void testScanToolsHaveDistinctPositiveRanges() {
    assert(isScanEquipment(EquipmentSlot::SURFACE_SCAN));
    assert(isScanEquipment(EquipmentSlot::BIOLOGY_AUDIT));
    assert(equipmentRange(EquipmentSlot::SURFACE_SCAN) > equipmentRange(EquipmentSlot::BIOLOGY_AUDIT));
    assert(equipmentRange(EquipmentSlot::STRUCTURAL_ANALYSIS) > 0.0f);
    assert(equipmentRange(EquipmentSlot::NONE) == 0.0f);
}

static void testHotkeyAssignmentRespectsToolAndItemRanges() {
    EquipmentComponent equipment;

    assert(assignEquipmentHotkey(equipment, 4, EquipmentSlot::SURFACE_SCAN));
    assert(equipment.hotkeys[4] == EquipmentSlot::SURFACE_SCAN);

    assert(assignEquipmentHotkey(equipment, 9, EquipmentSlot::WATER));
    assert(equipment.hotkeys[9] == EquipmentSlot::WATER);
}

static void testHotkeyAssignmentRejectsWrongRanges() {
    EquipmentComponent equipment;

    assert(!assignEquipmentHotkey(equipment, 4, EquipmentSlot::WATER));
    assert(equipment.hotkeys[4] == EquipmentSlot::FINANCIAL_FORENSICS);

    assert(!assignEquipmentHotkey(equipment, 9, EquipmentSlot::STRUCTURAL_ANALYSIS));
    assert(equipment.hotkeys[9] == EquipmentSlot::NONE);
}

static void testHotkeyAssignmentKeepsZeroReserved() {
    EquipmentComponent equipment;

    assert(!assignEquipmentHotkey(equipment, 0, EquipmentSlot::FOOD));
    assert(equipment.hotkeys[0] == EquipmentSlot::NONE);

    assert(!assignEquipmentHotkey(equipment, equipment.hotkeys.size(), EquipmentSlot::FOOD));
}

int main() {
    testDefaultHotkeysEquipToolsAndSupplies();
    testConsumableSlotsMapToItemTypes();
    testScanSlotsMapToInventoryToolTypes();
    testScanToolsHaveDistinctPositiveRanges();
    testHotkeyAssignmentRespectsToolAndItemRanges();
    testHotkeyAssignmentRejectsWrongRanges();
    testHotkeyAssignmentKeepsZeroReserved();
    return 0;
}
