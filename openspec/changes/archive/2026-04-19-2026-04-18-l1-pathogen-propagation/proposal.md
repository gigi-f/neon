## Why

The L1 biology layer models hunger, thirst, fatigue, organs, and injuries, but citizens still cannot transmit illness through crowding or shared indoor spaces. This leaves dense pedestrian and interior simulation without a biological consequence and blocks later L2 mood/trauma cascades.

## What Changes

- **New**: `PathogenComponent` in `components.h` for strain, load, infectiousness, severity, incubation, and immune response.
- **New**: `PathogenSystem::update()` in `simulation_systems.h`.
- **Modified**: `AmbientSpawnSystem::spawnCitizens()` seeds a small number of infected citizens so propagation has live sources.
- **Modified**: `main.cpp` wires `PathogenSystem` into the L1 gate after `BiologySystem` and before `InjurySystem`, with an L2-gated mood/trauma cascade.
- **Modified**: `main.cpp` debug HUD adds an infected entity counter.
- **New**: `tests/pathogen_system_tests.cpp` covers deterministic exposure, progression, L2 cascade, and infected counting.
- **Modified**: `todo/TODO.md` marks pathogen proximity/shared-space propagation complete.

## Capabilities

### New Capabilities
- `pathogen-propagation`: deterministic L1 pathogen spread by outdoor proximity and same-building shared spaces, infection progression into health, lung, fever, and oxygen effects, and L2 mood/trauma impact from symptomatic infection.

### Modified Capabilities
- `biology-metabolism`: L1 biology now receives pathogen-driven health and vital sign pressure in addition to metabolic/environmental drains.

## Impact

- `src/components.h` — add `PathogenComponent`.
- `src/simulation_systems.h` — add `PathogenSystem`; seed rare infected citizens.
- `src/main.cpp` — instantiate and call `PathogenSystem`.
- L2 cognitive/mood cascade is limited to direct PAD pressure and one illness memory; rumor/quarantine/social policy remain future work.
