## Why

The current procedural city generation is useful for stress-testing systems, but it obscures the gameplay loop we need to validate next: a readable, deterministic NPC day cycle and meaningful player interaction points. We need a hand-authored sandbox microcity that concentrates the key city roles and travel links in one compact map.

## What Changes

- Remove procedural city layout generation and replace it with a fixed sandbox layout profile.
- Define five required districts in the sandbox map: housing, workplace, market, leisure, and upper-class quarters.
- Ensure transit paths connect all required districts so NPC movement and player observation are reliable.
- Establish the follow-on execution phase for NPC routine validation: `sleep => transit => work => transit => leisure => home => repeat`.

## Capabilities

### New Capabilities

- `sandbox-city-layout`: deterministic authored city layout for core district-and-transit gameplay validation.

### Modified Capabilities

- `scan-targeting`: map assumptions move from broad procedural spread to dense authored district topology.
- `simulation-alerts`: alerts should remain meaningful in a compact sandbox where events cluster.

## Impact

- Affected code: world/city generation and spawn initialization paths in `src/main.cpp` and related helpers.
- Affected planning: backlog priority shifts to sandbox-first loop validation before large-scale city generation work.
- No new third-party dependencies required.
