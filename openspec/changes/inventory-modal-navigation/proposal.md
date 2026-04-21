## Why

Discrete inventory interactions currently rely on bracket-based slot cycling and a single-line HUD summary. That blocks the next TODO requirement for a navigable inventory modal where players can quickly inspect carried items, see which slot is selected, and understand assigned hotkeys.

## What Changes

- Add an inventory modal toggle in the gameplay loop.
- Render a modal grid of inventory slots with a clear cursor highlight.
- Show an icon and item label for occupied slots, plus the assigned hotkey number when the slot's item maps to an equipped hotkey.
- Allow directional cursor navigation while the modal is open and keep existing use/drop actions available from the modal.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `discrete-inventory`: Adds a player-visible inventory modal presentation and directional slot navigation.

## Impact

- Affected code: `src/main.cpp`, `src/equipment.h`, and focused tests.
- No new dependencies or asset formats.
