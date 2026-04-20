## ADDED Requirements

### Requirement: MarketComponent data model
Commercial buildings MAY carry a `MarketComponent` that tracks stock levels for three item categories (FOOD, WATER, MEDICAL), a base price per unit, a maximum stock capacity, and a restock rate in units per real second.

#### Scenario: Component initialized at world gen
- **WHEN** a COMMERCIAL zone building is created during world generation
- **THEN** it SHALL receive a `MarketComponent` with non-zero `food_stock`, `water_stock`, `medical_stock` at or near `max_stock`

#### Scenario: Component absent on non-commercial buildings
- **WHEN** a building belongs to a non-COMMERCIAL zone type
- **THEN** it SHALL NOT receive a `MarketComponent`

---

### Requirement: Price elasticity
The effective price for a stock category SHALL be `base_price / clamp(stock / max_stock, 0.1f, 2.0f)`.

#### Scenario: Low stock raises price
- **WHEN** `stock` is less than 10% of `max_stock`
- **THEN** effective price SHALL be at least `base_price / 0.1f` (10× base)

#### Scenario: Abundant stock lowers price
- **WHEN** `stock` equals or exceeds `max_stock`
- **THEN** effective price SHALL be `base_price / 1.0f` or lower

#### Scenario: Stock at zero blocks purchase
- **WHEN** `stock == 0` for a category
- **THEN** the MarketSystem SHALL NOT allow a purchase of that category

---

### Requirement: NPC need-driven purchase
The MarketSystem SHALL, at each L3 tick, allow an NPC within ≤ 60 world units of a market building to purchase one item if:
1. The NPC has a `BiologyComponent` with `hunger < 0.4f` (for FOOD) or `thirst < 0.4f` (for WATER)
2. The NPC's `EconomicComponent.credits` ≥ effective price
3. The market building has stock > 0 for that category

#### Scenario: Hungry NPC with sufficient credits buys food
- **WHEN** an NPC has `hunger < 0.4f`, `credits >= effective_food_price`, is within 60 WU of a market, and food_stock > 0
- **THEN** `credits` SHALL decrease by `effective_food_price` and `hunger` SHALL increase by `0.5f` (clamped to 1.0f) and `food_stock` SHALL decrease by 1.0f

#### Scenario: Thirsty NPC with insufficient credits does not buy
- **WHEN** an NPC has `thirst < 0.4f` but `credits < effective_water_price`
- **THEN** no transaction SHALL occur

#### Scenario: NPC too far from market does not buy
- **WHEN** an NPC's need threshold is met and credits are sufficient, but the nearest market is > 60 WU away
- **THEN** no transaction SHALL occur this tick

---

### Requirement: Stock restock over time
Each market building's stock categories SHALL increase by `restock_rate * dt * time_scale` per real second, clamped to `max_stock`.

#### Scenario: Stock recovers after depletion
- **WHEN** `food_stock` reaches 0
- **THEN** after sufficient ticks, `food_stock` SHALL recover toward `max_stock` at rate `restock_rate`

#### Scenario: Stock does not exceed max
- **WHEN** stock is at `max_stock` and restock fires
- **THEN** stock SHALL remain at `max_stock` (no overflow)
