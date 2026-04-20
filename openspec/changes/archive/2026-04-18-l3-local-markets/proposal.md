## Why

L3 has a wage loop (WageSystem credits citizens) but nowhere to spend credits — citizens just accumulate wealth with no feedback. Adding district markets closes the L3 economic loop: wages flow in, purchases flow out, and supply/demand pressure creates emergent price signals.

## What Changes

- Add `MarketComponent` to building entities: tracks per-category stock, base price, and a demand pressure float.
- Add `BUY_FOOD` and `BUY_WATER` desire variants to `DesireType`; GDISystem prefers market pathfinding when a citizen has sufficient credits.
- New `MarketSystem` (L3, 1 Hz): handles purchase transactions when citizen arrives at a market, adjusts prices via supply/demand, and restocks over time.
- Market entities spawned at world-gen time in commercial/mixed-use zones (one per district block cluster).
- Free `ItemComponent` pickups remain as fallback for broke/unemployed citizens.

## Capabilities

### New Capabilities
- `market-entity`: MarketComponent data structure, market spawning in world generation, glyph representation
- `market-transaction`: GDI desire scoring for BUY_FOOD/BUY_WATER, credit deduction + hunger/thirst restore on arrival
- `market-price-dynamics`: MarketSystem supply/demand pressure, price adjustment, periodic restock

### Modified Capabilities
- `economic-component`: DesireType enum gains BUY_FOOD and BUY_WATER variants; GDISystem item-search branch extended to consult markets

## Impact

- `src/components.h` — new `MarketComponent`; extend `DesireType`
- `src/simulation_systems.h` — `GDISystem::update()` new branch; new `MarketSystem` class
- `src/world_generation.h` — spawn market entities in commercial zones
- `src/main.cpp` — register `MarketSystem` in L3 coordinator slot
