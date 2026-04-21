## Context

This is a layout correctness pass, not a content expansion pass. We need strict geometric hygiene first so future district additions are additive and predictable.

## Goals / Non-Goals

**Goals:**

- No building overlap with roadway rectangles.
- No station/stair/waiting-area overlap with roadway rectangles.
- Player-owned vehicle spawns away from maglift track.
- Validation and tests encode these guarantees.

**Non-Goals:**

- No new district sub-section implementation in this change.
- No NPC routine loop changes.

## Approach

1. Introduce explicit placement anchors for track, roads, stations, and housing blocks with separated bounding boxes.
2. Keep roadway network simple while preserving transit access.
3. Add geometry-overlap checks in sandbox validation helper.
4. Update tests to assert non-overlap invariants.
5. Update player-owned vehicle spawn and vehicle type to avoid track confusion.
