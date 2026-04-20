## ADDED Requirements

### Requirement: Demand pressure decay
Each L3 tick `MarketSystem::update()` SHALL decay every market's `demand_pressure` by 0.05f toward 0.0f. Pressure SHALL NOT go below 0.0f.

#### Scenario: Idle market pressure decays
- **WHEN** no purchases occur at a market for one L3 tick
- **THEN** demand_pressure SHALL decrease by 0.05f (clamped to minimum 0.0f)

### Requirement: Current price derivation
The effective price for a single unit SHALL be computed as:
`current_price = base_price * (1.0f + demand_pressure)`
This value SHALL be clamped to `[base_price * 0.5f, base_price * 3.0f]`.

#### Scenario: Price at zero demand pressure equals base price
- **WHEN** demand_pressure == 0.0f
- **THEN** current_price SHALL equal base_price

#### Scenario: Price rises with demand
- **WHEN** demand_pressure == 1.0f
- **THEN** current_price SHALL equal base_price * 2.0f

#### Scenario: Price ceiling enforced
- **WHEN** demand_pressure would yield current_price > base_price * 3.0f
- **THEN** current_price SHALL be clamped to base_price * 3.0f

### Requirement: Periodic restock
Each L3 tick, if `market.stock < market.max_stock`, stock SHALL increment by 1. This produces a gradual restock rate of 1 unit/second at L3 frequency.

#### Scenario: Partially depleted market restocks
- **WHEN** market.stock < market.max_stock at the start of an L3 tick
- **THEN** market.stock SHALL increase by 1 after the tick

#### Scenario: Full market does not over-restock
- **WHEN** market.stock == market.max_stock
- **THEN** market.stock SHALL remain unchanged

### Requirement: GDI market preference scoring
In `GDISystem`, when scoring `BUY_FOOD` or `BUY_WATER` desires, the candidate target search SHALL prefer the nearest market weighted by affordability. A market the citizen cannot afford (credits < current_price) SHALL be skipped entirely, not assigned as a goal.

#### Scenario: Affordable market selected as goal
- **WHEN** citizen has credits >= current_price at nearest market with matching category
- **THEN** GDISystem SHALL assign that market entity as the GoalComponent target

#### Scenario: Unaffordable market skipped
- **WHEN** citizen has credits < current_price at ALL markets of the needed category
- **THEN** GDISystem SHALL fall back to searching free ItemComponent pickups for SATISFY_HUNGER/SATISFY_THIRST
