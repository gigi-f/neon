# environmental-hazards Specification

## Purpose
TBD - created by archiving change environmental-hazards. Update Purpose after archive.
## Requirements
### Requirement: Global Weather State Transitions
The `TimeOfDaySystem` SHALL evaluate weather transitions during time-of-day phase shifts (e.g., DAWN to DAY).
- **Probabilities**: Base chances for CLEAR (70%), OVERCAST (15%), ACID_RAIN (10%), SMOG (5%).

#### Scenario: Weather changes at dawn
- **WHEN** the `TimeOfDayComponent.phase` transitions to `DAWN`
- **THEN** the system SHALL roll a probability check to update the `weather` field.

### Requirement: Smog Atmosphere Impact
When the global weather state is `SMOG`, the `AtmosphereSystem` SHALL reduce the `air_quality` of all cells in the `AtmosphereGridComponent` by 50.0.

#### Scenario: Smog reduces air quality
- **WHEN** the weather transitions to `SMOG`
- **THEN** all exterior macro-cells SHALL drop their `air_quality` significantly.

### Requirement: Flooding Placeholder Hazard
The simulation SHALL support a placeholder `FLOODING` event that can be triggered independently of the weather state, affecting movement speed.

#### Scenario: Lowland areas flood
- **WHEN** a `FLOODING` event is active
- **THEN** any NPC occupying an exposed ground-level position SHALL have their `MovementComponent.speed` reduced by 50%.

