## Why

Routes currently render as paths and signposts, but their readouts do not explain why the connection exists or what flow it carries. Phase 67 needs those paths to become readable city flow without introducing a route planner or larger traffic system.

## What Changes

- Derive a route purpose label from the two endpoint building roles on each pedestrian path.
- Include that purpose in ordinary path and signpost inspection readouts.
- Add debugger scan detail for paths and signposts that exposes expected route cargo or access.
- Keep spoofed signpost readouts coherent by preserving destination, purpose, and carried-flow text while showing the existing corrupted signal consequence.
- Add tests for purpose derivation, normal/spoofed signpost readouts, and debugger scan enrichment.

## Capabilities

### New Capabilities
- `route-purpose-readouts`: Derived route-purpose labels for paths, signposts, and debugger scans.

### Modified Capabilities

## Impact

- Affects `src/world_builder.h`, `src/infrastructure_solver.h`, and `tests/world_builder_tests.cpp`.
- No new dependencies, controls, persistence state, minimap, planner, traffic, or route graph UI.
