## MODIFIED Requirements

### Requirement: Equipped player tool slot
The player SHALL have a current equipped slot that can represent no item, a survival consumable, or a scan tool. Numeric hotkey assignments `1` through `9` SHALL be configurable at runtime and SHALL retain default assignments until rebound. Numeric hotkey `0` SHALL remain assigned to no slot.

#### Scenario: Numeric hotkey selects a slot
- **WHEN** the player presses a configured numeric hotkey
- **THEN** the equipped slot SHALL change to that hotkey's configured item or tool

#### Scenario: Zero clears equipped tool
- **WHEN** the player presses hotkey `0`
- **THEN** the equipped slot SHALL become empty

#### Scenario: Rebound numeric hotkey selects rebound slot
- **WHEN** the player presses a numeric hotkey after rebinding it
- **THEN** the equipped slot SHALL change to the rebound item or tool
