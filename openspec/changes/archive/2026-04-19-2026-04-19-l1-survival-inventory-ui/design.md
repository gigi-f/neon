## Context

`ItemComponent` already represents FOOD, WATER, and MEDICAL pickups. `ConsumableSystem` owns item effects and contains injury-aware MEDICAL handling. The player currently calls `ConsumableSystem::playerPickupWithType()` from `main.cpp`, which consumes the nearest item immediately on interact. The existing SDL bitmap text HUD already displays player biology stats.

## Goals / Non-Goals

**Goals:**
- Add minimal carried counts for survival item categories.
- Keep the UI visible in the existing HUD without introducing new windows or menu state.
- Preserve NPC item consumption behavior.
- Reuse existing item effect semantics for player consumption.
- Keep full inventory management, dropping, trading, and provenance out of scope.

**Non-Goals:**
- No generic inventory grid.
- No item stacks beyond FOOD/WATER/MEDICAL counters.
- No drop/use targeting menu.
- No NPC inventory.
- No provenance tracking.

## Decisions

### D1: Dedicated survival inventory component

Use `SurvivalInventoryComponent` rather than overloading `ItemComponent` or `EconomicComponent`. The component carries only three integer counters and lives on the player for now. It is cheap, explicit, and easy to expand later.

### D2: Pickup collects; number keys consume

Interact near a survival item increments the matching counter and destroys the world item. Biological restoration is moved to explicit consumption via number keys. This makes the HUD meaningful and avoids accidentally wasting a pickup when stats are already high.

### D3: Reuse ConsumableSystem item effects

`ConsumableSystem` remains the owner of player-side item effect application. MEDICAL continues to reduce the worst active injury before restoring health.

### D4: HUD only, no modal UI

The current renderer already has stable top-left HUD text. Adding a single `INV F:x W:y M:z` line keeps the first implementation low-risk and always visible during play.

## Risks / Trade-offs

- Integer counters are not a general item model; later Phase 9 inventory management should replace or wrap this component.
- Number keys are simple but not remappable yet.
- Collection changes interaction feel because pickups no longer immediately restore biology.
