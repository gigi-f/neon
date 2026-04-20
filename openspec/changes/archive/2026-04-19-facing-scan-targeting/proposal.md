## Why

Surface Scan already makes nearby entities readable, but selecting the nearest entity can feel wrong when a closer object is behind or beside the player. The next visibility step is to make scans respect the player's current facing so the highlighted target matches player intent.

## What Changes

- **New**: Direction-aware scan targeting that prefers entities in front of the player.
- **Modified**: All scan panel hotkeys use the same facing-aware target selection path.
- **Modified**: `todo/TODO.md` marks the Surface Scan targeting improvement complete.

## Capabilities

### New Capabilities
- `scan-targeting`: player scan panels select nearby entities with a preference for entities in the player's facing direction.

### Modified Capabilities

## Impact

- `src/main.cpp` - update scan target selection and scan hotkey call sites.
- `todo/TODO.md` - mark the targeting TODO complete.
