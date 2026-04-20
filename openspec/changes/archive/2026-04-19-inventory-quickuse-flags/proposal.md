## Why

Discrete inventory entries now exist, but the food/water/medical quick-use path still consumes survival counters directly. The next inventory step is to route quick-use through actual carried item slots while preserving the existing hotkey controls. Item flags also need to become visible through scan panels so legal risk and value tags can drive player decisions.

## What Changes

- Use the first matching discrete inventory item when food, water, or medical quick-use is triggered.
- Preserve the existing counter fallback for legacy state where counters exist without discrete slots.
- Store item flags on world items and preserve them through pickup and drop.
- Show item flags in Surface Scan and Financial Forensics.

## Capabilities

### Modified Capabilities

- `discrete-inventory`: quick-use can consume discrete carried items by type.
- `equipped-tools`: survival consumable hotkeys remain quick-use actions.

### New Capabilities

- `item-flag-visibility`: scan panels expose item legal/value/faction flags.

## Impact

- `src/components.h` - add flags to world item data.
- `src/inventory.h` - add typed quick-use helper and flag label helpers.
- `src/main.cpp` - route quick-use through discrete inventory and show flags in scan panels.
- `tests/discrete_inventory_tests.cpp` - cover typed quick-use and flag preservation.
- `tests/scan_panel_tests.cpp` - cover Surface/Financial item flag text.
- `CMakeLists.txt` - add scan panel test target.
- `todo/TODO.md` - mark the matching inventory TODO items complete.
