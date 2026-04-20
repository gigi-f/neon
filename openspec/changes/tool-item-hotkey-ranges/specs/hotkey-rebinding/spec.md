## MODIFIED Requirements

### Requirement: Numeric hotkey rebinding
The player SHALL be able to assign the currently equipped slot to numeric hotkeys that match that slot's reserved range. Scan tools SHALL only bind to hotkeys `1` through `5`; item or consumable slots SHALL only bind to hotkeys `6` through `9`; hotkey `0` SHALL remain assigned to no slot.

#### Scenario: Rebind scan tool to tool range
- **WHEN** the player holds the rebinding modifier and presses a numeric hotkey from `1` through `5` while a scan tool is equipped
- **THEN** that numeric hotkey SHALL be assigned to the currently equipped scan tool

#### Scenario: Rebind consumable to item range
- **WHEN** the player holds the rebinding modifier and presses a numeric hotkey from `6` through `9` while a consumable item is equipped
- **THEN** that numeric hotkey SHALL be assigned to the currently equipped consumable item

#### Scenario: Reject scan tool outside tool range
- **WHEN** the player attempts to bind a scan tool to a numeric hotkey from `6` through `9`
- **THEN** the hotkey assignment SHALL remain unchanged

#### Scenario: Reject item outside item range
- **WHEN** the player attempts to bind a consumable item to a numeric hotkey from `1` through `5`
- **THEN** the hotkey assignment SHALL remain unchanged

#### Scenario: Empty hotkey remains fixed
- **WHEN** the player holds the rebinding modifier and presses hotkey `0`
- **THEN** hotkey `0` SHALL remain assigned to no slot
