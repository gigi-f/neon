## ADDED Requirements

### Requirement: MarketComponent data structure
The system SHALL define a `MarketComponent` struct in `components.h` with the following fields:
- `item_category` — `ItemComponent::Type` (FOOD or WATER) indicating what this market sells
- `base_price` — `float`, credits per unit, set at spawn time based on zone wealth
- `demand_pressure` — `float` in [0.0, 2.0], accumulates on purchase, decays each L3 tick
- `stock` — `int`, units currently available; restocks over time up to `max_stock`
- `max_stock` — `int`, capacity ceiling (default 50)

#### Scenario: Component default values
- **WHEN** a MarketComponent is default-constructed
- **THEN** base_price SHALL be 10.0f, demand_pressure SHALL be 0.0f, stock SHALL equal max_stock (50), item_category SHALL be FOOD

### Requirement: Market entity spawning in world generation
World generation SHALL assign `MarketComponent` to one building entity per commercial-zone block cluster. The spawned market's `item_category` SHALL alternate FOOD/WATER across clusters to ensure both types exist in each district.

#### Scenario: Commercial zone receives a market
- **WHEN** world generation processes a commercial-zone building cluster
- **THEN** exactly one building in that cluster SHALL receive a `MarketComponent`

#### Scenario: Both market types exist per district
- **WHEN** world generation completes for a district with two or more commercial clusters
- **THEN** at least one FOOD market and one WATER market SHALL exist in that district

### Requirement: Market glyph representation
Market buildings SHALL display a distinct glyph to distinguish them visually. A `GlyphComponent` with chars `"$"` and color RGB (255, 210, 50) (amber-gold) SHALL be assigned at spawn. This overrides any existing building glyph.

#### Scenario: Market glyph assigned at spawn
- **WHEN** a building entity receives a `MarketComponent` during world generation
- **THEN** its `GlyphComponent` SHALL be updated to chars `"$"`, r=255, g=210, b=50
