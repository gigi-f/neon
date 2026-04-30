# Change: Transit Station Districts

## Why

The prototype has stabilized enough to test whether the existing loop survives a larger authored world. The next slice needs a second district and a simple transit link that carries the player between districts without restoring traffic, schedules, vehicles, road hierarchy, or city-scale infrastructure.

## What Changes

- Allow the sandbox config to build two self-contained authored district clusters.
- Add paired station entities with `StationComponent` and a visible `T` glyph.
- Let `E` at a station board a transit interior where the player can move while ride time advances.
- Let the player choose either `E LOOK OUT WINDOW` for immediate destination-platform exit or wait for arrival doors to open and then `E EXIT TRANSIT`.
- Persist in-progress transit state in tiny save/load and expose the district/transit state through HUD, inspection, and AI playtest snapshots.

## Impact

- Affected spec: `transit-stations`
- Affected code: `src/components.h`, `src/world_config.h`, `src/world_builder.h`, `src/infrastructure_solver.h`, `src/save_state.h`, `src/main.cpp`, `src/ai_playtest.h`, `tests/world_builder_tests.cpp`, `tests/ai_playtest_tests.cpp`
