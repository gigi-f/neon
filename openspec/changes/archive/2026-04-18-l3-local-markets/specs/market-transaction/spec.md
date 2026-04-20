## ADDED Requirements

### Requirement: Arrival detection and transaction execution
`MarketSystem::update()` SHALL iterate all citizens that have a `GoalComponent` targeting an entity with `MarketComponent`. When the citizen is within 25 WU of the market's `TransformComponent` origin, a transaction SHALL be attempted.

#### Scenario: Citizen within arrival radius triggers transaction
- **WHEN** a citizen with active market goal is within 25 WU of the market entity
- **THEN** MarketSystem SHALL attempt a transaction this L3 tick

#### Scenario: Citizen still travelling does not trigger transaction
- **WHEN** a citizen with active market goal is more than 25 WU from the market
- **THEN** no transaction SHALL occur and the goal persists

### Requirement: Transaction success conditions
A transaction SHALL succeed when ALL of the following hold:
1. `market.stock > 0`
2. `citizen.credits >= current_price` (where `current_price = base_price * (1.0 + demand_pressure)`)
3. The citizen's relevant biology stat (hunger for FOOD market, thirst for WATER market) is below 90.0f

#### Scenario: Successful food purchase
- **WHEN** citizen has hunger < 90, credits >= market price, market has stock > 0, and market sells FOOD
- **THEN** credits SHALL be decremented by current_price, hunger SHALL increase by 40.0f (capped at 100), market stock SHALL decrement by 1, market demand_pressure SHALL increase by 0.1f

#### Scenario: Successful water purchase
- **WHEN** citizen has thirst < 90, credits >= market price, market has stock > 0, and market sells WATER
- **THEN** credits SHALL be decremented by current_price, thirst SHALL increase by 40.0f (capped at 100), market stock SHALL decrement by 1, market demand_pressure SHALL increase by 0.1f

#### Scenario: Transaction fails — insufficient credits
- **WHEN** citizen's credits < current_price
- **THEN** no transaction occurs; GDISystem SHALL NOT re-assign this citizen to a market on the next tick (commitment reset to 1.0f so GDI re-evaluates)

#### Scenario: Transaction fails — market out of stock
- **WHEN** market.stock == 0
- **THEN** no transaction occurs; citizen goal is cleared so GDISystem searches for next-nearest market

### Requirement: Goal cleared after successful transaction
After a successful transaction the citizen's `GoalComponent::target_entity` SHALL be set to MAX_ENTITIES and `IntentionComponent::commitment` SHALL be set to 0.0f so GDISystem re-evaluates on the next L2 tick.

#### Scenario: Post-purchase goal reset
- **WHEN** a transaction completes successfully
- **THEN** GoalComponent target SHALL be MAX_ENTITIES and IntentionComponent commitment SHALL be 0.0f
