## Context

WageSystem (L3, 1 Hz) already drips `daily_wage * time_scale / 1440` credits into every NPC's `EconomicComponent.credits` each tick. Commercial buildings exist in world generation (COMMERCIAL zone type). `BiologyComponent` tracks hunger/thirst in [0,1] where 0 = starving. `ItemComponent` defines FOOD/WATER/MEDICAL item types whose effects are already handled by `ConsumableSystem`. No spending path exists — NPCs never lose credits.

## Goals / Non-Goals

**Goals:**
- Add `MarketComponent` to a subset of commercial buildings at world gen time
- Price = `base_price / clamp(stock / max_stock, 0.1, 2.0)` — scarce stock raises price, abundant stock lowers it
- NPCs at L3 tick: if hunger < 0.4 and credits ≥ price → buy FOOD; if thirst < 0.4 and credits ≥ price → buy WATER
- Stock depletes on purchase; restocks at a fixed rate per second (configurable per building)
- Apply item effect inline on purchase (same deltas as ConsumableSystem: hunger/thirst +0.5)
- Wire into existing L3 gate in `main.cpp`

**Non-Goals:**
- No supply chain (no extraction or production phase — flat restock rate only)
- No item provenance tracking
- No property ownership or rent
- No player UI for market interaction (NPC-only this phase)
- No inter-district price arbitrage

## Decisions

**1. MarketComponent lives on buildings, not a global market entity.**
Buildings already have spatial position and zone type. Attaching MarketComponent directly means MarketSystem can `view<PositionComponent, MarketComponent>()` and range-query buying NPCs nearby — no indirection needed.

**2. Stock stored as three floats (food_stock, water_stock, medical_stock), not a generic array.**
We have exactly three item categories in scope. A fixed struct is simpler and cache-friendlier than `std::map<ItemType, float>`. Extend to array later if SKUs multiply.

**3. NPC purchase is proximity-gated (≤ 60 WU from market building).**
Prevents unrealistic global purchases. Radius chosen to cover a city block. NPCs don't pathfind to markets yet — they buy opportunistically when nearby during normal wandering.

**4. Effect applied inline, not via item entity.**
Spawning an ItemEntity and immediately having the NPC pick it up adds two system steps with no behavioral benefit. The `ConsumableSystem` item-effect logic is short and can be duplicated as a free function `applyItemEffect(Registry&, Entity npc, ItemType)` shared by both paths.

**5. Restock is time-based (restock_rate units/sec), not event-based.**
Keeps MarketSystem self-contained at L3. A supply chain system can later override restock_rate via a directive without restructuring MarketSystem.

## Risks / Trade-offs

- **NPC clustering at markets**: All hungry NPCs will converge on the nearest market. Mitigation: proximity gate (≤ 60 WU) limits simultaneous buyers; stock depletion naturally disperses demand. Full pathfinding is a later concern.
- **Stock going negative**: Clamp stock ≥ 0 in transact(); if stock == 0, purchase is refused (price spikes prevent most attempts via the clamp floor at 0.1 ratio).
- **L3 tick rate (1 Hz) may miss fast market dynamics**: Acceptable — this is an ambient economy sim, not a real-time trading engine. Price updates at 1 Hz are imperceptible to the player.
- **Duplicate effect logic**: `applyItemEffect` duplicates ConsumableSystem deltas. Acceptable for now; a future refactor can consolidate.
