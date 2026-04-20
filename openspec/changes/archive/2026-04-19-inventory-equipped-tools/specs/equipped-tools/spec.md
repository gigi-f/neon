## MODIFIED Requirements

### Requirement: Equipped player tool slot
The player SHALL have a current equipped slot that can represent no item, a survival consumable, or a carried scan tool. Numeric hotkey assignments `1` through `9` SHALL be configurable at runtime and SHALL retain default assignments until rebound. Numeric hotkey `0` SHALL remain assigned to no slot. Scan tool hotkeys SHALL select their configured slot only when the player carries the matching inventory tool, and scan tools SHALL fire only from the action key after being equipped.

#### Scenario: Numeric hotkey selects a slot
- **WHEN** the player presses a configured numeric hotkey for a carried item or tool
- **THEN** the equipped slot SHALL change to that hotkey's configured item or tool

#### Scenario: Zero clears equipped tool
- **WHEN** the player presses hotkey `0`
- **THEN** the equipped slot SHALL become empty

#### Scenario: Rebound numeric hotkey selects rebound slot
- **WHEN** the player presses a numeric hotkey after rebinding it
- **AND** the rebound item or tool is carried when inventory ownership is required
- **THEN** the equipped slot SHALL change to the rebound item or tool

### Requirement: Scan tools expose range
Each scan tool SHALL have an explicit range used when selecting scan targets, using a scan tool SHALL require the matching carried inventory tool, and that range SHALL be visible as a world-space bounding box while the scan tool is equipped.

#### Scenario: Equipped scan uses tool range
- **WHEN** the player presses the action key with an equipped scan tool that exists in their inventory
- **THEN** scan target selection SHALL use that tool's configured range

#### Scenario: HUD shows equipped range
- **WHEN** the player has any slot equipped
- **THEN** the HUD SHALL show the equipped slot name and its current range

#### Scenario: Equipped scan shows range box
- **WHEN** the player has a carried scan tool equipped
- **THEN** the world view SHALL show the scan tool's range bounding box

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
