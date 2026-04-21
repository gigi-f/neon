## Implementation

- [x] Implement deterministic sandbox layout containing transit spine plus housing district only.
- [x] Add startup validation helper for housing presence and station index coverage.
- [x] Wire startup to fail fast if sandbox housing validation fails.
- [x] Add/adjust tests for housing-subsection guarantees.
- [x] Mark `Housing` as done under Milestone 0 in `todo/TODO.md`.
- [x] Validate with `openspec validate sandbox-housing-district --strict`, `cmake --build build`, and `rtk test ctest --test-dir build --output-on-failure`.
