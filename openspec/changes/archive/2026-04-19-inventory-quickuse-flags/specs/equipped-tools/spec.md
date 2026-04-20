## MODIFIED Requirements

### Requirement: Survival consumables remain quick usable
Food, water, and medical inventory slots SHALL remain directly usable through their hotkeys while also updating the equipped slot. When a matching discrete carried item exists, quick-use SHALL consume that item before falling back to legacy survival counters.

#### Scenario: Consumable hotkey consumes and equips
- **WHEN** the player presses the food, water, or medical hotkey
- **THEN** the matching inventory item or counter SHALL be consumed when available
- **AND** the matching slot SHALL become the current equipped slot

#### Scenario: Equipped consumable can be reused
- **WHEN** a survival consumable is equipped
- **AND** the player uses the equipped slot
- **THEN** the matching inventory item or counter SHALL be consumed when available
