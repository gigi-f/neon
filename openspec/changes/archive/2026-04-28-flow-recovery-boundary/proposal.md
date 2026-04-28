## Summary

Make flow recovery visible through the same local route objects that show a blockage, and define the save/load boundary for the derived blockage state.

## Motivation

Phase 69 made spoofed route signposts produce a visible local `FLOW: BLOCKED` consequence. Phase 70 needs the reverse action to feel equally legible without adding a repair system, event history, or city-wide flow model.

## Scope

- Use the existing `Shift+G` restore action on spoofed route signposts as the recovery action.
- Show recovered flow on the restored signpost, affected path, and workplace/supply flow readout.
- Keep active blockage persistence derived from the already-saved spoofed signpost state.
- Treat recovery acknowledgement as volatile readout state; loading a restored signpost returns it to normal clear labels.
- Add deterministic tests for blocked to recovered transitions, stale text clearing, and save/load boundaries.

## Out of Scope

- New repair minigames or commands.
- New management UI, logs, journals, or objective markers.
- New generalized event/history persistence.
- New city-wide flow graph or cascade simulation.
