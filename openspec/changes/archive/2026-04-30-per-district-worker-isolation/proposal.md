# Change: Per-District Worker Isolation

## Why

Transit now connects two authored districts, but several worker and interference readouts still behave like there is only one local scope. The next slice needs each district's worker loop, suspicion surface, wage clue, clinic access clue, and dependency disruption readout to remain legible and local.

## What Changes

- Make fixed worker spawning prefer one local housing-workplace route per district before filling any remaining worker slots.
- Add short district tags to multi-district worker, route, purpose, wage, local notice, and local witness readouts while preserving one-district strings.
- Scope clinic ledger/wage mismatch readouts to the district where the worker record exists.
- Scope dependency disruption state and dependency scan/readouts to the district of the affected workplace/supply target.
- Keep AI playtest warps district-local so terminal scenarios inspect the current district unless no local target exists.

## Impact

- Affected spec: `district-worker-isolation`
- Affected code: `src/components.h`, `src/fixed_actor_system.h`, `src/world_builder.h`, `src/main.cpp`, `src/ai_playtest.h`, `tests/world_builder_tests.cpp`, `tests/ai_playtest_tests.cpp`
