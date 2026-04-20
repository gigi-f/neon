## Why

The core scan tools and survival consumables are still invoked as global key actions. The next playable-loop step is to make those actions behave like equipped inventory slots so the player has one active item/tool, visible range, and hotkey-driven access before the full inventory UI exists.

## What Changes

- Add a lightweight equipment model for player hotkey slots.
- Give scan tools explicit ranges and route scan activation through equipped slots.
- Preserve direct food/water/medical quick-use behavior while making those keys equip the matching slot.
- Show the currently equipped slot and range on the HUD.

## Capabilities

### New Capabilities

- `equipped-tools`: player inventory slots can equip consumables and scan tools with configured ranges.

## Impact

- `src/equipment.h` - new equipment slot model, hotkey defaults, item mapping, and ranges.
- `src/main.cpp` - player equipment component, hotkey/use routing, scan range routing, and HUD display.
- `tests/equipment_tests.cpp` - deterministic equipment mapping coverage.
- `CMakeLists.txt` - equipment test target.
- `todo/TODO.md` - mark the completed first-slice equipment acceptance items.
