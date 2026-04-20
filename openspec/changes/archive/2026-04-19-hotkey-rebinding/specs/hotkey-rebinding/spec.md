## ADDED Requirements

### Requirement: Numeric hotkey rebinding
The player SHALL be able to assign the currently equipped slot to numeric hotkeys `1` through `9`.

#### Scenario: Rebind numeric hotkey to equipped slot
- **WHEN** the player holds the rebinding modifier and presses a numeric hotkey
- **AND** an item or scan tool is currently equipped
- **THEN** that numeric hotkey SHALL be assigned to the currently equipped slot

#### Scenario: Rebound hotkey uses new assignment
- **WHEN** a numeric hotkey has been rebound
- **AND** the player presses that numeric hotkey without the rebinding modifier
- **THEN** the game SHALL equip and use the rebound slot

### Requirement: Empty hotkey remains fixed
Hotkey `0` SHALL remain assigned to no slot.

#### Scenario: Zero clears equipped slot
- **WHEN** the player presses hotkey `0`
- **THEN** the equipped slot SHALL become empty

#### Scenario: Rebinding modifier with zero keeps empty slot
- **WHEN** the player holds the rebinding modifier and presses hotkey `0`
- **THEN** hotkey `0` SHALL remain assigned to no slot
