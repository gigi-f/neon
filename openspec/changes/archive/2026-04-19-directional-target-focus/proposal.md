## Why

Scan targeting should feel like directional focus control, not a one-shot nearest-target picker. Arrow keys now need to cycle among in-range targets before the action key fires the equipped tool.

## What Changes

- Arrow keys choose the available in-range target most aligned with the pressed direction.
- A selected target remains locked and follows the entity while it stays in scan range.
- If the only available target is already selected and the player presses an arrow key, selection clears.
- `Space` fires the equipped scan tool against the locked target when valid; otherwise it falls back to the normal facing-aware target.

## Capabilities

### New Capabilities

### Modified Capabilities

- `scan-targeting`: Refine manual scan cursor behavior into directional focus cycling and lock clearing.

## Impact

- Affects `src/target_selection.h`, `src/main.cpp`, target selection tests, and scan-targeting specs.
