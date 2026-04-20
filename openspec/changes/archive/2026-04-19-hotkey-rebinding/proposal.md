## Why

The equipment model now has numeric hotkey slots, but players cannot change the defaults. The next TODO item needs a small rebinding path before inventory work can rely on player-selected quick slots.

## What Changes

- Add player-controlled rebinding for numeric equipment hotkeys.
- Use the currently equipped item or scan tool as the source for the new binding.
- Keep `0` reserved for no equipment.
- Preserve existing direct-use behavior for normal numeric key presses.

## Capabilities

### New Capabilities

- `hotkey-rebinding`: Player-controlled assignment of equipment slots to numeric hotkeys.

### Modified Capabilities

- `equipped-tools`: Numeric hotkeys become configurable at runtime while retaining the existing default mappings.

## Impact

- Affects `src/equipment.h`, `src/main.cpp`, equipment tests, and the Milestone 2 TODO checklist.
- No new dependencies or persistent settings are introduced.
