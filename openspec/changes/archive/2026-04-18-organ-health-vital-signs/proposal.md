## Why

To deepen the L1 Biology & Metabolism simulation, NPCs need more granular health tracking than a single health bar. Organ health and vital signs provide a foundation for complex medical statuses, injuries, disease propagation, and nuanced death cascades, making the city feel more alive and consequential.

## What Changes

- Addition of an `OrganHealth` struct or similar granular representation of vital organs (e.g., Heart, Lungs, Brain, Liver, Kidneys).
- Addition of vital signs tracking (e.g., Blood Pressure, Heart Rate, Oxygen Saturation).
- Modification of `BiologyComponent` to include these granular health metrics.
- Systems logic to degrade overall health based on critical organ failure or vital sign anomalies.

## Capabilities

### New Capabilities
- `organ-health`: Granular health tracking for individual organs (Heart, Lungs, Brain, etc.) and vital signs (Blood Pressure, Heart Rate, Oxygen Saturation) that influence overall survival and states.

### Modified Capabilities
- `biology-metabolism`: Expanded to factor in organ health and vitals alongside the existing hunger, thirst, and fatigue metrics.

## Impact

- `BiologyComponent` (`src/components.h`) will increase in size to accommodate new structs/fields.
- `L1_BiologySystem` (presumably in `src/simulation_systems.h` or `src/systems.h`) will need updates to process organ state decay and vital signs fluctuations based on metabolism or trauma.
- UI/HUD Inspection Panels (`Biological Audit`) will need to expose these new values to the player.
