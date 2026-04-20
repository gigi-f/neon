## Why

Currently, the city of Neon is physically static. Buildings and infrastructure do not degrade, which removes a critical layer of environmental pressure and economic necessity (maintenance/repairs). Implementing structural integrity and material decay creates a "ticking clock" for the urban environment, driving demand for resources and providing a tangible consequence for environmental hazards like acid rain.

## What Changes

- **Structural Integrity Component**: Add a new component to track the physical health of buildings and infrastructure entities.
- **Decay System**: Implement a system that reduces structural integrity over time based on material type and environmental exposure.
- **Acid Rain Acceleration**: Environmental states (Acid Rain) will now have a direct mechanical impact by significantly accelerating the decay of exposed structures.
- **Visual Feedback**: The ASCII renderer will be updated to reflect decaying states (e.g., color shifts or glyph changes) when integrity drops below certain thresholds.
- **Collapse Mechanics**: Foundations for structural failure when integrity reaches zero.

## Capabilities

### New Capabilities
- `structural-integrity`: Defines the requirements for how physical structures track health, decay over time, and respond to environmental stressors like acid rain.

### Modified Capabilities
- `temperature-simulation`: Update to include "Acid Rain" as a formal environmental state that interacts with the L0 physics layer (acceleration of material decay).

## Impact

- **ECS**: New `StructuralComponent` added to `components.h`.
- **Systems**: New `StructuralDecaySystem` in `simulation_systems.h`.
- **Environment**: Update `TemperatureSystem` (or environment coordinator) to trigger acid rain effects.
- **Rendering**: Updates to `main.cpp` or glyph lookups to handle damaged visual states.
