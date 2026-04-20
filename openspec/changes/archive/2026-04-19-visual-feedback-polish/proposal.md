## Why

The simulation now exposes scan panels, overlays, and alert events, but player actions and danger still feel physically disconnected. The next visibility pass needs lightweight feedback so pickups, scans, blocked actions, collisions, and hazard damage are immediately readable without adding a full audio system or expensive effects.

## What Changes

- Add a deterministic feedback state for short screen shake and screen flash effects.
- Emit named feedback cues for pickup, consume, scan, warning, denied action, impact, and hazard damage.
- Wire existing player action and hazard paths to the feedback state.
- Keep feedback cheap and HUD-compatible.

## Impact

- **New spec**: `visual-feedback`
- **Modified code**:
  - `src/simulation_systems.h` - feedback event/state helpers
  - `src/main.cpp` - action, hazard, and rendering integration
  - `tests/visual_feedback_tests.cpp` - deterministic feedback behavior
  - `CMakeLists.txt` - test target
