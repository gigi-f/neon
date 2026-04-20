## Purpose
Simulation of metabolic decay (hunger, thirst, fatigue) and basic health cascades for citizens and the player.
## Requirements
### Requirement: BiologyComponent
Every citizen NPC and the player SHALL carry a `BiologyComponent` with four float stats:
`health` (0–100), `hunger` (0–100), `thirst` (0–100), `fatigue` (0–100).
All stats initialize at 80.0 (well-fed, healthy).

#### Scenario: Component Assignment
- **WHEN** a citizen entity is spawned by `AmbientSpawnSystem`
- **THEN** it SHALL receive a default `BiologyComponent` (all stats = 80.0)
- **AND WHEN** the player entity is created at startup
- **THEN** it SHALL also receive a `BiologyComponent`

### Requirement: Metabolic Decay
`BiologySystem::update()` SHALL run at the L1 tick gate (every 10 frames ≈ 6 Hz at 60 fps).
Stats decay per real second, scaled by `TimeOfDayComponent.time_scale` (game-minutes / real-second):

| Stat    | Rate                          |
|---------|-------------------------------|
| hunger  | 0.5 × time_scale per call-dt  |
| thirst  | 0.8 × time_scale per call-dt  |
| fatigue | 0.2 × time_scale per call-dt  |

All stats clamp to [0, 100].

#### Scenario: Health Cascade
- **WHEN** `hunger < 10` OR `thirst < 10` for an entity
- **THEN** `health` SHALL decay at `1.0 × time_scale × dt` per update, clamped to 0

#### Scenario: Citizen Death
- **WHEN** a citizen's `health <= 0`
- **THEN** the entity SHALL be destroyed via `registry.destroy()` (collected after iteration)

### Requirement: Player Bio HUD
The on-screen HUD SHALL display the player's bio stats below the existing status lines
using `drawText()` whenever the player entity has a `BiologyComponent`.

#### Scenario: HUD Display
- **WHEN** the player entity has a `BiologyComponent`
- **THEN** the bio stats SHALL be rendered on the HUD using `drawText()` with color coding based on stat levels.

### Requirement: Infrastructure Unchanged
Road, transit, intersection, and spawning systems SHALL NOT be modified beyond
adding the `BiologyComponent` assignment in `spawnCitizens()`.

#### Scenario: No Regressions
- **WHEN** biology systems are updated
- **THEN** core road and transit systems SHALL maintain their baseline functionality.

### Requirement: Organ-Based Decay and Death
In addition to generic metabolic decay, organs SHALL decay during starvation/dehydration, and critical organ failure SHALL trigger death.

#### Scenario: Organ Starvation Decay
- **WHEN** `hunger < 10` OR `thirst < 10` for an entity
- **THEN** Liver and Kidney health SHALL decay at `0.1 × time_scale × dt` per update, clamped to 0.

#### Scenario: Critical Organ Failure
- **WHEN** Brain health `<= 0` OR Heart health `<= 0`
- **THEN** the entity SHALL be destroyed via `registry.destroy()`.

