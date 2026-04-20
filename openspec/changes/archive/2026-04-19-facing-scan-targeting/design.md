## Context

`nearestScanTarget()` currently scans every entity with `TransformComponent` and selects the nearest entity within range. The player already has `PlayerComponent::facing`, and movement updates that field, so no new component state is required.

## Goals / Non-Goals

**Goals:**
- Prefer scan targets in the player's facing direction.
- Preserve range-limited nearest-target behavior when no forward target exists.
- Apply consistently to Surface, Biology, Cognitive, Financial, and Structural scan panels.

**Non-Goals:**
- Add a cursor, raycast UI, or manual target cycling.
- Change scan range, scan panel content, or target marker rendering.

## Decisions

- Use a facing vector derived from `PlayerComponent::facing` and a dot-product cone test. This is cheap, deterministic, and avoids screen-space dependencies.
- Rank forward targets ahead of off-axis targets, then use distance as the tie-breaker. This keeps selection intuitive without making close side targets impossible when nothing is in front.
- Keep the helper in `main.cpp` because scan panel input and rendering already live there and no other system currently consumes scan selection.

## Risks / Trade-offs

- A target just outside the forward cone can lose to a farther target in front. Mitigation: use a broad cone threshold so "in front" includes slight diagonals.
- The helper remains local to `main.cpp`, which limits isolated unit testing. Mitigation: validate with build diagnostics and keep the algorithm small and side-effect free.
