## Context

The world is currently simulating a cell-based temperature grid, but it does not differentiate between a citizen standing in an open alley and one sitting inside a high-rise apartment. To provide shelter mechanics and realistic atmospheric behavior, we need to introduce interior micro-environments that buffer against the exterior world.

## Goals / Non-Goals

**Goals:**
- Implement `InteriorComponent` to track when an entity is inside a specific building.
- Introduce `BuildingAtmosphereComponent` to hold per-building temperature and pressure/air-quality values.
- Introduce `AtmosphereGridComponent` as an exterior counterpart to the temperature grid for atmospheric values (e.g., smog, pressure, or air quality).
- Update the `TemperatureSystem` so building interiors slowly equalize with their exterior macro-cell, acting as insulated shelters.
- Allow entities with `InteriorComponent` to experience the building's atmosphere rather than the exterior grid's atmosphere.

**Non-Goals:**
- Rendering physical interiors (rooms, doors) — this is Phase 11 work. We are purely simulating the state.
- Complex fluid dynamics for gas.

## Decisions

- **Interior Tracking:** An `InteriorComponent` will hold the `Entity` ID of the building. Systems checking environmental effects (like `BiologySystem`) will first check for this component. If present, they read from the building's `BuildingAtmosphereComponent`. Otherwise, they read from the exterior grids.
- **Building Insulation:** The `TemperatureSystem` will process all `BuildingAtmosphereComponent`s. It will find the building's exterior macro-cell and pull the building's internal temperature toward the cell's temperature at a very slow rate (e.g., `insulation_factor = 0.01`), simulating a sealed environment buffering external extremes.
- **Entering/Exiting:** Since physical doors aren't rendered yet, we will add a simple interaction mechanism: pressing a designated key (e.g., 'E' or 'Space') while near a building's center or within its footprint will toggle the `InteriorComponent` on the player. Citizens (NPCs) will automatically add/remove the component when their schedule dictates they are "WORKING" or "SLEEPING" inside their assigned buildings.

## Risks / Trade-offs

- **[Risk]** Entities visually appearing on the street while "inside". → **Mitigation**: We will disable the rendering of entities (or render them with a distinct interior overlay/transparency) when they possess an `InteriorComponent`.
- **[Risk]** Updating thousands of buildings. → **Mitigation**: Buildings are processed linearly in the L2 gate. The math is a simple linear interpolation, which is highly efficient.
