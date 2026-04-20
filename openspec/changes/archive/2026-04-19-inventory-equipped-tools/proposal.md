## Why

The TODO's remaining equipment item asks for tools to be equipped from inventory instead of acting as disconnected global key actions. The code already has equipped slots, hotkeys, scan ranges, and a bounded inventory, so the next step is to make carried tool entries the source of truth for scan equipment.

## What Changes

- Represent scan tools as discrete inventory entries alongside consumables.
- Seed the player with baseline carried scan tools at spawn.
- Let the selected inventory entry or shortcut equip a scan tool, while consumable entries continue to use/drop normally.
- Require the action key (`Space`) to fire equipped scan tools.
- Require hotkey and equipped scan use to have a matching carried tool, preserving existing numeric controls and feedback.
- Show a world-space range bounding box for the equipped scan tool and reserve arrow keys for cursor/focus instead of movement.
- Mark the matching equipment TODO planning item complete after validation.

## Capabilities

### New Capabilities

- `inventory-equipped-tools`: Carried inventory entries can represent equippable scan tools.

### Modified Capabilities

- `equipped-tools`: Equipped scan tool use requires a matching carried inventory tool.
- `discrete-inventory`: Discrete inventory entries can contain non-consumable tool items.

## Impact

- Affects `src/components.h`, `src/equipment.h`, `src/inventory.h`, `src/main.cpp`, equipment and inventory tests, OpenSpec specs, and `todo/TODO.md`.
- No new runtime dependency or save migration is introduced.
