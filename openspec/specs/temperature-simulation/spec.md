## Purpose

Defines the grid-based thermal simulation for the city of Neon, including heat diffusion, ambient equilibrium, and zone-based heat sources.
## Requirements
### Requirement: TemperatureGridComponent data model
The worldConfig entity SHALL carry a `TemperatureGridComponent` that maintains a 2D grid of floating-point temperature values (Celsius) at a resolution of `macro_cell_size` (40 WU).

#### Scenario: Grid initialized at start
- **WHEN** the simulation starts
- **THEN** a `TemperatureGridComponent` SHALL be initialized with dimensions derived from `world_min` and `world_max`
- **AND** all cells SHALL be initialized to the starting ambient temperature (e.g., 20.0°C)

---

### Requirement: Thermal Diffusion and Equilibrium
The `TemperatureSystem` SHALL, at each L2 tick (~2 Hz), evolve the temperature grid according to:
1. **Diffusion**: Heat SHALL spread between adjacent macro-cells (4-way connectivity) with a configurable diffusion coefficient.
2. **Ambient Equilibrium**: Every cell SHALL pull toward the `ambient_target` (from `TimeOfDayComponent`) at a configurable rate.

#### Scenario: Heat spreads from hot to cold
- **WHEN** a cell is significantly hotter than its neighbors
- **THEN** over subsequent L2 ticks, the temperature in neighboring cells SHALL increase

#### Scenario: Environment cools at night
- **WHEN** the `TimeOfDayComponent.phase` is NIGHT (target 12.0°C) and the grid is at 25.0°C
- **THEN** all cells SHALL slowly decrease toward 12.0°C

---

### Requirement: Zone-based Heat Sources
Specific zones SHALL inject heat into their corresponding macro-cells each L2 tick.
- `URBAN_CORE`: +0.5°C per tick
- `INDUSTRIAL`: +0.8°C per tick

#### Scenario: Industrial zones create heat islands
- **WHEN** a macro-cell contains one or more buildings in an `INDUSTRIAL` zone
- **THEN** its temperature SHALL be consistently higher than rural/residential cells

---

### Requirement: Biological Temperature Cascade
The `BiologySystem` (L1) SHALL apply a health penalty to any NPC occupying a macro-cell with extreme temperature.
- **Heatstroke Threshold**: > 45.0°C
- **Hypothermia Threshold**: < -5.0°C

#### Scenario: NPC in extreme heat takes damage
- **WHEN** an NPC's position maps to a macro-cell with temperature 50.0°C
- **THEN** its `health` SHALL decrease by 2.0 per game-hour equivalent

### Requirement: Environmental Weather States
The `TemperatureSystem` SHALL manage a global `WeatherState` enum that includes: `CLEAR`, `OVERCAST`, `ACID_RAIN`, and `SMOG`.

#### Scenario: Weather transitions to Acid Rain
- **WHEN** the ambient humidity (calculated from temperature and zone) is high and the temperature is below 25.0°C
- **THEN** there SHALL be a probabilistic chance to transition to `ACID_RAIN`.

#### Scenario: Acid Rain impacts L1 Biology
- **WHEN** the global weather state is `ACID_RAIN`
- **THEN** any NPC with `is_exposed = True` SHALL receive a minor health penalty of 0.5 per L2 tick.

