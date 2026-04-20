## Why

The L1 survival loop has FOOD, WATER, and MEDICAL items with biological effects, but the player still consumes pickups immediately and has no on-screen survival inventory state. This blocks basic survival readability and makes item categories hard to verify during normal play.

## What Changes

- **New**: `SurvivalInventoryComponent` stores player-carried FOOD, WATER, and MEDICAL counts.
- **Modified**: Player interact pickup collects nearby survival items into inventory instead of immediately consuming them.
- **New**: Number-key consumption for survival inventory slots: 1 = FOOD, 2 = WATER, 3 = MEDICAL.
- **Modified**: On-screen HUD renders survival inventory counts beside the existing biology stats.
- **New**: Focused tests cover collection and keyed consumption effects.
- **Modified**: `todo/TODO.md` marks "Inventory UI for survival items" complete.

## Capabilities

### New Capabilities
- `survival-inventory-ui`: player-facing survival item counters and simple consume controls for FOOD, WATER, and MEDICAL.

### Modified Capabilities
- Player item pickup no longer applies biological effects immediately; effects occur when the player consumes a carried survival item.

## Impact

- `src/components.h` — add `SurvivalInventoryComponent`.
- `src/simulation_systems.h` — add player collection and inventory consumption helpers.
- `src/main.cpp` — assign player inventory, wire 1/2/3 consumption, and render HUD counts.
- `tests/survival_inventory_tests.cpp` — deterministic component/system tests.
- `CMakeLists.txt` — register the survival inventory test binary.
