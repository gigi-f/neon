## Context

The world is currently a static 2D grid with no environmental simulation. While biology and economics are implemented, they are not influenced by the physical state of the environment. L0 Physics aims to introduce cell-based simulation layers, starting with Temperature.

## Goals / Non-Goals

**Goals:**
- Implement a macro-cell based temperature grid (40 WU resolution).
- Implement thermal diffusion between adjacent cells.
- Implement equilibrium toward an ambient target defined by the TimeOfDay cycle.
- Allow specific zones (URBAN_CORE, INDUSTRIAL) to act as heat sources.
- Cascade extreme temperature effects to NPC health (BiologySystem).

**Non-Goals:**
- Individual building insulation or interior temperature (for now).
- Fluid dynamics or complex wind-driven heat transfer.
- Structural material melting or freezing.

## Decisions

- **Singleton Grid Storage:** The `TemperatureGridComponent` will be a singleton attached to the `worldConfig` entity. It will store a `std::vector<float>` representing the flattened 2D grid.
- **Resolution:** 40x40 World Units per cell. This matches the existing `macro_cell_size` and provides a good balance between simulation fidelity and performance.
- **Update Frequency:** The `TemperatureSystem` will run at the **L2 gate** (~2 Hz). This is sufficient for atmospheric temperature changes which are slower than physics but faster than day cycles.
- **Diffusion Algorithm:** A simple 4-neighbor average with a diffusion coefficient: `new_temp = old_temp + diffusion * (neighbor_avg - old_temp)`.
- **Equilibrium:** Cells will slowly pull toward an `ambient_target` defined in the `TimeOfDayComponent`.
- **Health Drain:** A simple threshold-based drain in `BiologySystem`. If `temp > 45.0` (Heatstroke) or `temp < -5.0` (Hypothermia), health decreases.

## Risks / Trade-offs

- **[Risk]** Large world sizes causing `std::vector` overhead. → **Mitigation**: World size is currently bounded at -1000 to 1000 WU, resulting in a 50x50 grid (2500 floats), which is negligible.
- **[Risk]** Memory access patterns for diffusion. → **Mitigation**: Use a double-buffer approach (read from current, write to back-buffer) to ensure stable diffusion and avoid order-of-operation artifacts.
- **[Trade-off]** Choosing 40 WU over a finer grid. → **Trade-off**: Lower resolution means heat sources are "blocky," but it aligns perfectly with the current city generation blocks.
