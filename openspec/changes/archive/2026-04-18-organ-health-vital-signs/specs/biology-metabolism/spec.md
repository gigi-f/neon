## ADDED Requirements

### Requirement: Organ-Based Decay and Death
In addition to generic metabolic decay, organs SHALL decay during starvation/dehydration, and critical organ failure SHALL trigger death.

#### Scenario: Organ Starvation Decay
- **WHEN** `hunger < 10` OR `thirst < 10` for an entity
- **THEN** Liver and Kidney health SHALL decay at `0.1 × time_scale × dt` per update, clamped to 0.

#### Scenario: Critical Organ Failure
- **WHEN** Brain health `<= 0` OR Heart health `<= 0`
- **THEN** the entity SHALL be destroyed via `registry.destroy()`.
