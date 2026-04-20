## Why

The city currently lacks a power infrastructure simulation. Introducing a power grid with nodes, conduits, and arterial supply lines is necessary to lay the groundwork for environmental hazard events (e.g. power blackouts, grid failures), economic impacts of power outages, and to add crucial depth to the L0 Physics & Environment layer.

## What Changes

- Add a new `PowerGridComponent` (or multiple related components like `PowerNodeComponent`, `PowerConduitComponent`) to represent the grid in the ECS.
- Introduce a new `PowerGridSystem` to simulate power flow, consumption, and generation across the grid.
- Update `world_generation.h` to procedurally generate power nodes and conduits.
- Integrate the power simulation into the L0 physics tick rate in the simulation coordinator.

## Capabilities

### New Capabilities
- `power-grid`: Simulates the generation, transmission, and consumption of electrical power across the city's grid via nodes and conduits.

### Modified Capabilities
- 

## Impact

- ECS will manage new `PowerNodeComponent` and `PowerConduitComponent` instances.
- World generation will place power nodes (e.g. substations, generators) and connect them.
- Future systems (L1-L4) and environmental hazard systems will depend on power availability from this grid.