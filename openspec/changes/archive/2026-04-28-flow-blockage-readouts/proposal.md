## Summary

Add one explicit local flow blockage readout derived from the existing route signpost spoof action.

## Motivation

The current route spoof state already creates local confusion and a workplace supply-flow consequence, but the affected route objects do not yet name the result as a concrete blockage. Phase 69 needs one readable blockage using an existing player verb without adding jobs, economy state, alerts, or city-wide cascade.

## Scope

- Use existing `RouteSignpostComponent::spoofed` state as the blockage source.
- Show `FLOW: BLOCKED` on the affected path and spoofed signpost.
- Keep the existing workplace/supply consequence local and reversible.
- Add deterministic tests for local, reversible behavior and unrelated-route boundaries.

## Out of Scope

- New persistence state.
- New job/economy/crisis systems.
- New alert feed, journal, minimap, objective marker, or city-wide flow graph.
