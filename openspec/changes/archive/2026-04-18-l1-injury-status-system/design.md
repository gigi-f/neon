## Context

`BiologyComponent` (`components.h:300`) tracks health, hunger, thirst, fatigue. `BiologySystem` (L1, 60 Hz) decays needs and cascades damage to health. The existing MEDICAL branch in `ConsumableSystem::update()` (`simulation_systems.h:1305`) simply does `bio.health += restore_value` — no injury awareness.

The current death pipeline: health → 0 → citizen destroyed → `ViolenceEvent` emitted → witnesses record SAW_VIOLENCE. There is no intermediate wounded state.

## Goals / Non-Goals

**Goals:**
- Add `InjuryComponent` (max 4 slots) and `InjuryType` enum to `components.h`
- Add `InjurySystem` at L1 — per-tick severity evolution and health drain per active injury
- Add `inflictInjury()` free function for use by any system that deals damage
- Integrate MEDICAL item consumption so it clears/reduces injuries, not just restores raw health
- Cascade high cumulative injury severity to PAD pleasure at the L2 tick rate

**Non-Goals:**
- L0 pathogen propagation (future work)
- Inventory UI or item carrying (future work)
- Organ health / vital signs (future work)
- Player-specific HUD for injuries (future work)
- Any L3/L4 changes

## Decisions

### D1: Fixed array of 4 injury slots (not a vector)
InjuryComponent uses `std::array<InjurySlot, 4>` — same pattern as `RelationshipComponent`'s fixed 8-slot table. Avoids heap allocation on every citizen, keeps component cache-friendly. 4 concurrent injuries is realistic and sufficient; excess `inflictInjury` calls do nothing if all slots are filled.

### D2: Per-type severity rates encoded as constants, not virtual dispatch
Each `InjuryType` maps to a fixed heal or worsen delta per tick via a `switch` in `InjurySystem`. Simple, zero-overhead. If rates need tuning they live in one switch block.

Per-type behavior:
| Type | Untreated direction | Rate (severity/s at time_scale=1) |
|---|---|---|
| LACERATION | heals | −0.002 |
| INTERNAL_BLEEDING | worsens | +0.005 |
| BROKEN_LIMB | heals very slowly | −0.0005 |
| CONCUSSION | heals slowly | −0.001 |
| BURN | heals slowly | −0.001 |

### D3: Health drain formula
Each active slot drains: `health -= severity * 0.5f * time_scale * dt` per tick (on top of BiologySystem's starvation drain). INTERNAL_BLEEDING at severity 1.0 drains ~1.8 HP/s at time_scale=60 — lethal in ~44 game-seconds without treatment.

### D4: PAD cascade runs inside InjurySystem at L2 cadence
Rather than adding another pass or modifying CognitiveSystem, InjurySystem accepts a `bool run_l2` flag. When true, it also walks citizens with both InjuryComponent and CognitiveComponent and applies the pleasure nudge. This mirrors the pattern used by SocialHierarchySystem.

### D5: MEDICAL item changes: injury-aware consumption
In `ConsumableSystem`, the MEDICAL branch (currently `bio.health += restore_value`) changes to:
1. Find the injury slot with highest severity.
2. Reduce that slot's severity by 0.5 (clamped to 0, clearing slot if 0).
3. If no injury present, fall back to `bio.health += restore_value` (unchanged behavior for uninjured NPCs).

### D6: inflictInjury as a free function in simulation_systems.h
Defined above `InjurySystem`. Called today only by `BiologySystem` when a citizen takes melee damage (not yet wired — placeholder for when violence deals damage, not just kills). Ready to call from any future system (crime, pathogen, etc.).

## Risks / Trade-offs

- **N² PAD cascade**: InjurySystem's L2 PAD pass iterates all citizens — same pattern as SocialHierarchySystem Pass 2 but cheaper (no pair loop, just per-entity). At 500 citizens this is negligible.
- **INTERNAL_BLEEDING worsening at L1 60 Hz**: Rate is small (+0.005/s), but ensure `dt` is real seconds not game-seconds to avoid runaway at high time_scale. InjurySystem MUST use raw `dt`, not `time_scale * dt`, for severity evolution — health drain uses `time_scale * dt`.
- **Slot exhaustion**: `inflictInjury` silently fails when all 4 slots are full. This is intentional — NPCs with 4 active injuries are already near death.
