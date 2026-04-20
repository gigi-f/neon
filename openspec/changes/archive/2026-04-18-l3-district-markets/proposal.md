## Why

The WageSystem drips credits into NPC wallets, but there's nowhere to spend them — the economy is inflow-only. Without a spending sink, credits accumulate with no behavioral consequence, making `EconomicComponent` hollow. District markets close the loop: wages flow in (WageSystem), spending flows out (MarketSystem), and NPC survival needs drive real economic pressure.

## What Changes

- **New**: `MarketComponent` on commercial buildings — tracks stock levels (FOOD, WATER, MEDICAL), base price, and a restock timer
- **New**: `MarketSystem::update()` at L3 — NPC transaction logic (need-driven purchases), price elasticity from stock ratio, periodic restocking
- **New**: `MarketSystem::transact()` free function — deducts credits from buyer, reduces stock, delivers item effect (hunger/thirst restore or injury heal)
- **Modified**: `ConsumableSystem` — item effects now fire on market purchase, not just from ground pickup

## Capabilities

### New Capabilities
- `district-markets`: MarketComponent data model, stock tracking (FOOD/WATER/MEDICAL), price = base_price / stock_ratio clamped, NPC purchase logic gated on need + credits, per-building restock rate

### Modified Capabilities
<!-- No spec-level behavior changes to existing capabilities -->

## Impact

- `components.h` — MarketComponent struct
- `simulation_systems.h` — MarketSystem class, transact() free function
- `main.cpp` — instantiate MarketSystem, wire into L3 gate
- `world_generation.h` — assign MarketComponent to COMMERCIAL zone buildings at generation time
