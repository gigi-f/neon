## 1. Data Model (components.h)

- [x] 1.1 Add `MarketComponent` struct under the `// ── L3 Economy ──` banner with fields: `float food_stock`, `float water_stock`, `float medical_stock`, `float max_stock`, `float base_price`, `float restock_rate`; initialize to sensible defaults (`max_stock=20.f`, `base_price=5.f`, `restock_rate=0.5f`, stocks at `max_stock`)

## 2. applyItemEffect Free Function (simulation_systems.h)

- [x] 2.1 Add `inline void applyItemEffect(Registry& registry, Entity npc, ItemType type)` above `MarketSystem`; apply same deltas as `ConsumableSystem`: FOOD → `bio.hunger += 0.5f`, WATER → `bio.thirst += 0.5f`, MEDICAL → reduce worst injury slot severity by 0.5 (or fallback `bio.health += 0.2f`); clamp all fields to [0,1]

## 3. MarketSystem (simulation_systems.h)

- [x] 3.1 Declare `class MarketSystem` with `void update(Registry&, float dt, float time_scale)`
- [x] 3.2 Implement **restock pass**: iterate `view<MarketComponent>()`, add `slot.restock_rate * dt * time_scale` to each stock field, clamp to `max_stock`
- [x] 3.3 Implement **transaction pass**: iterate `view<PositionComponent, BiologyComponent, EconomicComponent>()`; for each NPC find nearest market within 60 WU using `view<PositionComponent, MarketComponent>()`; if hunger < 0.4 and food_stock > 0 and credits ≥ effective food price → call `doTransact()` for FOOD; same for thirst/WATER
- [x] 3.4 Implement `doTransact(Registry&, Entity npc, MarketComponent& market, ItemType type)`: compute effective price = `base_price / std::clamp(stock/max_stock, 0.1f, 2.0f)`, deduct credits, decrement stock by 1.f (clamp ≥ 0), call `applyItemEffect()`
- [x] 3.5 Guard: if stock == 0 for the chosen category, skip the transaction silently

## 4. World Generation Wiring (world_generation.h)

- [x] 4.1 After placing a COMMERCIAL zone building entity, attach a `MarketComponent` with default values

## 5. Main Loop Wiring (main.cpp)

- [x] 5.1 Instantiate `MarketSystem marketSystem` alongside other system instances
- [x] 5.2 In the L3 gate block: call `marketSystem.update(registry, dt, time_scale)` after `wageSystem.update()`

## 6. Verification

- [x] 6.1 Build with `make` — zero new warnings or errors
- [ ] 6.2 Confirm COMMERCIAL buildings have `MarketComponent` in a debug run (add a temporary `entity_count()` style log or inspect via debug HUD)
- [ ] 6.3 Run simulation for 60+ seconds; observe NPC credit balances decrease as they wander near markets
- [ ] 6.4 Deplete a market's food_stock by lowering `max_stock`/raising demand; confirm price rises and stock restocks over time
