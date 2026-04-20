## MODIFIED Requirements

### Requirement: Price elasticity
The effective market base price for a stock category SHALL be `base_price / clamp(stock / max_stock, 0.1f, 2.0f)`. Player-facing trade prices SHALL further apply market greed and player market reputation modifiers before preview and confirmation.

#### Scenario: Low stock raises price
- **WHEN** `stock` is less than 10% of `max_stock`
- **THEN** effective base price SHALL be at least `base_price / 0.1f` (10x base)

#### Scenario: Abundant stock lowers price
- **WHEN** `stock` equals or exceeds `max_stock`
- **THEN** effective base price SHALL be `base_price / 1.0f` or lower

#### Scenario: Stock at zero blocks purchase
- **WHEN** `stock == 0` for a category
- **THEN** the MarketSystem SHALL NOT allow a purchase of that category

#### Scenario: Greedy market raises player buy price
- **WHEN** the player previews a buy offer at a market with a positive greed margin
- **THEN** the displayed buy price SHALL be higher than the scarcity-only base price before confirmation

#### Scenario: Good reputation improves trade terms
- **WHEN** the player has positive market reputation and previews a trade
- **THEN** buy prices SHALL be discounted and sell payouts SHALL improve relative to neutral reputation

#### Scenario: Confirmation uses current adjusted price
- **WHEN** the player confirms a previewed trade after market stock or player reputation changes
- **THEN** confirmation SHALL recompute the adjusted price and only execute if the current adjusted trade remains available
