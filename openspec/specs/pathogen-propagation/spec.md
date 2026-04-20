# pathogen-propagation Specification

## Purpose
Define deterministic pathogen exposure, infection progression, L2 cognitive impact, debug visibility, and test coverage for active disease state in the simulation.
## Requirements
### Requirement: PathogenComponent
`components.h` SHALL define `PathogenComponent` under the L1 Biology section.

The component SHALL track:
- `strain_id`
- `infection_load`
- `infectiousness`
- `severity`
- `incubation_timer`
- `immune_response`
- `trauma_recorded`

#### Scenario: Default pathogen state is mild but infectious
- **WHEN** `PathogenComponent` is default-constructed
- **THEN** it SHALL represent strain 1 with infection load above the infectious threshold, positive infectiousness, zero severity, non-negative incubation, and positive immune response.

### Requirement: Outdoor proximity transmission
`PathogenSystem::update()` SHALL expose susceptible entities to infectious entities within 24 WU outdoors.

#### Scenario: Nearby outdoor susceptible receives exposure
- **WHEN** an infectious citizen and susceptible citizen are outdoors within 24 WU
- **THEN** the susceptible citizen SHALL receive a `PathogenComponent` or increased matching-strain `infection_load`.

#### Scenario: Distant outdoor susceptible is not exposed
- **WHEN** an infectious citizen and susceptible citizen are outdoors farther than 24 WU apart
- **THEN** proximity transmission SHALL NOT apply.

### Requirement: Shared-space transmission
`PathogenSystem::update()` SHALL expose susceptible entities sharing the same `InteriorComponent.building_entity` as an infectious entity.

#### Scenario: Shared building exposure
- **WHEN** an infectious entity and susceptible entity are inside the same building
- **THEN** the susceptible entity SHALL receive exposure even if their positions are farther than 24 WU apart.

### Requirement: Infection progression
`PathogenSystem::update()` SHALL progress active infections after incubation.

Progression SHALL:
- reduce `incubation_timer` by game-time-scaled delta;
- increase `severity` when incubation is complete and load remains high;
- reduce `infection_load` by immune response over time;
- drain `BiologyComponent.health`;
- drain `BiologyComponent.organs.lungs`;
- lower `BiologyComponent.vitals.oxygen_sat`;
- raise `BiologyComponent.vitals.heart_rate`.

#### Scenario: Symptomatic infection pressures biology
- **WHEN** an infected entity has incubation complete and non-zero severity
- **THEN** health, lungs, and oxygen saturation SHALL decrease over time and heart rate SHALL increase.

### Requirement: L1 wiring
`main.cpp` SHALL instantiate `PathogenSystem` and call it in the L1 gate after `BiologySystem` and before `InjurySystem`.

#### Scenario: Pathogens update with biology
- **WHEN** the L1 simulation gate ticks
- **THEN** pathogen propagation and progression SHALL update once.

### Requirement: L2 mood and trauma cascade
`PathogenSystem::update()` SHALL accept an L2 gate signal for symptomatic infection impact on `CognitiveComponent`.

When the L2 gate is active and an infected entity has symptom severity at or above 0.15:
- `CognitiveComponent.pleasure` SHALL decrease;
- `CognitiveComponent.arousal` SHALL increase.

When symptom severity is at or above 0.35, the entity SHALL record one `BECAME_ILL` memory and SHALL NOT record duplicate illness memories for the same active pathogen component.

#### Scenario: Symptomatic infection impacts mood
- **WHEN** a symptomatic infected entity has `CognitiveComponent`
- **AND** `PathogenSystem::update()` runs with the L2 gate active
- **THEN** pleasure decreases and arousal increases.

#### Scenario: Severe illness records one trauma memory
- **WHEN** a severely symptomatic infected entity has not recorded pathogen trauma
- **AND** `PathogenSystem::update()` runs with the L2 gate active
- **THEN** a `BECAME_ILL` memory is recorded once.

### Requirement: Debug infected counter
`PathogenSystem` SHALL expose a deterministic infected counter based on entities carrying `PathogenComponent`, and the on-screen debug HUD SHALL display that count.

#### Scenario: HUD can report infected entities
- **WHEN** entities carry `PathogenComponent`
- **THEN** the infected counter SHALL equal the number of those entities.

### Requirement: Infected glyph tint
Visible entities carrying `PathogenComponent` SHALL render with an infected glyph tint during normal glyph rendering.

The tint SHALL remain deterministic and SHALL increase visible warmth/intensity as pathogen severity rises.

#### Scenario: Infected entity is visually distinguishable
- **WHEN** an entity has both `GlyphComponent` and `PathogenComponent`
- **THEN** its rendered glyph SHALL use the infected tint instead of the base glyph color.

### Requirement: Deterministic test scaffolding
The build SHALL include a CTest target for `PathogenSystem` deterministic behavior.

#### Scenario: Pathogen tests run through CTest
- **WHEN** CTest runs the pathogen system target
- **THEN** outdoor exposure, shared-interior exposure, progression, L2 mood/trauma cascade, and infected counting SHALL be asserted without random inputs.
