## 1. Data Model (components.h)

- [x] 1.1 Add `InjuryType` enum (`NONE, LACERATION, INTERNAL_BLEEDING, BROKEN_LIMB, CONCUSSION, BURN`) under the `// ── L1 Biology ──` banner
- [x] 1.2 Add `InjurySlot` struct (`InjuryType type`, `float severity`) with defaults `NONE` / `0.0f`
- [x] 1.3 Add `InjuryComponent` struct with `static constexpr int MAX_SLOTS = 4` and `std::array<InjurySlot, MAX_SLOTS> slots{}`

## 2. inflictInjury Free Function (simulation_systems.h)

- [x] 2.1 Add `inline void inflictInjury(Registry& registry, Entity e, InjuryType type, float severity)` above `InjurySystem`
- [x] 2.2 Implement: emplace `InjuryComponent` if absent, find first `NONE` slot, fill it; silent-drop if all slots occupied

## 3. InjurySystem (simulation_systems.h)

- [x] 3.1 Declare `class InjurySystem` with `void update(Registry&, float dt, float time_scale, bool run_l2)`
- [x] 3.2 Implement Pass 1 severity evolution: per-type delta per real second via `switch`, clamp [0,1], clear slot at 0
- [x] 3.3 Implement Pass 1 health drain: `bio.health -= slot.severity * 0.5f * time_scale * dt` per active slot; clamp health ≥ 0
- [x] 3.4 Implement Pass 2 PAD cascade (when `run_l2 == true`): sum active severities; if > 0.5 nudge `cog.pleasure -= 0.1f` clamped to -1

## 4. ConsumableSystem MEDICAL Update (simulation_systems.h)

- [x] 4.1 In `ConsumableSystem::update()` MEDICAL branch: check for `InjuryComponent`; find worst-severity active slot; reduce by 0.5, clear if 0; fall back to `bio.health +=` only when no injuries present
- [x] 4.2 Apply the same injury-aware logic to `ConsumableSystem::playerPickup()` MEDICAL branch

## 5. Wiring (main.cpp)

- [x] 5.1 Instantiate `InjurySystem injurySystem` alongside other system instances
- [x] 5.2 In the L1 gate: call `injurySystem.update(registry, dt, time_scale, coordinator.tick_l2())` after `biologySystem.update()` and before `memoryFormationSystem.update()`

## 6. Verification

- [x] 6.1 Build with `make` (or CMake) — zero new warnings or errors
- [x] 6.2 Spawn a citizen, confirm `InjuryComponent` is absent until `inflictInjury` is called
- [x] 6.3 Manually call `inflictInjury` on a citizen with `INTERNAL_BLEEDING 0.5`; observe health declining in debug HUD
- [x] 6.4 Spawn a MEDICAL item near an injured citizen; confirm injury severity reduces on consumption instead of health restoring
