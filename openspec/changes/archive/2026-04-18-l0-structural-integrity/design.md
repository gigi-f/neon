## Context

The Neon world is currently physically static. To introduce urban decay and environmental pressure, we need a system that tracks the physical health of structures and infrastructure, decaying them over time and in response to hazards like acid rain.

## Goals / Non-Goals

**Goals:**
- Implement a `StructuralComponent` for buildings/infrastructure.
- Create a `StructuralDecaySystem` that evolves integrity over time.
- Implement `WeatherState` in `TimeOfDayComponent`.
- Implement `ACID_RAIN` impact on structural decay and NPC health.
- Provide visual feedback for decaying structures.

**Non-Goals:**
- Complex physics-based collapse (simple `CollapsedComponent` for now).
- Material-specific heat conductivity (staying at macro-cell temperature).
- Repair/Maintenance systems (to be handled in future L3 Economic changes).

## Decisions

### 1. StructuralComponent Design
- **Decision**: Store `integrity` as a float [0, 100] and `material_type` as an enum.
- **Rationale**: Simple, low memory overhead, and allows for differentiated decay rates.
- **Alternatives**: Using a full material property struct. Rejected for initial MVP as it adds unnecessary complexity.

### 2. Weather State Management
- **Decision**: Add `WeatherState` enum to the `TimeOfDayComponent` singleton.
- **Rationale**: Weather is a global environmental property, fitting well alongside Time of Day.
- **Alternatives**: A separate `WeatherComponent`. Rejected to minimize singleton bloat.

### 3. Decay System Cadence
- **Decision**: Run `StructuralDecaySystem` at the L3 tick rate (1 Hz).
- **Rationale**: Decay is a slow process; 1 Hz is more than sufficient for tracking urban decay without taxing the CPU.

### 4. Exposure Mapping
- **Decision**: For now, buildings are assumed `is_exposed = true`. Future iterations can use Raycasting or Z-layer checks for "interior" structures.
- **Rationale**: Simplifies initial implementation while achieving the core goal of acid rain impact.

## Risks / Trade-offs

- **[Risk]** Rapid decay in acid rain might destroy the city too quickly.
  - **Mitigation**: Use conservative multipliers and ensure `ACID_RAIN` probability is tuned to be a rare event.
- **[Risk]** High entity count for `StructuralComponent`.
  - **Mitigation**: Iterate only over buildings and static infrastructure, which are already managed in dense arrays or views.
