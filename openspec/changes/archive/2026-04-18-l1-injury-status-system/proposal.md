## Why

BiologyComponent tracks health, hunger, thirst, and fatigue — but NPCs who take damage (e.g., caught in violence) have no injury state. Without injuries, lethal encounters collapse instantly to a binary alive/dead with no intermediate suffering, undermining the survival simulation and the planned pathogen/medical systems downstream.

## What Changes

- **New**: `InjuryType` enum (`NONE`, `LACERATION`, `INTERNAL_BLEEDING`, `BROKEN_LIMB`, `CONCUSSION`, `BURN`) in `components.h`
- **New**: `InjurySlot` struct (type, severity float [0–1], duration_ticks int) and `InjuryComponent` (fixed array of 4 slots) in `components.h`
- **New**: `inflictInjury(Registry&, Entity, InjuryType, float severity)` free function — fills next free slot in `InjuryComponent`
- **New**: `InjurySystem::update()` at L1 — per-tick severity decay/worsening, health penalty application, slot clearing at zero severity
- **Modified**: `ConsumableSystem` MEDICAL item branch — clears worst injury slot or reduces severity by 0.5
- **Modified**: `InjurySystem` (or dedicated pass) — PAD pleasure nudge when total injury severity > 0.5 at L2 tick rate

## Capabilities

### New Capabilities
- `injury-status`: InjuryComponent data model, InjuryType enum, inflictInjury API, per-type heal/worsen rates, health cascade, PAD pleasure cascade
- `medical-treatment`: ConsumableSystem integration — MEDICAL items clear or reduce active injuries

### Modified Capabilities
<!-- None — BiologySystem and ConsumableSystem internals change but their spec-level requirements (need decay, MEDICAL heals) are unchanged -->

## Impact

- `components.h` — InjuryType enum, InjurySlot struct, InjuryComponent struct
- `simulation_systems.h` — InjurySystem class, ConsumableSystem MEDICAL branch update, inflictInjury free function
- `main.cpp` — wire InjurySystem into SimulationCoordinator at L1
- No schema changes to existing components (BiologyComponent, CognitiveComponent unchanged)
