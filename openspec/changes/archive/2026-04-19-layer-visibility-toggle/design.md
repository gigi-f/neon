## Context

Rendering currently happens in `src/main.cpp` with direct SDL draw calls. Scan panels already expose detailed entity state, but the player needs a lower-friction way to notice which nearby entities are relevant to Surface, L0, L1, L2, and L3 systems. There is not yet a faction component, so L4 can only be represented as an empty/future overlay state.

## Goals / Non-Goals

**Goals:**
- Add a single key that cycles layer overlay modes.
- Keep overlays cheap by reusing visible entity bounds and simple SDL primitives.
- Give every existing layer a readable map-level signal:
  - Surface: entity class/category markers.
  - L0 Environment: structural, power, weather/hazard-adjacent infrastructure state.
  - L1 Biology: health/infection/item markers.
  - L2 Social: rank, cognition, and relationship markers.
  - L3 Economy: markets, employers, wallets, and employment markers.
  - L4 Faction: selectable placeholder mode until faction data exists.
- Show the active overlay in the HUD.

**Non-Goals:**
- Add new simulation state, faction systems, territory maps, or full-screen per-pixel effects.
- Change scan panel behavior or entity selection.
- Add expensive city-wide preprocessing.

## Decisions

- Use `O` as the overlay cycle key because the existing inspection keys already occupy `I`, `Shift+I`, `C`, `F`, and `T`.
- Render overlays after normal world rendering and before the scan target marker. That keeps the base scene visible while ensuring layer hints are not hidden by the normal glyph passes.
- Use alpha-blended rectangles and short bitmap-font labels. This matches the existing renderer and avoids introducing texture or shader work.
- Treat L4 as a valid mode with a HUD label but no markers until faction components exist. This satisfies the navigation contract without hardcoding fake faction data.

## Risks / Trade-offs

- Dense scenes can become noisy. Mitigation: only render markers for entities carrying layer-relevant components and use compact labels.
- L4 has no visible map content yet. Mitigation: expose the mode now and leave the marker function data-driven so future faction components can plug in cleanly.
- `main.cpp` continues to own rendering helpers. This is consistent with the current project structure, though a future renderer split may be warranted.
