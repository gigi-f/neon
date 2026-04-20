## ADDED Requirements

### Requirement: Atmosphere Data Model
The worldConfig entity SHALL carry an `AtmosphereGridComponent` that maintains a 2D grid of floating-point atmospheric air quality values (0.0 to 100.0, where 100.0 is pure air and 0.0 is toxic/smog) at a resolution of `macro_cell_size` (40 WU).

#### Scenario: Grid initialized at start
- **WHEN** the simulation starts
- **THEN** an `AtmosphereGridComponent` SHALL be initialized with dimensions derived from `world_min` and `world_max`
- **AND** all cells SHALL be initialized to a baseline air quality (e.g., 100.0)

---

### Requirement: Interior and Building Atmosphere Components
Buildings that are enterable SHALL carry a `BuildingAtmosphereComponent` containing internal `temperature` and `air_quality` fields. Entities inside a building SHALL carry an `InteriorComponent` containing the `building_entity` ID.

#### Scenario: Building internal state tracking
- **WHEN** an enterable building is generated
- **THEN** it SHALL receive a `BuildingAtmosphereComponent` initialized with ambient baseline values (e.g., 20.0°C and 100.0 air quality).

---

### Requirement: Interior Environmental Buffering
The `TemperatureSystem` SHALL pull the `temperature` of each `BuildingAtmosphereComponent` toward the temperature of the exterior macro-cell that the building occupies, but at a heavily damped rate (insulation).

#### Scenario: Building shelters from extreme heat
- **WHEN** the exterior macro-cell is at 50.0°C and the building's interior is at 22.0°C
- **THEN** the building's interior temperature SHALL increase very slowly (e.g., 0.01°C per L2 tick)

---

### Requirement: Interior State Override for Entities
When an entity possesses an `InteriorComponent`, systems evaluating environmental effects (e.g., `BiologySystem` for temperature damage) SHALL read from the linked `BuildingAtmosphereComponent` instead of the exterior grid.

#### Scenario: NPC in a building is protected from heatstroke
- **WHEN** an NPC occupies a macro-cell with 50.0°C exterior temperature but has an `InteriorComponent` linked to a building at 25.0°C
- **THEN** the `BiologySystem` SHALL NOT apply extreme temperature damage.

---

### Requirement: Interior Visual Hiding
The rendering system SHALL NOT render entities that possess an `InteriorComponent`.

#### Scenario: Player enters a building
- **WHEN** the player gains the `InteriorComponent`
- **THEN** the player's glyph SHALL NOT be drawn on the screen until they exit.
