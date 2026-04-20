## 1. ECS Components Updates

- [x] 1.1 Add `bool is_flooded = false;` to `TimeOfDayComponent` (or create a separate global hazard component) to track the flooding state independently of the weather.

## 2. Weather Transitions

- [x] 2.1 Update `TimeOfDaySystem::update()` in `src/simulation_systems.h` to roll probabilistic checks on phase transitions (e.g., DAWN to DAY).
- [x] 2.2 Assign probabilities (70% CLEAR, 15% OVERCAST, 10% ACID_RAIN, 5% SMOG) for the new `WeatherState` during these shifts.

## 3. Smog & Atmosphere

- [x] 3.1 Update `AtmosphereSystem::update()` in `src/simulation_systems.h` to subtract `50.0` from `air_quality` across the `AtmosphereGridComponent` when the global weather state is `SMOG`.

## 4. Acid Rain & Structural Decay

- [x] 4.1 Update `StructuralDecaySystem::update()` in `src/simulation_systems.h` to apply a `50.0x` multiplier to the base decay rate for exposed buildings when the global weather state is `ACID_RAIN`.

## 5. Flooding Placeholder

- [x] 5.1 Create a temporary keybinding (e.g., 'F' key) in `src/main.cpp` to toggle the `is_flooded` placeholder state for testing.
- [x] 5.2 Update `MovementSystem::update()` in `src/simulation_systems.h` to reduce `MovementComponent.speed` by 50% for ground-level NPCs (not on elevated paths) when `is_flooded` is true.
