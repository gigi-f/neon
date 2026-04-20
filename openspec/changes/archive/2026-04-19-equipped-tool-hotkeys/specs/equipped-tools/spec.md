## ADDED Requirements

### Requirement: Equipped player tool slot
The player SHALL have a current equipped slot that can represent no item, a survival consumable, or a scan tool.

#### Scenario: Numeric hotkey selects a slot
- **WHEN** the player presses a configured numeric hotkey
- **THEN** the equipped slot SHALL change to that hotkey's configured item or tool

#### Scenario: Zero clears equipped tool
- **WHEN** the player presses hotkey `0`
- **THEN** the equipped slot SHALL become empty

### Requirement: Scan tools expose range
Each scan tool SHALL have an explicit range used when selecting scan targets.

#### Scenario: Equipped scan uses tool range
- **WHEN** the player uses an equipped scan tool
- **THEN** scan target selection SHALL use that tool's configured range

#### Scenario: HUD shows equipped range
- **WHEN** the player has any slot equipped
- **THEN** the HUD SHALL show the equipped slot name and its current range

### Requirement: Survival consumables remain quick usable
Food, water, and medical inventory slots SHALL remain directly usable through their hotkeys while also updating the equipped slot.

#### Scenario: Consumable hotkey consumes and equips
- **WHEN** the player presses the food, water, or medical hotkey
- **THEN** the matching inventory counter SHALL be consumed when available
- **AND** the matching slot SHALL become the current equipped slot

#### Scenario: Equipped consumable can be reused
- **WHEN** a survival consumable is equipped
- **AND** the player uses the equipped slot
- **THEN** the matching inventory counter SHALL be consumed when available
