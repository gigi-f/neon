## Context

WageSystem deposits credits into `EconomicComponent::credits` every L3 tick, but nothing spends them. Citizens seeking hunger/thirst satisfaction currently search for free `ItemComponent` pickups scattered on the map — a placeholder loop with no economic feedback. This design introduces `MarketComponent` buildings and a `MarketSystem` that closes the L3 loop: credits → purchase → biology restore → price signal.

Existing anchors:
- `EconomicComponent` (credits, employer, daily_wage) on every citizen
- `ItemComponent` (FOOD/WATER/MEDICAL, restore_value) for pickup items — stays as fallback
- `GDISystem` selects goals via `DesireType`; hunger/thirst branch searches ItemComponents
- `ConsumableSystem` fires when a citizen reaches an ItemComponent entity and consumes it
- L3 tick rate: 1 Hz via SimulationCoordinator

## Goals / Non-Goals

**Goals:**
- Market entities spawned at world-gen time in commercial zones (one per ~4-block cluster)
- Citizens with sufficient credits prefer market pathfinding over free item scavenging
- Transaction on arrival: deduct credits, restore hunger/thirst, adjust market demand pressure
- Price dynamics: demand pressure increases price; restock reduces it over time
- MarketSystem runs at L3 (1 Hz), price changes are gradual (not per-frame spikes)

**Non-Goals:**
- Player-facing shop UI (Phase 9 Trade & Barter)
- Supply chain / provenance tracking (next L3 milestone)
- Multi-category inventory per market (each market sells one primary category for now)
- Market bankruptcy or closure mechanics

## Decisions

### 1. MarketComponent lives on building entities, not new market-only entities

**Rationale:** World generation already spawns building entities with `TransformComponent` and zone type. Adding `MarketComponent` to a subset of those buildings avoids a separate entity population and reuses existing rendering. The GDI market-search iterates `view<MarketComponent, TransformComponent>()`, same pattern as station search.

**Alternative considered:** Dedicated market entities (no building attachment). Rejected — doubles entity count in commercial zones and requires separate spawning logic.

### 2. Extend DesireType with BUY_FOOD and BUY_WATER

**Rationale:** The GDISystem desire-scoring branch is the single place that assigns goals. Adding two market desires keeps the architecture consistent: each desire maps cleanly to one goal target type. The scoring function checks `credits >= item_price` before preferring BUY over SATISFY — broke citizens fall back to free items naturally.

**Alternative considered:** A single `SHOP` desire with category resolution at commit time. Rejected — GDI commitment tracking is per-desire; merging them would require storing the sub-category on IntentionComponent, adding coupling.

### 3. MarketSystem handles transactions, not ConsumableSystem

**Rationale:** ConsumableSystem fires on `ItemComponent` proximity — market buildings are not items. A separate `MarketSystem` iterates `view<CitizenComponent, GoalComponent, EconomicComponent, BiologyComponent>()`, checks if the goal target has a `MarketComponent`, and if the citizen is within arrival radius, executes the transaction. This keeps ConsumableSystem unchanged and market logic co-located.

### 4. Demand pressure as a single float per market, not per-citizen tracking

**Rationale:** Full per-citizen demand tracking is O(N×M) storage with no gameplay value at this simulation scale. A market's `demand_pressure` accumulates +0.1 per purchase and decays toward 0 at 0.05/tick (L3). Price = `base_price * (1.0 + demand_pressure)`. Simple, cache-friendly, emergent.

### 5. World-gen spawns one MarketComponent per commercial building cluster

**Rationale:** World generation already groups buildings by zone type. Assigning `MarketComponent` to the first commercial-zone building in each cluster (by existing building ID ordering) requires no new spatial data structures. Market density naturally mirrors commercial zone density.

## Risks / Trade-offs

- **All citizens converge on one market** → Mitigation: demand pressure raises price, making distant cheaper markets more attractive via GDI `bestDist` scoring (distance + price factor).
- **Unemployed citizens can never eat if free items run out** → Mitigation: Free `ItemComponent` scavenging path is preserved as fallback; GDISystem only prefers market when `credits >= price`.
- **MarketSystem runs at L3 (1 Hz) but CitizenAISystem runs at L0 (60 Hz)** → Accepted: transaction check in MarketSystem is the authoritative event; CitizenAISystem only moves the citizen, it doesn't consume.
- **Price can drift to 0 if no demand** → Mitigation: price is clamped to `[base_price * 0.5, base_price * 3.0]`.
