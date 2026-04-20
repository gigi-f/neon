## Purpose
Track granular injury statuses on biological entities, evolve untreated injury severity, drain health from active wounds, and cascade severe pain into L2 cognition.

## Requirements

### Requirement: InjuryType enum and InjurySlot struct
`components.h` SHALL declare, under the `// ── L1 Biology ──` banner:
```cpp
enum class InjuryType { NONE, LACERATION, INTERNAL_BLEEDING, BROKEN_LIMB, CONCUSSION, BURN };

struct InjurySlot {
    InjuryType type     = InjuryType::NONE;
    float      severity = 0.0f;  // [0.0, 1.0]; 0 = slot empty
};
```

#### Scenario: Default slot is empty
- **WHEN** an `InjurySlot` is default-constructed
- **THEN** `type` SHALL be `InjuryType::NONE` and `severity` SHALL be `0.0f`

---

### Requirement: InjuryComponent
`components.h` SHALL declare `InjuryComponent` under the `// ── L1 Biology ──` banner:
```cpp
struct InjuryComponent {
    static constexpr int MAX_SLOTS = 4;
    std::array<InjurySlot, MAX_SLOTS> slots{};
};
```
A slot is considered **active** if `slot.type != InjuryType::NONE && slot.severity > 0.0f`.

#### Scenario: Fresh component has no active injuries
- **WHEN** `InjuryComponent` is default-constructed
- **THEN** all 4 slots SHALL be inactive

---

### Requirement: inflictInjury free function
`simulation_systems.h` SHALL expose:
```cpp
inline void inflictInjury(Registry& registry, Entity e, InjuryType type, float severity);
```
Behaviour:
1. If `e` does not have `InjuryComponent`, add it via `registry.emplace<InjuryComponent>(e)`.
2. Find the first slot where `slot.type == InjuryType::NONE`.
3. If a free slot exists, set `slot.type = type` and `slot.severity = std::clamp(severity, 0.0f, 1.0f)`.
4. If all 4 slots are occupied, do nothing (silent drop).

#### Scenario: First injury fills slot 0
- **WHEN** `inflictInjury(reg, e, InjuryType::LACERATION, 0.6f)` is called on an entity with no InjuryComponent
- **THEN** slot 0 SHALL have type `LACERATION` and severity `0.6f`

#### Scenario: Full component drops new injuries
- **WHEN** all 4 slots are active and `inflictInjury` is called
- **THEN** no slot SHALL change

---

### Requirement: InjurySystem — severity evolution
`InjurySystem::update(Registry&, float dt, float time_scale, bool run_l2)` SHALL run at the L1 tick gate (every 10 frames, approximately 6 Hz at 60 fps).

**Pass 1 — severity evolution** (every call, uses raw `dt` NOT `time_scale * dt`):

For each entity with `InjuryComponent`:
- For each active slot, apply per-type delta per second:

| InjuryType | Untreated direction | Δ severity / real second |
|---|---|---|
| LACERATION | heals | −0.002 |
| INTERNAL_BLEEDING | worsens | +0.005 |
| BROKEN_LIMB | heals | −0.0005 |
| CONCUSSION | heals | −0.001 |
| BURN | heals | −0.001 |

- Clamp severity to [0.0, 1.0].
- If severity reaches 0.0, set `slot.type = InjuryType::NONE` (clear slot).

#### Scenario: Laceration heals over time
- **WHEN** a citizen has a LACERATION with severity 0.5 and 250 real seconds elapse (no treatment)
- **THEN** severity SHALL be approximately 0.0 and slot SHALL be cleared

#### Scenario: Internal bleeding worsens without treatment
- **WHEN** a citizen has INTERNAL_BLEEDING at severity 0.3 and no MEDICAL item is consumed
- **THEN** severity SHALL increase each InjurySystem tick

---

### Requirement: InjurySystem — health drain
`InjurySystem` SHALL drain `BiologyComponent.health` from each entity that has active injury slots.

**Pass 1 continued** (requires entity also has `BiologyComponent`):

For each active slot: `bio.health -= slot.severity * 0.5f * time_scale * dt`

`bio.health` SHALL be clamped to ≥ 0.0f after the loop (matching existing BiologySystem pattern).

#### Scenario: Severe internal bleeding drains health
- **WHEN** a citizen has INTERNAL_BLEEDING at severity 1.0 and time_scale = 60
- **THEN** health SHALL decrease by approximately 1.8 HP per real second

#### Scenario: Cleared injury stops health drain
- **WHEN** all injury slots are inactive (type NONE or severity 0)
- **THEN** no health drain SHALL be applied by InjurySystem

---

### Requirement: InjurySystem — PAD pleasure cascade
`InjurySystem` SHALL reduce `CognitiveComponent.pleasure` for entities whose cumulative active injury severity exceeds the pain threshold during L2-aligned updates.

**Pass 2 — PAD cascade** (only when `run_l2 == true`):

For each entity with `InjuryComponent` AND `CognitiveComponent`:
- Compute `total_severity = sum of severity across all active slots`.
- If `total_severity > 0.5f`: `cog.pleasure = std::max(-1.0f, cog.pleasure - 0.1f)`.

#### Scenario: High total severity reduces pleasure
- **WHEN** a citizen has two injuries with severity 0.4 each (total = 0.8 > 0.5) and `run_l2 = true`
- **THEN** `cog.pleasure` SHALL decrease by 0.1 (clamped to -1.0)

#### Scenario: Low total severity has no PAD effect
- **WHEN** total active severity ≤ 0.5 or `run_l2 = false`
- **THEN** `cog.pleasure` SHALL NOT be modified by InjurySystem

---

### Requirement: SimulationCoordinator wiring
`main.cpp` SHALL instantiate `InjurySystem` and call it in the L1 gate:
```cpp
injurySystem.update(registry, dt, time_scale, coordinator.tick_l2());
```
`InjurySystem` SHALL run after `BiologySystem` and before `MemoryFormationSystem`.

#### Scenario: InjurySystem runs every frame at L1
- **WHEN** the L1 coordinator gate opens for a frame
- **THEN** `InjurySystem::update()` SHALL be called once
