## Implementation

- [x] Re-anchor sandbox layout geometry to remove housing/road overlap.
- [x] Place station/stair/waiting-area geometry off roadway surfaces.
- [x] Add placement-overlap checks to sandbox validation helper.
- [x] Move player-owned vehicle spawn away from track and set non-maglift type.
- [x] Extend tests to cover overlap invariants.
- [x] Validate with `openspec validate sandbox-placement-basics --strict`, `cmake --build build`, and `rtk test ctest --test-dir build --output-on-failure`.
