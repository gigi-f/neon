## Context

The current sandbox has derived pedestrian paths, route signposts, a reversible `Shift+G` spoof verb, and Phase 67 route purpose labels. Signpost spoofing already pauses workers on the affected path; this change adds clearer flow-level presentation on top of the existing mechanic.

## Goals / Non-Goals

**Goals:**
- Corrupt route-flow labels only while the relevant signpost is spoofed.
- Surface a local site consequence for the current workplace/supply dependency.
- Keep movement blocking and restoration semantics exactly where they already live.

**Non-Goals:**
- No new input, route planner, minimap, traffic simulation, alert feed, wanted/risk state, or broad flow graph.
- No persistence migration; spoofed signpost state already persists.
- No city-wide cascading consequence.

## Decisions

- Derive flow disruption from existing `RouteSignpostComponent::spoofed` state instead of adding a new component.
- Treat the workplace/supply path as the only current site-level flow consequence: spoofing that path shows `SUPPLY FLOW: DISRUPTED` on the workplace, and restoring the signpost clears it.
- Keep labor-route spoofing local to its path/signpost and worker movement block; it must not create the supply-flow site consequence.

## Risks / Trade-offs

- Route-specific consequence logic is narrow. This is intentional for the current sandbox; future phases can generalize only after another real route needs it.
- Multiple signposts per path mean either endpoint signpost can disrupt the path. This matches the existing movement block behavior and keeps the rule easy to inspect.
