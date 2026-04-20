## 1. Components Update

- [x] 1.1 Update `BiologyComponent` in `src/components.h` to include `OrganHealth` struct with Heart, Lungs, Brain, Liver, Kidneys fields initialized to 100.0.
- [x] 1.2 Update `BiologyComponent` in `src/components.h` to include vital signs: `blood_pressure` array, `heart_rate`, and `oxygen_saturation`.

## 2. Systems Update

- [x] 2.1 Update `BiologySystem::update()` in `src/simulation_systems.h` (or wherever it lives) to decay Liver and Kidney health when `hunger < 10` or `thirst < 10`.
- [x] 2.2 Update citizen death condition in `BiologySystem::update()` to trigger when `health <= 0` or `Brain <= 0` or `Heart <= 0`.
- [x] 2.3 Modify HUD display in `main.cpp` (or UI logic) to show vital signs (e.g., HR, O2) alongside base stats if applicable, or at least ensure new fields are compatible with existing bio inspection.
