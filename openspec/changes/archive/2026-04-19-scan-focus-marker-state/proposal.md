## Why

Pre-fire scan focus and an active scan result currently use the same world marker, which makes it hard to tell whether the tool has actually fired. The targeting UI needs separate visual states now that arrow keys can focus before `Space` opens the scan panel.

## What Changes

- Draw pre-fire scan focus with a distinct marker style.
- Preserve the stronger active scan-panel marker after the tool fires.
- Document the marker-state distinction in scan-targeting requirements.

## Capabilities

### New Capabilities

### Modified Capabilities

- `scan-targeting`: Add marker-state requirements for focused versus active scan targets.

## Impact

- Affects `src/main.cpp` rendering and `openspec/specs/scan-targeting/spec.md`.
