## Why

The equipped-tool slice still opens scan panels against an automatically chosen target. The next TODO step is to let the player steer that target manually before using the equipped action, so inspection starts to feel like item/tool use instead of a global hotkey.

## What Changes

- Add arrow-key target cursor movement while a scan panel is active.
- Keep the existing facing-aware automatic target as the initial selection.
- Keep target movement constrained to the active scan tool range.
- Show the manually selected target with the existing scan marker and panel content.

## Capabilities

### New Capabilities

### Modified Capabilities

- `scan-targeting`: scan targets can be manually moved between eligible nearby entities with arrow keys after the initial automatic selection.

## Impact

- `src/main.cpp` - route arrow-key keydown events to scan target cursor movement when a scan panel is open.
- `src/target_selection.h` - shared target selection helpers for automatic and manual scan targeting.
- `tests/target_selection_tests.cpp` - deterministic coverage for directional cursor movement and range constraints.
- `CMakeLists.txt` - target selection test target.
- `todo/TODO.md` - mark the manual selection cursor acceptance item complete.
