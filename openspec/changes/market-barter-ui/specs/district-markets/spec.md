## ADDED Requirements

### Requirement: Player market trade preview
The player SHALL be able to preview a one-unit trade with a nearby market before confirming it. The preview SHALL include buy/sell mode, item type, effective price or payout, player credits, relevant market stock, and an unavailable reason when the transaction cannot currently execute.

#### Scenario: Buy preview shows payable offer
- **WHEN** the player selects a FOOD buy offer while within trade range of a market with stock and sufficient credits
- **THEN** the preview SHALL show FOOD, BUY, the effective food price, current credits, and available food stock

#### Scenario: Buy preview shows denial reason
- **WHEN** the selected market has no WATER stock or the player cannot afford the WATER offer
- **THEN** the preview SHALL show the selected WATER offer as unavailable with a reason before confirmation

#### Scenario: Sell preview shows payout
- **WHEN** the selected inventory entry contains a sellable FOOD, WATER, or MEDICAL item near a market
- **THEN** the preview SHALL show SELL, the selected item type, and the payout amount before confirmation

---

### Requirement: Player market transaction execution
The player SHALL be able to confirm the currently previewed trade. Buy confirmation SHALL deduct credits, decrement market stock by one unit, and add one matching item to discrete inventory. Sell confirmation SHALL remove the selected sellable item, add credits to the player, and increase matching market stock by one unit clamped to max stock.

#### Scenario: Player buys food
- **WHEN** the player confirms an available FOOD buy offer with enough credits and an empty inventory slot
- **THEN** player credits SHALL decrease by the effective food price
- **AND** market food stock SHALL decrease by one unit
- **AND** the player's discrete inventory SHALL gain one FOOD item

#### Scenario: Player cannot buy without inventory space
- **WHEN** the player confirms a buy offer while every discrete inventory slot is occupied
- **THEN** no credits SHALL be deducted
- **AND** no market stock SHALL be consumed

#### Scenario: Player sells water
- **WHEN** the player confirms an available sell offer for a selected WATER inventory item
- **THEN** player credits SHALL increase by the sell payout
- **AND** the selected WATER inventory slot SHALL become empty
- **AND** market water stock SHALL increase by one unit without exceeding max stock
