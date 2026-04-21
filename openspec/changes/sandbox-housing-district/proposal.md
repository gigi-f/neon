## Why

Milestone 0 now drives a sandbox-first world build, but district rollout should happen one subsection at a time to keep behavior and validation focused. The first subsection is **Housing** so NPC birth/home origin and future sleep/home loop hooks have a stable anchor.

## What Changes

- Implement only the housing district in the sandbox map.
- Keep procedural city generation removed; world initialization remains authored.
- Ensure housing has transit access so future routine traversal is testable.
- Add explicit startup validation for housing presence and basic transit station coverage.

## Capabilities

### Modified Capabilities

- `sandbox-city-layout`: progresses to phase-1 district rollout with only housing implemented.

## Impact

- Affected code: `src/world_generation.h`, `src/main.cpp`, focused tests.
- Backlog progress: only `Housing` sub-item is completed in this step.
