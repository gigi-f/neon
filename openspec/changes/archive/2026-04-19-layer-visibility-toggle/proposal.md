## Why

The simulation now has enough L0-L3 state to inspect through panels, but those systems are hard to read while moving through the city. The next visibility step is a cheap layer overlay cycle that lets the player scan the map at a glance before choosing a specific entity to inspect.

## What Changes

- **New**: A layer overlay mode that cycles through Off, Surface, L0 Environment, L1 Biology, L2 Social, L3 Economy, and L4 Faction.
- **New**: Lightweight screen-space glyph/outline markers for visible entities with layer-relevant state.
- **Modified**: The debug/survival HUD reports the active layer overlay mode.
- **Modified**: `todo/TODO.md` marks the Layer visibility toggle complete.

## Capabilities

### New Capabilities
- `layer-visibility`: player can cycle layer overlays and see cheap visible markers for each simulation layer.

### Modified Capabilities

## Impact

- `src/main.cpp` - add overlay mode state, input handling, HUD text, and layer marker rendering.
- `todo/TODO.md` - mark the layer visibility toggle and its acceptance items complete.
