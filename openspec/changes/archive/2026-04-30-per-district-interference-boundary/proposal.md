# Change: Per-District Interference Boundary

## Why

Transit now lets the player move between two authored districts, so interference state must travel only through explicit save/load and district return boundaries. A spoofed route, disrupted dependency, laid-low local witness, or wage spoof in district A should remain inspectable when the player comes back from district B without leaking into B.

## What Changes

- Persist active dependency disruptions alongside spoofed signposts and worker records in tiny save state.
- Clear stale route/dependency state before applying saved state so loading in another district does not leak previous runtime state.
- Strengthen district-tagged readouts and tests for spoofed routes, disrupted dependencies, `LAID_LOW` traces, witness state, and wage records.
- Keep AI playtest signpost warps district-local and exercise cross-district transit plus signpost inspection in the terminal harness.

## Impact

- Affected spec: `district-interference-boundary`
- Affected code: `src/save_state.h`, `src/world_builder.h`, `src/ai_playtest.h`, `tools/ai_playtest.cpp`, `tests/world_builder_tests.cpp`, `tests/ai_playtest_tests.cpp`
