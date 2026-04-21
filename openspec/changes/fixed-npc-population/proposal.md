## Why

Sandbox runtime currently keeps adding citizens over time, which makes population behavior unstable and hard to tune. We need a fixed citizen count for deterministic sandbox iteration.

## What Changes

- Seed a fixed number of citizens during sandbox world generation.
- Disable runtime citizen spawning and citizen distance-based despawning.
- Prevent citizens from being destroyed by biology death flow.

## Capabilities

### Modified Capabilities

- `sandbox-city-layout`: citizen population becomes fixed-size at runtime.

## Impact

- Affected code: `src/world_generation.h`, `src/main.cpp`, `src/simulation_systems.h`, `tests/world_generation_tests.cpp`.
