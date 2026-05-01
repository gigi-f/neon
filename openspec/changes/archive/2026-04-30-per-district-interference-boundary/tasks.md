## 1. Persistence Boundary

- [x] 1.1 Persist active dependency disruptions with district ids in tiny save state.
- [x] 1.2 Clear stale route/dependency state before applying saved district interference state.
- [x] 1.3 Preserve older tiny save versions without requiring the new dependency section.

## 2. District Readouts And AI Surface

- [x] 2.1 Include district context in multi-district dependency scan readouts.
- [x] 2.2 Keep AI playtest signpost warps scoped to the player's current district.
- [x] 2.3 Exercise cross-district signpost inspection in the terminal playtest harness.

## 3. Verification

- [x] 3.1 Add deterministic tests for spoofing in A while B is clean.
- [x] 3.2 Add deterministic tests for save/load while standing in B with active A interference.
- [x] 3.3 Add deterministic tests that returning to A restores the original spoof, dependency disruption, `LAID_LOW` trace, witness, and wage state.
- [x] 3.4 Run build, CTest, OpenSpec validation, cclsp diagnostics, feature-completion verification, and an adaptive terminal playtest.
