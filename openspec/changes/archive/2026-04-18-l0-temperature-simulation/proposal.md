## Why

The current simulation lacks environmental depth. Temperature provides a critical L0 physics layer that drives survival pressure (L1), NPC behavior (L2), and structural decay (L0). Establishing a macro-cell based temperature grid is the first step toward a coherent environmental simulation including heat islands, power grid heat, and acid rain.

## What Changes

- **New**: `TemperatureGridComponent` singleton — stores a 2D grid of temperature values at macro-cell resolution (40 WU).
- **New**: `TemperatureSystem` at L2 — handles thermal diffusion, ambient equilibrium toward time-of-day targets, and heat source propagation from specific zone types (URBAN_CORE, INDUSTRIAL).
- **Modified**: `BiologySystem` (L1) — health cascades when NPCs are exposed to extreme temperature cells without protection.
- **Modified**: `WorldConfigComponent` — added ambient base temperature targets for DAWN/DAY/DUSK/NIGHT.

## Capabilities

### New Capabilities
- `temperature-simulation`: Macro-cell based temperature grid (40 WU resolution), thermal diffusion between adjacent cells, ambient equilibrium logic based on time-of-day, and zone-based heat modifiers.

### Modified Capabilities
<!-- No requirement changes to existing capabilities -->

## Impact

- `src/components.h` — `TemperatureGridComponent` struct, `WorldConfigComponent` updates.
- `src/simulation_systems.h` — `TemperatureSystem` implementation.
- `src/main.cpp` — Initialize temperature grid on `worldConfig` entity.
- `src/world_generation.h` — Initial temperature seeding based on zone layout.
