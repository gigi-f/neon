## 1. Components

- [x] 1.1 Add `MarketComponent` struct to `components.h` with fields: `item_category` (ItemComponent::Type), `base_price` (float, default 10.0f), `demand_pressure` (float, default 0.0f), `stock` (int, default 50), `max_stock` (int, default 50)
- [x] 1.2 Extend `DesireType` enum in `components.h` to add `BUY_FOOD` and `BUY_WATER` after existing values

## 2. World Generation

- [x] 2.1 In `world_generation.h`, identify commercial-zone building clusters during world gen
- [x] 2.2 Assign `MarketComponent` to one building per commercial cluster; alternate item_category FOOD/WATER across clusters
- [x] 2.3 Override that building's `GlyphComponent` to chars `"$"`, r=255, g=210, b=50 when MarketComponent is assigned

## 3. GDI System — Market Desire Scoring

- [x] 3.1 In `GDISystem::update()`, before scoring SATISFY_HUNGER, search for nearest affordable FOOD market (credits >= current_price); if found, score BUY_FOOD = hunger_urgency + 0.1f
- [x] 3.2 Similarly score BUY_WATER = thirst_urgency + 0.1f when affordable WATER market exists
- [x] 3.3 In the desire→goal translation block, handle `BUY_FOOD` and `BUY_WATER` by targeting the nearest affordable market entity of the matching category (same distance loop as station search)
- [x] 3.4 Ensure SATISFY_HUNGER / SATISFY_THIRST remain as fallback when no affordable market is found

## 4. Market System

- [x] 4.1 Create `MarketSystem` class in `simulation_systems.h` (L3, 1 Hz)
- [x] 4.2 In `MarketSystem::update()`, iterate all markets: decay `demand_pressure` by 0.05f (min 0.0f); restock by 1 if stock < max_stock
- [x] 4.3 Compute `current_price = base_price * (1.0f + demand_pressure)`, clamp to [base_price * 0.5f, base_price * 3.0f]
- [x] 4.4 Iterate citizens with GoalComponent targeting a MarketComponent entity; check arrival radius (25 WU)
- [x] 4.5 On arrival: verify stock > 0 and credits >= current_price and relevant bio stat < 90.0f
- [x] 4.6 On successful transaction: deduct credits, restore hunger/thirst by 40.0f (cap 100), decrement stock, add 0.1f demand_pressure
- [x] 4.7 On successful transaction: clear GoalComponent (target = MAX_ENTITIES) and set IntentionComponent::commitment = 0.0f
- [x] 4.8 On failed transaction (out of stock): clear GoalComponent so GDI re-searches next tick

## 5. Wiring

- [x] 5.1 In `main.cpp`, instantiate `MarketSystem` and call `market_system.update(registry, dt)` inside the L3 coordinator slot (alongside WageSystem)
- [x] 5.2 Register `MarketComponent` with the registry (ensure it is accessible via `registry.view<MarketComponent, TransformComponent>()`)
