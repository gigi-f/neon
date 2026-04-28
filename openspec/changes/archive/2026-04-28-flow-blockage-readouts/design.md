## Design

The blockage is derived from route spoof state that already exists and is already player-actionable through the inherited gadget. A path is blocked when any signpost attached to that path is spoofed.

The route objects are the readout source for this phase:

- `pathInspectionReadout(...)` adds `FLOW: BLOCKED` only for the affected path.
- `routeSignpostReadout(...)` adds `FLOW: BLOCKED` only for the spoofed signpost readout.
- Restoring the signpost clears the derived label because there is no new stored blockage state.

The existing workplace/supply route consequence remains the dependent-site readout. This keeps the phase local and avoids introducing a new system-wide flow model.
