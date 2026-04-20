## Why

The current environmental simulation (temperature) treats the entire world as an open, uniform exterior. To create meaningful survival mechanics (shelter from storms, breathable air, smog accumulation) and realistic L0 physics, the simulation must differentiate between open exterior spaces and enclosed interior spaces. Pressure and ventilation modeling establishes atmospheric containment, allowing buildings to act as isolated micro-environments that shield inhabitants from external extremes and handle gas diffusion.

## What Changes

- **New**: `AtmosphereGridComponent` singleton — stores a 2D grid of pressure/gas concentration values at macro-cell resolution, similar to the temperature grid.
- **New**: `InteriorComponent` — attached to entities (Player, NPCs) when they are inside a building, tracking the `stable_id` of the building.
- **New**: `BuildingAtmosphereComponent` — attached to enterable buildings to track their internal temperature and pressure/ventilation state independently of the macro-cell they occupy.
- **New**: `AtmosphereSystem` at L2 — handles atmospheric diffusion (wind/pressure equalization) across the macro grid, and models slow leakage between exterior macro-cells and building interiors based on an insulation/ventilation factor.
- **Modified**: `TemperatureSystem` (L2) — updated to sync building interior temperatures slowly with the exterior, allowing interiors to provide shelter.
- **Modified**: `BiologySystem` (L1) — updated to read from `BuildingAtmosphereComponent` if an entity has an `InteriorComponent`, rather than the exterior `TemperatureGridComponent`.

## Capabilities

### New Capabilities
- `pressure-ventilation`: Atmospheric pressure/gas grid, interior vs. exterior environment separation, building micro-environments (temperature/pressure), and ventilation leakage logic.

### Modified Capabilities
- `temperature-simulation`: Added support for building interior temperatures buffering against exterior macro-cell extremes.

## Impact

- `src/components.h` — `AtmosphereGridComponent`, `InteriorComponent`, `BuildingAtmosphereComponent` structs.
- `src/simulation_systems.h` — `AtmosphereSystem` implementation, modifications to `TemperatureSystem` and `BiologySystem`.
- `src/main.cpp` — Initialize atmospheric grids and wire the new system into the L2 tick gate.
- `src/world_generation.h` — Initialize `BuildingAtmosphereComponent` on generated buildings.
