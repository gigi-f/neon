## Why

Current sandbox geometry has critical placement issues: housing overlaps road surfaces, transit stop structures sit on road tiles, and the player-owned vehicle appears on or near the maglift track. This breaks readability and blocks reliable world-building iteration.

## What Changes

- Rework sandbox coordinate anchors to guarantee non-overlap between district buildings and roadway geometry.
- Place station/platform/stair/waiting-area geometry off roadway surfaces.
- Move player-owned vehicle spawn to an off-track roadway position and use non-maglift vehicle type.
- Add placement validation checks and tests so overlap regressions fail early.

## Capabilities

### Modified Capabilities

- `sandbox-city-layout`: placement invariants are enforced for district and transit support geometry.

## Impact

- Affected code: `src/world_generation.h`, `src/main.cpp`, `tests/world_generation_tests.cpp`.
- Provides a stable geometric baseline before continuing district-by-district expansion.
