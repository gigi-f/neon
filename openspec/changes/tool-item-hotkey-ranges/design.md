## Context

`EquipmentComponent` stores a ten-entry hotkey array indexed by numeric key. `assignEquipmentHotkey()` currently allows any nonzero key to receive any equipment slot, and the SDL event loop uses that helper for Ctrl+number rebinding. The existing defaults were built before the TODO split the keyboard into a tool band (`1-5`) and item band (`6-9`).

## Goals / Non-Goals

**Goals:**

- Make the default hotkey layout match the TODO: five scan tools on `1-5`, item quick-use on `6-9`, `0` empty.
- Preserve the existing single equipped slot, Space-to-use behavior, and Ctrl+number rebinding path.
- Make invalid rebindings deterministic and testable through `assignEquipmentHotkey()`.
- Keep the current consumable quick-use fallback behavior unchanged after the key numbers move.

**Non-Goals:**

- No full navigable inventory modal; that is the next TODO after this slice.
- No persistent keybinding settings.
- No new item categories beyond the current food, water, medical, and scan tools.

## Decisions

- Add small classification helpers in `equipment.h` for scan-tool slots and item/consumable slots.
  - Rationale: range rules belong with the equipment model and can be unit tested without SDL.

- Enforce range rules inside `assignEquipmentHotkey()`.
  - Rationale: the main loop already routes Ctrl+number rebinding through this helper, so one rule covers defaults, tests, and future callers.

- Leave key `9` defaulted to `NONE` until there is a fourth item class.
  - Rationale: the TODO reserves `6-9` for items, but the current game only has three consumable item slots. Binding a duplicate consumable would make the UI less clear.

## Risks / Trade-offs

- Players used to the old defaults will need updated docs and HUD expectations.
- Invalid Ctrl+number rebind attempts currently fail silently except for the existing helper return value. A later inventory modal can surface a clearer denial cue when hotkey assignment becomes a visible UI operation.
