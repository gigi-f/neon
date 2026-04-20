## ADDED Requirements
### Requirement: Layer overlay cycle
The game SHALL provide a player input that cycles the active layer overlay through Off, Surface, L0 Environment, L1 Biology, L2 Social, L3 Economy, and L4 Faction modes.

#### Scenario: Overlay mode advances
- **WHEN** the player presses the layer overlay cycle input
- **THEN** the active overlay mode SHALL advance to the next layer mode
- **AND** pressing the input from L4 Faction SHALL return the overlay to Off

### Requirement: Cheap map-level markers
Layer overlays SHALL use inexpensive glyph, color, or rectangle markers over currently rendered entities rather than full-screen per-pixel effects.

#### Scenario: Existing layer state is visible
- **WHEN** a layer overlay is active and visible entities carry components relevant to that layer
- **THEN** the renderer SHALL draw compact layer markers for those entities

#### Scenario: Unsupported future layer is selectable
- **WHEN** the L4 Faction overlay is active before faction data exists
- **THEN** the renderer SHALL keep the mode selectable without inventing faction markers

### Requirement: Active overlay HUD status
The HUD SHALL report the currently active layer overlay mode.

#### Scenario: Overlay HUD reflects current mode
- **WHEN** the player cycles to a layer overlay
- **THEN** the HUD SHALL display the active overlay name
