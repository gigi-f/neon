## 1. ECS Components

- [x] 1.1 Add `PowerNodeComponent` and `PowerConduitComponent` structs to `src/components.h`. The node should have supply, demand, and powered status.

## 2. Simulation System

- [x] 2.1 Create `PowerGridSystem` class in `src/simulation_systems.h` with an `update()` method that calculates supply vs demand and updates the powered status of nodes.
- [x] 2.2 Register and run `PowerGridSystem` in `src/main.cpp` (or `SimulationCoordinator` if it exists) to execute at the L2 or L3 tick rate.

## 3. World Generation

- [x] 3.1 Update `src/world_generation.h` to generate `PowerNodeComponent` for specific buildings or standalone generator entities.