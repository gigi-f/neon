#pragma once

#include <array>
#include <cstddef>
#include "components.h"

enum class EquipmentSlot {
    NONE,
    FOOD,
    WATER,
    MEDICAL,
    SURFACE_SCAN,
    BIOLOGY_AUDIT,
    COGNITIVE_PROFILE,
    FINANCIAL_FORENSICS,
    STRUCTURAL_ANALYSIS
};

struct EquipmentComponent {
    EquipmentSlot equipped = EquipmentSlot::NONE;
    std::array<EquipmentSlot, 10> hotkeys = {
        EquipmentSlot::NONE,
        EquipmentSlot::SURFACE_SCAN,
        EquipmentSlot::BIOLOGY_AUDIT,
        EquipmentSlot::COGNITIVE_PROFILE,
        EquipmentSlot::FINANCIAL_FORENSICS,
        EquipmentSlot::STRUCTURAL_ANALYSIS,
        EquipmentSlot::FOOD,
        EquipmentSlot::WATER,
        EquipmentSlot::MEDICAL,
        EquipmentSlot::NONE
    };
};

inline const char* equipmentName(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::NONE:                return "NONE";
        case EquipmentSlot::FOOD:                return "FOOD";
        case EquipmentSlot::WATER:               return "WATER";
        case EquipmentSlot::MEDICAL:             return "MEDICAL";
        case EquipmentSlot::SURFACE_SCAN:        return "SURFACE SCAN";
        case EquipmentSlot::BIOLOGY_AUDIT:       return "BIO AUDIT";
        case EquipmentSlot::COGNITIVE_PROFILE:   return "COG PROFILE";
        case EquipmentSlot::FINANCIAL_FORENSICS: return "FIN FORENSICS";
        case EquipmentSlot::STRUCTURAL_ANALYSIS: return "STRUCT ANALYSIS";
    }
    return "NONE";
}

inline bool isConsumableEquipment(EquipmentSlot slot) {
    return slot == EquipmentSlot::FOOD ||
           slot == EquipmentSlot::WATER ||
           slot == EquipmentSlot::MEDICAL;
}

inline bool isScanEquipment(EquipmentSlot slot) {
    return slot == EquipmentSlot::SURFACE_SCAN ||
           slot == EquipmentSlot::BIOLOGY_AUDIT ||
           slot == EquipmentSlot::COGNITIVE_PROFILE ||
           slot == EquipmentSlot::FINANCIAL_FORENSICS ||
           slot == EquipmentSlot::STRUCTURAL_ANALYSIS;
}

inline bool isToolHotkeyIndex(std::size_t index) {
    return index >= 1 && index <= 5;
}

inline bool isItemHotkeyIndex(std::size_t index) {
    return index >= 6 && index <= 9;
}

inline bool canAssignEquipmentHotkey(std::size_t index, EquipmentSlot slot) {
    if (index == 0 || index >= 10) {
        return false;
    }
    if (slot == EquipmentSlot::NONE) {
        return true;
    }
    if (isScanEquipment(slot)) {
        return isToolHotkeyIndex(index);
    }
    if (isConsumableEquipment(slot)) {
        return isItemHotkeyIndex(index);
    }
    return false;
}

inline bool assignEquipmentHotkey(EquipmentComponent& equipment, std::size_t index, EquipmentSlot slot) {
    if (!canAssignEquipmentHotkey(index, slot)) {
        return false;
    }
    equipment.hotkeys[index] = slot;
    return true;
}

inline ItemComponent::Type equipmentItemType(EquipmentSlot slot) {
    if (slot == EquipmentSlot::WATER) return ItemComponent::WATER;
    if (slot == EquipmentSlot::MEDICAL) return ItemComponent::MEDICAL;
    return ItemComponent::FOOD;
}

inline ItemComponent::Type equipmentInventoryType(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::FOOD:                return ItemComponent::FOOD;
        case EquipmentSlot::WATER:               return ItemComponent::WATER;
        case EquipmentSlot::MEDICAL:             return ItemComponent::MEDICAL;
        case EquipmentSlot::SURFACE_SCAN:        return ItemComponent::SURFACE_SCAN_TOOL;
        case EquipmentSlot::BIOLOGY_AUDIT:       return ItemComponent::BIOLOGY_AUDIT_TOOL;
        case EquipmentSlot::COGNITIVE_PROFILE:   return ItemComponent::COGNITIVE_PROFILE_TOOL;
        case EquipmentSlot::FINANCIAL_FORENSICS: return ItemComponent::FINANCIAL_FORENSICS_TOOL;
        case EquipmentSlot::STRUCTURAL_ANALYSIS: return ItemComponent::STRUCTURAL_ANALYSIS_TOOL;
        case EquipmentSlot::NONE:                return ItemComponent::FOOD;
    }
    return ItemComponent::FOOD;
}

inline EquipmentSlot equipmentFromInventoryType(ItemComponent::Type type) {
    switch (type) {
        case ItemComponent::FOOD:                       return EquipmentSlot::FOOD;
        case ItemComponent::WATER:                      return EquipmentSlot::WATER;
        case ItemComponent::MEDICAL:                    return EquipmentSlot::MEDICAL;
        case ItemComponent::SURFACE_SCAN_TOOL:          return EquipmentSlot::SURFACE_SCAN;
        case ItemComponent::BIOLOGY_AUDIT_TOOL:         return EquipmentSlot::BIOLOGY_AUDIT;
        case ItemComponent::COGNITIVE_PROFILE_TOOL:     return EquipmentSlot::COGNITIVE_PROFILE;
        case ItemComponent::FINANCIAL_FORENSICS_TOOL:   return EquipmentSlot::FINANCIAL_FORENSICS;
        case ItemComponent::STRUCTURAL_ANALYSIS_TOOL:   return EquipmentSlot::STRUCTURAL_ANALYSIS;
    }
    return EquipmentSlot::NONE;
}

inline float equipmentRange(EquipmentSlot slot) {
    switch (slot) {
        case EquipmentSlot::SURFACE_SCAN:        return 150.0f;
        case EquipmentSlot::BIOLOGY_AUDIT:       return 120.0f;
        case EquipmentSlot::COGNITIVE_PROFILE:   return 115.0f;
        case EquipmentSlot::FINANCIAL_FORENSICS: return 130.0f;
        case EquipmentSlot::STRUCTURAL_ANALYSIS: return 160.0f;
        case EquipmentSlot::FOOD:
        case EquipmentSlot::WATER:
        case EquipmentSlot::MEDICAL:
        case EquipmentSlot::NONE:
            return 0.0f;
    }
    return 0.0f;
}
