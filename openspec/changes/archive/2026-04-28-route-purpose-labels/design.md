## Context

The sandbox derives pedestrian paths from configured building connections. `PathComponent` already stores endpoint entities, and `RouteSignpostComponent` stores the path and target role. Current readouts are generic, so route infrastructure is visible but not semantically readable.

## Goals / Non-Goals

**Goals:**
- Derive labels from existing endpoint roles instead of storing new route state.
- Use the same helper for path inspection, signpost inspection, and debugger scans.
- Keep spoofing local to signpost readout corruption and existing movement-blocking behavior.

**Non-Goals:**
- No route planner, minimap, traffic simulation, access-control system, or persistence migration.
- No new player controls.
- No broad dependency graph beyond the existing current-scope dependency edge.

## Decisions

- Derive route purpose from endpoint roles at readout time. This keeps labels deterministic and avoids new state that can go stale.
- Return a small value object with public label, carried flow, expected cargo, and access detail. This keeps ordinary inspection readable while giving the debugger richer detail.
- For unknown or invalid paths, fall back to `PUBLIC ACCESS` and `UNKNOWN` style labels so readouts remain coherent.
- Spoofed signposts keep their destination, purpose, and carried-flow labels while showing the existing corrupted signal consequence. Phase 68 can deliberately corrupt the flow label as a separate mechanical enrichment.

## Risks / Trade-offs

- Endpoint-role matching can become verbose as more route types are added. Mitigation: keep the helper centralized so future phases extend one switch-like function.
- Derived labels are only as accurate as the current path endpoints. Mitigation: tests cover the current housing-workplace and workplace-supply paths.
