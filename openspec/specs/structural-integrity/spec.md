# structural-integrity Specification

## Purpose
TBD - created by archiving change l0-structural-integrity. Update Purpose after archive.
## Requirements
### Requirement: Material-based Structural Integrity
Buildings and critical infrastructure entities SHALL possess a `StructuralComponent` containing:
- `integrity`: Float (0.0 to 100.0)
- `material_type`: Enum (STEEL, REINFORCED_CONCRETE, COMPOSITE, SCRAP)
- `last_maintenance_tick`: 64-bit integer
- `is_exposed`: Boolean (True if exterior, False if interior/sheltered)

#### Scenario: Component initialization
- **WHEN** a new building entity is created
- **THEN** it SHALL be assigned a `StructuralComponent` with `integrity` set to 100.0 and a `material_type` based on its zone and building type.

---

### Requirement: Time-based Urban Decay
The `StructuralDecaySystem` SHALL, at each L3 tick (1 Hz), reduce the `integrity` of all entities with a `StructuralComponent` based on their `material_type`.
- **Base Decay Rates**:
  - STEEL: 0.0001% per tick
  - REINFORCED_CONCRETE: 0.0002% per tick
  - COMPOSITE: 0.00005% per tick
  - SCRAP: 0.001% per tick

#### Scenario: Buildings decay over time
- **WHEN** the simulation runs for several L3 ticks
- **THEN** the `integrity` value of a building SHALL decrease by its material's base decay rate.

---

### Requirement: Acid Rain Damage Acceleration
When the environmental state is set to `ACID_RAIN`, the `StructuralDecaySystem` SHALL multiply the decay rate of all entities with `is_exposed = True` by a factor of 50.0.

#### Scenario: Acid rain accelerates exterior decay
- **WHEN** the environment state transitions to `ACID_RAIN`
- **THEN** an exposed building with `material_type = REINFORCED_CONCRETE` SHALL lose 0.01% integrity per tick (0.0002% * 50.0).

---

### Requirement: Visual Representation of Decay
The rendering system SHALL update the glyph color of buildings based on their `integrity` level.
- **Integrity > 80%**: Default color
- **80% >= Integrity > 40%**: Shift toward a "worn" color (e.g., muted grey/brown)
- **40% >= Integrity > 0%**: Shift toward a "damaged" color (e.g., dark rust/dark grey)

#### Scenario: Building color changes as it decays
- **WHEN** a building's `integrity` drops from 90.0 to 75.0
- **THEN** its rendered color SHALL transition to the "worn" palette.

---

### Requirement: Collapse Trigger
When an entity's `integrity` reaches 0.0, it SHALL be marked with a `CollapsedComponent`.

#### Scenario: Building reaches zero integrity
- **WHEN** a building's `integrity` drops to 0.0
- **THEN** the entity SHALL receive a `CollapsedComponent`
- **AND** it SHALL no longer provide shelter or functionality.

