## Why

The Neon Oubliette simulation requires dynamic environmental hazard events to challenge the city's infrastructure and its inhabitants. Introducing these hazards adds necessary depth to the L0 layer (Physics & Environment), forcing responses from higher simulation layers such as structural integrity decay, biological health impact, and traffic flow disruption.

## What Changes

- Implement a system to trigger and manage environmental hazards.
- Add Acid Rain, which accelerates structural decay on exposed buildings.
- Add Smog, which severely reduces air quality (AtmosphereGrid) and impacts biological entities.
- Add a Flooding placeholder event as a localized hazard template.
- Update the time-of-day and weather systems to transition into and out of these hazard states seamlessly.

## Capabilities

### New Capabilities
- `environmental-hazards`: The core system responsible for triggering, managing duration, and applying the effects of acid rain, smog, and flooding placeholder.

### Modified Capabilities
- `structural-integrity`: Modified to factor in acid rain exposure as an accelerated decay modifier.
- `temperature-simulation`: (Wait, atmosphere grid) Modified to handle smog air quality debuffs.

## Impact

- `src/simulation_systems.h`: New `HazardSystem` or updates to existing systems like `TimeOfDaySystem`, `AtmosphereSystem` and `StructuralDecaySystem`.
- `src/components.h`: Modifications to `TimeOfDayComponent` and `WeatherState` to trigger hazard states.
- L1 Biological and L0 Structural layers will be directly impacted by these new L0 hazards.