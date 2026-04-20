## 1. ECS Infrastructure

- [x] 1.1 Add `WeatherState` enum and `weather` field to `TimeOfDayComponent` in `src/components.h`.
- [x] 1.2 Add `MaterialType` enum and `StructuralComponent` to `src/components.h`.
- [x] 1.3 Add `CollapsedComponent` to `src/components.h`.

## 2. Environment & Decay Systems

- [x] 2.1 Update `TemperatureSystem::update` in `src/simulation_systems.h` to evolve `WeatherState` based on ambient conditions.
- [x] 2.2 Implement `StructuralDecaySystem` in `src/simulation_systems.h` with material-based decay rates.
- [x] 2.3 Add `ACID_RAIN` multiplier to `StructuralDecaySystem`.
- [x] 2.4 Add minor health drain to `BiologySystem` for NPCs exposed to `ACID_RAIN`.

## 3. World Generation & Initialization

- [x] 3.1 Update building generation in `src/world_generation.h` to attach `StructuralComponent` with appropriate `material_type` based on zone.
- [x] 3.2 Initialize global `WeatherState::CLEAR` in `main.cpp`.

## 4. Visuals & Feedback

- [x] 4.1 Update building rendering in `main.cpp` to adjust colors based on `integrity` levels.
- [x] 4.2 Add `CollapsedComponent` visual state (e.g., specific glyph or dark color).

## 5. Integration & Validation

- [x] 5.1 Wire `StructuralDecaySystem` into `SimulationCoordinator`.
- [x] 5.2 Verify decay rates over time in Debug HUD or logs.
- [x] 5.3 Force `ACID_RAIN` state to verify acceleration and NPC damage.
