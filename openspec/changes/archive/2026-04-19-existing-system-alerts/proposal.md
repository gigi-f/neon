## Why

The simulation already changes weather, flooding, infection state, and structural collapse, but those events are mostly invisible unless the player is watching debug output or inspecting the right entity. Milestone 1 needs lightweight alerts so existing systems become readable player-facing signals before the Intel Log exists.

## What Changes

- Add a short in-memory alert stack for recent simulation events.
- Emit alerts when weather changes, flooding toggles, an infection becomes meaningfully worse, or a building collapses.
- Render the recent alerts on the HUD without introducing expensive full-screen effects or persistent UI state.
- Keep alert history bounded so it can later feed Intel Log work without unbounded growth.

## Capabilities

### New Capabilities
- `simulation-alerts`: Player-facing notification stack for important existing-system events.

### Modified Capabilities

## Impact

- Affected code: `src/components.h`, `src/simulation_systems.h`, `src/main.cpp`, and focused deterministic tests.
- No new runtime dependencies.
- Alert generation is tied to existing L0-L3 simulation ticks and remains bounded in memory.
