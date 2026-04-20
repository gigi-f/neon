# Requirements

### Requirement: Bitmap Font Loading
The system SHALL load the omelette monospaced font atlas (`om_large_plain_black_rgba.png`)
at startup as an SDL_Texture with white glyphs on transparent background, suitable for
SDL_SetTextureColorMod-based per-draw tinting.

#### Scenario: Font Inversion at Load
- **WHEN** the RGBA atlas (black glyphs on transparent, 256×96) is loaded
- **THEN** each pixel's RGB SHALL be inverted (0,0,0 → 255,255,255), preserving alpha,
  so that SDL color modulation can tint glyphs to any desired color at render time

### Requirement: GlyphComponent
Each visually-rendered entity SHALL carry a GlyphComponent specifying: glyph string,
RGBA color, scale multiplier, and a center flag.

#### Scenario: Single-character Entity
- **WHEN** a CitizenComponent entity has `GlyphComponent.chars = "."`
- **THEN** a single glyph SHALL be rendered centered on the entity's screen-space position,
  scaled by `GlyphComponent.scale * camera.scale`

#### Scenario: Multi-character Entity
- **WHEN** a TransitVehicle entity has `GlyphComponent.chars = "MMMM"`
- **THEN** all 4 glyphs SHALL be rendered left-to-right, centered as a group on the
  entity's screen-space position

### Requirement: Unified Glyph Render Pass
All entities with TransformComponent + GlyphComponent SHALL be rendered via a unified
glyph pass (ground pass and elevated pass) replacing all type-specific SDL_RenderFillRect
entity-drawing blocks.

#### Scenario: Ground vs. Elevated Separation
- **WHEN** the ground glyph pass executes
- **THEN** entities with VehicleComponent.type == MAGLIFT SHALL be skipped
- **AND** rendered instead in the subsequent elevated glyph pass (after maglift track draw)

### Requirement: Road and Infrastructure Rendering Unchanged
Road tiles, waiting areas, staircases, and traffic lights SHALL continue to use their
existing SDL_RenderFillRect rendering. Only entity types with GlyphComponent are affected.

### Requirement: Debug HUD On-Screen
The per-frame debug HUD (FPS, entity count, time of day, camera position) SHALL be
rendered on-screen using `drawText()` before each `SDL_RenderPresent`, in addition to
the existing window-title HUD.
