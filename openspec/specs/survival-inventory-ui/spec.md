# survival-inventory-ui Specification

## Purpose
Tracks the minimal player-facing survival inventory for common L1 consumables. FOOD, WATER, and MEDICAL remain simple counters with explicit consume controls until the broader inventory management system exists.

## Requirements
### Requirement: Survival inventory counters

The player SHALL carry a `SurvivalInventoryComponent` with integer counters for FOOD, WATER, and MEDICAL survival items.

#### Scenario: Player starts with empty survival inventory
- **WHEN** the player entity is created
- **THEN** FOOD, WATER, and MEDICAL counts SHALL default to 0

### Requirement: Player collection stores survival items

Player interact pickup SHALL collect the nearest FOOD, WATER, or MEDICAL item within 30 WU into the matching survival inventory counter, destroy the item entity, and suppress lower-priority interactions for that keypress.

#### Scenario: Collect nearby food
- **WHEN** the player presses interact while a FOOD item is within 30 WU
- **THEN** `SurvivalInventoryComponent.food_count` SHALL increase by 1
- **AND** the FOOD item entity SHALL be destroyed
- **AND** player hunger SHALL NOT change until FOOD is consumed from inventory

### Requirement: Number keys consume survival inventory

The player SHALL be able to consume survival inventory slots using number keys: 1 consumes FOOD, 2 consumes WATER, and 3 consumes MEDICAL.

#### Scenario: Consume carried water
- **WHEN** the player has `water_count > 0` and presses 2
- **THEN** `water_count` SHALL decrease by 1
- **AND** player thirst SHALL increase by the standard survival item restore amount, clamped to 100

#### Scenario: Empty slot has no effect
- **WHEN** the player has `food_count == 0` and presses 1
- **THEN** player hunger SHALL remain unchanged

### Requirement: Survival inventory HUD

The on-screen HUD SHALL render FOOD, WATER, and MEDICAL survival inventory counts while the player has `SurvivalInventoryComponent`.

#### Scenario: Counts are visible
- **WHEN** the player has FOOD=2, WATER=1, and MEDICAL=0
- **THEN** the HUD SHALL include those three counts in a stable top-left text line
