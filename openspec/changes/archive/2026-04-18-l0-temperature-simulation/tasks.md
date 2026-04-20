## 1. Data Model

- [x] 1.1 Add `TemperatureGridComponent` to `src/components.h` (singleton with `std::vector<float>` grid and dimensions)
- [x] 1.2 Update `TimeOfDayComponent` in `src/components.h` with `ambient_target` temperature field
- [x] 1.3 Add ambient temperature targets per phase (DAWN/DAY/DUSK/NIGHT) to `WorldConfigComponent` or `TimeOfDaySystem`

## 2. Temperature System

- [x] 2.1 Implement `TemperatureSystem` in `src/simulation_systems.h` with thermal diffusion logic
- [x] 2.2 Add equilibrium logic to `TemperatureSystem` pulling cells toward `ambient_target`
- [x] 2.3 Add zone-based heat injection logic (URBAN_CORE/INDUSTRIAL) using `ZoningComponent` spatial mapping

## 3. Integration & Wiring

- [x] 3.1 Initialize `TemperatureGridComponent` on the `worldConfig` entity in `src/main.cpp`
- [x] 3.2 Wire `TemperatureSystem::update` into the L2 tick gate in `src/main.cpp`
- [x] 3.3 Update `BiologySystem::update` in `src/simulation_systems.h` to apply health drain for extreme temperatures (>45°C, <-5°C)

## 4. Verification

- [x] 4.1 Log temperature min/max/avg to console at 1Hz to verify simulation stability
- [x] 4.2 Confirm heat island formation (higher temps in URBAN_CORE vs SLUM/RESIDENTIAL)
- [x] 4.3 Verify NPC health decrease when spawning/moving into high-heat zones
