## 1. Specification

- [x] 1.1 Add a route-flow blockage requirement for spoofed signpost state.

## 2. Implementation

- [x] 2.1 Derive a `FLOW: BLOCKED` label from spoofed signposts on the affected path.
- [x] 2.2 Show the label in path and signpost inspection readouts, and clear it on restore.
- [x] 2.3 Keep the blockage local to the affected route and existing workplace/supply consequence.

## 3. Tests And Bookkeeping

- [x] 3.1 Add deterministic tests for blocked, restored, and unrelated route behavior.
- [x] 3.2 Update `todo/TODO.md` Phase 69 status.
- [x] 3.3 Validate OpenSpec, build, CTest through rtk, and C++ diagnostics.
