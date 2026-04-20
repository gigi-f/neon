## Context

`nearestScanTarget()` currently lives in `main.cpp` and chooses the first scan target from entities within the active tool range, preferring the player's facing direction. The TODO asks for a selection cursor controlled by arrow keys before triggering actions, and scan panels already have the target marker and selected-entity panel needed to represent that cursor.

## Goals / Non-Goals

**Goals:**
- Let arrow-key keydown events move the active scan target to another eligible entity.
- Preserve the existing facing-aware initial target selection.
- Reuse scan tool ranges so cursor movement cannot select out-of-range entities.
- Keep selection logic deterministic and covered by a small test.

**Non-Goals:**
- Build a full inventory grid or target reticle UI.
- Add hotkey rebinding.
- Add mouse targeting or screen-space raycasting.
- Change scan panel contents or scan tool ranges.

## Decisions

- Extract scan target helpers to `target_selection.h`, which can be included by both `main.cpp` and a small CTest binary.
- Treat the selected scan entity as the cursor. Arrow keys choose the best candidate in that world direction from the current target, while still filtering all candidates by distance from the player.
- Use the player position as the cursor origin when no current target exists, so arrow keys can recover from an empty or stale selection.
- Keep movement on keydown only. The continuous keyboard state can still drive player movement, but each key press advances the cursor once.

## Risks / Trade-offs

- Arrow-key movement can also move the player because movement still reads held keyboard state. This keeps controls simple for this slice; a later full targeting mode can pause movement or add a dedicated cursor mode if needed.
- Directional selection is world-space, not screen-space. The current camera is not rotated, so world-space directions match on-screen arrows.
