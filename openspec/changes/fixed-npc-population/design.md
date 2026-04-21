## Context

Citizens are currently seeded in a compact cluster. The sandbox needs startup population distributed around the map and never stacked onto occupied non-road tiles.

## Goals / Non-Goals

**Goals:**

- Spread initial citizens across multiple road regions around the sandbox.
- Ensure seeded citizens are placed on road surfaces.
- Reject any candidate spawn that overlaps occupied non-road geometry.
- Keep deterministic fixed population count.

**Non-Goals:**

- No re-enabling runtime citizen spawn.
- No district expansion changes.

## Approach

1. Build a list of eligible spawn roads (exclude maglift track).
2. Deterministically sample candidate points across those roads.
3. Accept only candidates that are on road and do not intersect non-road occupied transforms.
4. Keep trying until fixed cohort is seeded.
5. Add tests for spread and occupancy invariants.
