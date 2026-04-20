#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include "components.h"
#include "equipment.h"

struct InventoryInspection {
    bool present = false;
    ItemComponent::Type type = ItemComponent::FOOD;
    float restore_value = 0.0f;
    uint32_t flags = 0;
};

struct InventoryPickupResult {
    bool picked_up = false;
    Entity item = MAX_ENTITIES;
    size_t slot = DiscreteInventoryComponent::CAPACITY;
    ItemComponent::Type type = ItemComponent::FOOD;
};

inline const char* itemTypeName(ItemComponent::Type type) {
    switch (type) {
        case ItemComponent::FOOD:                       return "FOOD";
        case ItemComponent::WATER:                      return "WATER";
        case ItemComponent::MEDICAL:                    return "MEDICAL";
        case ItemComponent::SURFACE_SCAN_TOOL:          return "SURFACE SCAN";
        case ItemComponent::BIOLOGY_AUDIT_TOOL:         return "BIO AUDIT";
        case ItemComponent::COGNITIVE_PROFILE_TOOL:     return "COG PROFILE";
        case ItemComponent::FINANCIAL_FORENSICS_TOOL:   return "FIN FORENSICS";
        case ItemComponent::STRUCTURAL_ANALYSIS_TOOL:   return "STRUCT ANALYSIS";
    }
    return "ITEM";
}

inline bool isConsumableItemType(ItemComponent::Type type) {
    return type == ItemComponent::FOOD ||
           type == ItemComponent::WATER ||
           type == ItemComponent::MEDICAL;
}

inline bool isScanToolItemType(ItemComponent::Type type) {
    return type == ItemComponent::SURFACE_SCAN_TOOL ||
           type == ItemComponent::BIOLOGY_AUDIT_TOOL ||
           type == ItemComponent::COGNITIVE_PROFILE_TOOL ||
           type == ItemComponent::FINANCIAL_FORENSICS_TOOL ||
           type == ItemComponent::STRUCTURAL_ANALYSIS_TOOL;
}

inline bool itemHasFlag(const CarriedItem& item, ItemFlag flag) {
    return (item.flags & static_cast<uint32_t>(flag)) != 0;
}

inline std::string itemFlagSummary(uint32_t flags) {
    std::string summary;
    auto append = [&](const char* label) {
        if (!summary.empty()) summary += ",";
        summary += label;
    };

    if ((flags & ITEM_FLAG_ILLEGAL) != 0) append("ILLEGAL");
    if ((flags & ITEM_FLAG_LEGAL) != 0) append("LEGAL");
    if ((flags & ITEM_FLAG_UNIQUE) != 0) append("UNIQUE");
    if ((flags & ITEM_FLAG_HIGH_VALUE) != 0) append("HIGH");
    if ((flags & ITEM_FLAG_FACTION_RELEVANT) != 0) append("FACTION");
    return summary.empty() ? "NONE" : summary;
}

inline size_t occupiedInventorySlots(const DiscreteInventoryComponent& inventory) {
    size_t count = 0;
    for (const auto& slot : inventory.slots) {
        if (slot.occupied) ++count;
    }
    return count;
}

inline int firstEmptyInventorySlot(const DiscreteInventoryComponent& inventory) {
    for (size_t i = 0; i < inventory.slots.size(); ++i) {
        if (!inventory.slots[i].occupied) return static_cast<int>(i);
    }
    return -1;
}

inline int selectedInventorySlot(const DiscreteInventoryComponent& inventory) {
    if (inventory.selected >= inventory.slots.size()) return -1;
    return static_cast<int>(inventory.selected);
}

inline void selectInventorySlot(DiscreteInventoryComponent& inventory, int delta) {
    constexpr int capacity = static_cast<int>(DiscreteInventoryComponent::CAPACITY);
    int next = static_cast<int>(inventory.selected) + delta;
    while (next < 0) next += capacity;
    inventory.selected = static_cast<size_t>(next % capacity);
}

inline void incrementSurvivalCounter(SurvivalInventoryComponent& inventory, ItemComponent::Type type) {
    if (type == ItemComponent::FOOD) {
        ++inventory.food_count;
    } else if (type == ItemComponent::WATER) {
        ++inventory.water_count;
    } else if (type == ItemComponent::MEDICAL) {
        ++inventory.medical_count;
    }
}

inline void decrementSurvivalCounter(SurvivalInventoryComponent& inventory, ItemComponent::Type type) {
    int* count = nullptr;
    if (type == ItemComponent::FOOD) {
        count = &inventory.food_count;
    } else if (type == ItemComponent::WATER) {
        count = &inventory.water_count;
    } else if (type == ItemComponent::MEDICAL) {
        count = &inventory.medical_count;
    }
    if (count && *count > 0) --(*count);
}

inline InventoryInspection inspectSelectedInventoryItem(const DiscreteInventoryComponent& inventory) {
    int selected = selectedInventorySlot(inventory);
    if (selected < 0) return {};

    const auto& item = inventory.slots[static_cast<size_t>(selected)];
    if (!item.occupied) return {};

    return {true, item.type, item.restore_value, item.flags};
}

inline void applyInventoryMedical(Registry& registry, Entity player, BiologyComponent& bio, float restore_value) {
    if (registry.has<InjuryComponent>(player)) {
        auto& injuries = registry.get<InjuryComponent>(player);
        int worst = -1;
        for (int i = 0; i < InjuryComponent::MAX_SLOTS; ++i) {
            if (injuries.slots[i].type == InjuryType::NONE) continue;
            if (worst < 0 || injuries.slots[i].severity > injuries.slots[worst].severity) {
                worst = i;
            }
        }
        if (worst >= 0) {
            injuries.slots[worst].severity -= 0.5f;
            if (injuries.slots[worst].severity <= 0.0f) {
                injuries.slots[worst].severity = 0.0f;
                injuries.slots[worst].type = InjuryType::NONE;
            }
            return;
        }
    }
    bio.health = std::min(100.0f, bio.health + restore_value);
}

inline void applyInventoryItemEffect(Registry& registry, Entity player, const CarriedItem& item) {
    auto& bio = registry.get<BiologyComponent>(player);
    if (item.type == ItemComponent::FOOD) {
        bio.hunger = std::min(100.0f, bio.hunger + item.restore_value);
    } else if (item.type == ItemComponent::WATER) {
        bio.thirst = std::min(100.0f, bio.thirst + item.restore_value);
    } else if (item.type == ItemComponent::MEDICAL) {
        applyInventoryMedical(registry, player, bio, item.restore_value);
    }
}

inline bool storeInventoryItem(
    DiscreteInventoryComponent& inventory,
    ItemComponent::Type type,
    float restore_value = 0.0f,
    uint32_t flags = ITEM_FLAG_LEGAL,
    Entity source = MAX_ENTITIES
) {
    int emptySlot = firstEmptyInventorySlot(inventory);
    if (emptySlot < 0) return false;

    auto& slot = inventory.slots[static_cast<size_t>(emptySlot)];
    slot.occupied = true;
    slot.type = type;
    slot.restore_value = restore_value;
    slot.flags = flags;
    slot.source = source;
    return true;
}

inline bool addBaselineScanTools(DiscreteInventoryComponent& inventory) {
    bool added = true;
    added = storeInventoryItem(inventory, ItemComponent::SURFACE_SCAN_TOOL, 0.0f,
                               ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE) && added;
    added = storeInventoryItem(inventory, ItemComponent::BIOLOGY_AUDIT_TOOL, 0.0f,
                               ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE) && added;
    added = storeInventoryItem(inventory, ItemComponent::COGNITIVE_PROFILE_TOOL, 0.0f,
                               ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE) && added;
    added = storeInventoryItem(inventory, ItemComponent::FINANCIAL_FORENSICS_TOOL, 0.0f,
                               ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE) && added;
    added = storeInventoryItem(inventory, ItemComponent::STRUCTURAL_ANALYSIS_TOOL, 0.0f,
                               ITEM_FLAG_LEGAL | ITEM_FLAG_UNIQUE) && added;
    return added;
}

inline bool hasInventoryItemForEquipment(Registry& registry, Entity player, EquipmentSlot slot) {
    if (slot == EquipmentSlot::NONE) return true;
    if (isConsumableEquipment(slot)) return true;
    if (!registry.has<DiscreteInventoryComponent>(player)) return false;

    ItemComponent::Type type = equipmentInventoryType(slot);
    const auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    for (const auto& item : inventory.slots) {
        if (item.occupied && item.type == type) return true;
    }
    return false;
}

inline EquipmentSlot equipSelectedInventoryTool(Registry& registry, Entity player) {
    if (!registry.has<DiscreteInventoryComponent>(player) || !registry.has<EquipmentComponent>(player)) {
        return EquipmentSlot::NONE;
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    int selected = selectedInventorySlot(inventory);
    if (selected < 0) return EquipmentSlot::NONE;

    auto& slot = inventory.slots[static_cast<size_t>(selected)];
    if (!slot.occupied || !isScanToolItemType(slot.type)) return EquipmentSlot::NONE;

    EquipmentSlot equipmentSlot = equipmentFromInventoryType(slot.type);
    registry.get<EquipmentComponent>(player).equipped = equipmentSlot;
    return equipmentSlot;
}

inline InventoryPickupResult collectNearestInventoryItem(
    Registry& registry,
    Entity player,
    float pickup_radius = 30.0f
) {
    if (!registry.has<TransformComponent>(player) || !registry.has<DiscreteInventoryComponent>(player)) {
        return {};
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    int emptySlot = firstEmptyInventorySlot(inventory);
    if (emptySlot < 0) return {};

    const auto& pt = registry.get<TransformComponent>(player);
    const float maxDist = pickup_radius * pickup_radius;
    Entity best = MAX_ENTITIES;
    float bestDist = maxDist;

    auto items = registry.view<ItemComponent, TransformComponent>();
    for (Entity item : items) {
        const auto& it = registry.get<TransformComponent>(item);
        float dx = it.x - pt.x;
        float dy = it.y - pt.y;
        float dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            best = item;
        }
    }
    if (best == MAX_ENTITIES) return {};

    const auto& item = registry.get<ItemComponent>(best);
    auto& slot = inventory.slots[static_cast<size_t>(emptySlot)];
    slot.occupied = true;
    slot.type = item.type;
    slot.restore_value = item.restore_value;
    slot.flags = item.flags;
    slot.source = best;
    inventory.selected = static_cast<size_t>(emptySlot);

    if (registry.has<SurvivalInventoryComponent>(player)) {
        incrementSurvivalCounter(registry.get<SurvivalInventoryComponent>(player), item.type);
    }

    ItemComponent::Type type = item.type;
    registry.destroy(best);
    return {true, best, static_cast<size_t>(emptySlot), type};
}

inline bool useSelectedInventoryItem(Registry& registry, Entity player) {
    if (!registry.has<DiscreteInventoryComponent>(player) || !registry.has<BiologyComponent>(player)) {
        return false;
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    int selected = selectedInventorySlot(inventory);
    if (selected < 0) return false;

    auto& slot = inventory.slots[static_cast<size_t>(selected)];
    if (!slot.occupied || !isConsumableItemType(slot.type)) return false;

    CarriedItem item = slot;
    applyInventoryItemEffect(registry, player, item);
    if (registry.has<SurvivalInventoryComponent>(player)) {
        decrementSurvivalCounter(registry.get<SurvivalInventoryComponent>(player), item.type);
    }
    slot = {};
    return true;
}

inline bool useFirstInventoryItemOfType(Registry& registry, Entity player, ItemComponent::Type type) {
    if (!registry.has<DiscreteInventoryComponent>(player) || !registry.has<BiologyComponent>(player)) {
        return false;
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    for (size_t i = 0; i < inventory.slots.size(); ++i) {
        if (inventory.slots[i].occupied && inventory.slots[i].type == type) {
            inventory.selected = i;
            return useSelectedInventoryItem(registry, player);
        }
    }
    return false;
}

inline Entity dropSelectedInventoryItem(Registry& registry, Entity player) {
    if (!registry.has<TransformComponent>(player) || !registry.has<DiscreteInventoryComponent>(player)) {
        return MAX_ENTITIES;
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    int selected = selectedInventorySlot(inventory);
    if (selected < 0) return MAX_ENTITIES;

    auto& slot = inventory.slots[static_cast<size_t>(selected)];
    if (!slot.occupied) return MAX_ENTITIES;

    const auto& pt = registry.get<TransformComponent>(player);
    Entity dropped = registry.create();
    registry.assign<TransformComponent>(dropped, pt.x + 12.0f, pt.y, 8.0f, 8.0f);
    registry.assign<ItemComponent>(dropped, slot.type, slot.restore_value, slot.flags);

    if (registry.has<SurvivalInventoryComponent>(player)) {
        decrementSurvivalCounter(registry.get<SurvivalInventoryComponent>(player), slot.type);
    }
    slot = {};
    return dropped;
}
