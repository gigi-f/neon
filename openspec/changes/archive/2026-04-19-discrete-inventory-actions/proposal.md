## Why

The player can carry food, water, and medical supplies only as counters. The next playable-loop step is to make picked-up items exist as inspectable inventory entries before the full inventory UI, barter flow, or provenance system is built.

## What Changes

- Add a fixed-capacity discrete inventory model for carried items.
- Add deterministic helpers for picking up nearby items, dropping selected items, inspecting selected items, and using selected consumables.
- Preserve the existing survival counters and quick-use behavior while creating a tested path for future inventory UI controls.
- Mark the TODO item for discrete pick up/drop/inspect/use once validated.

## Capabilities

### New Capabilities

- `discrete-inventory`: player inventory can store individual item entries and perform core item actions.

## Impact

- `src/components.h` - add discrete inventory component and carried item record.
- `src/inventory.h` - add inventory action helpers.
- `tests/discrete_inventory_tests.cpp` - deterministic coverage for pick up, drop, inspect, and use.
- `CMakeLists.txt` - add the discrete inventory test target.
- `todo/TODO.md` - mark the completed first-slice inventory action.
