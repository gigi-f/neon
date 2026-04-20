## Context

The Neon Oubliette simulation requires a power grid to simulate the generation, transmission, and consumption of electricity. This will enable environmental hazard events like blackouts, structural decay interactions, and economic effects (loss of productivity or functionality). Currently, we have cell-based temperature and pressure modeling in the L0 physics layer, but no power grid.

## Goals / Non-Goals

**Goals:**
- Implement a power grid simulation as part of the L0 layer.
- Add power nodes (generators, substations, consumers) and conduits (wiring).
- Procedurally generate a baseline power grid during world generation.
- Integrate the grid simulation into `SimulationCoordinator` or the existing L0 physics loop.

**Non-Goals:**
- Implementing actual blackout consequences or cascading failures into L1-L4 layers in this specific change (that will be a follow-up).
- Highly complex AC/DC electrical physics (a simple capacity/demand/flow model is sufficient).

## Decisions

- **Component Design**: 
  - `PowerNodeComponent`: Represents a generator (supply), substation (distribution/storage), or consumer (demand). Tracks current power status.
  - `PowerConduitComponent`: Connects nodes, carrying power over distance.
- **Simulation Flow**:
  - `PowerGridSystem` will run during the L0/L2 update cycle. It will gather total supply, calculate demand, and distribute power along conduits to nodes. If demand exceeds supply, localized or city-wide brownouts occur (flagged on the nodes).
- **Procedural Generation**:
  - `world_generation.h` will place main power generators in specific zones (e.g. Industrial or URBAN_CORE) and run conduits along primary/secondary roads to distribute power to buildings.

## Risks / Trade-offs

- **Risk: Performance overhead of traversing the power graph every tick.** 
  - Mitigation: The power grid topology changes infrequently. We can pre-calculate connected components or use a simpler hierarchical distribution model rather than a full circuit simulation. The grid update will be gated at L2 (2 Hz) or L3 (1 Hz) rather than L0 (60 Hz).