## Why

Phase 67 made path and signpost flow labels readable, but spoofing a signpost still only reads as a route signal problem. Phase 68 makes the existing `Shift+G` spoof verb visibly corrupt the route-flow label and propagate a local consequence to the affected site.

## What Changes

- Spoofed signposts corrupt their carried-flow label in ordinary inspection, such as `CARRIES: ???`.
- A spoofed supply route derives a site-level readout on the dependent workplace, such as `SUPPLY FLOW: DISRUPTED`.
- Restoring the signpost clears the flow corruption and site-level consequence.
- Existing movement-blocking behavior remains unchanged.
- Add tests for spoofed flow labels, restoration, site consequence, and unrelated-route boundaries.

## Capabilities

### New Capabilities

### Modified Capabilities
- `route-purpose-readouts`: spoofing now corrupts visible route-flow labels and exposes a local flow consequence.

## Impact

- Affects `src/world_builder.h` and `tests/world_builder_tests.cpp`.
- No new controls, global risk state, alert feed, traffic system, planner, minimap, persistence schema, or management UI.
