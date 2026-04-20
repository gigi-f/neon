## 1. Data Model

- [x] 1.1 Add `AtmosphereGridComponent` to `src/components.h` (singleton with `std::vector<float>` grid and dimensions)
- [x] 1.2 Add `InteriorComponent` to `src/components.h` containing `Entity building_entity`
- [x] 1.3 Add `BuildingAtmosphereComponent` to `src/components.h` containing `temperature` and `air_quality`

## 2. Core Implementation

- [x] 2.1 Update `src/world_generation.h` to assign `BuildingAtmosphereComponent` to generated buildings
- [x] 2.2 Initialize `AtmosphereGridComponent` on the `worldConfig` entity in `src/main.cpp`
- [x] 2.3 Create `AtmosphereSystem` in `src/simulation_systems.h` (similar to `TemperatureSystem` with diffusion) and wire to L2 in `src/main.cpp`

## 3. Interior Logic

- [x] 3.1 Update `TemperatureSystem` in `src/simulation_systems.h` to buffer `BuildingAtmosphereComponent` temperatures against exterior macro-cells
- [x] 3.2 Update `BiologySystem` in `src/simulation_systems.h` to check for `InteriorComponent` and read from `BuildingAtmosphereComponent` if present
- [x] 3.3 Update `src/main.cpp` rendering pass to skip rendering entities that possess an `InteriorComponent`

## 4. Interaction

- [x] 4.1 Update `src/main.cpp` interaction logic (E/Space) to toggle `InteriorComponent` when the player is near a building's center
- [x] 4.2 Verify player glyph is hidden when entering a building, and temperature damage is mitigated
