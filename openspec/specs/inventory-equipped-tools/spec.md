# inventory-equipped-tools Specification

## Purpose
TBD - created by archiving change inventory-equipped-tools. Update Purpose after archive.
## Requirements
### Requirement: Inventory can carry scan tools
The player inventory SHALL be able to contain scan tool entries in addition to consumable entries.

#### Scenario: Player starts with baseline scan tools
- **WHEN** a new player entity is initialized
- **THEN** the player's discrete inventory SHALL include entries for each baseline scan tool

#### Scenario: Selected scan tool is equipped
- **WHEN** the selected inventory entry contains a scan tool
- **THEN** using that inventory entry SHALL set the player's equipped slot to the matching scan tool
- **AND** the inventory entry SHALL remain occupied

#### Scenario: Scan shortcut only equips
- **WHEN** the player presses a scan tool shortcut
- **THEN** the matching carried scan tool SHALL become equipped
- **AND** the scan tool SHALL NOT fire until the player presses the action key

### Requirement: Scan use is inventory-owned
Equipped scan tools SHALL only be usable while the player carries the matching scan tool inventory entry.

#### Scenario: Carried scan tool can be used
- **WHEN** the player presses the action key with an equipped scan tool that exists in their inventory
- **THEN** the matching scan panel SHALL open using that tool's configured range

#### Scenario: Missing scan tool is denied
- **WHEN** the player presses the action key with an equipped scan tool that is not present in their inventory
- **THEN** no scan panel SHALL open
- **AND** denied feedback SHALL be emitted

#### Scenario: Equipped scan range is visible before firing
- **WHEN** the player has a carried scan tool equipped
- **THEN** the renderer SHALL show a world-space bounding box for that tool's range before the action key is pressed

### Requirement: Arrow keys control targeting only
Arrow keys SHALL control cursor or focus movement and SHALL NOT move the player character or a driven vehicle.

#### Scenario: Arrow key does not move player
- **WHEN** the player presses an arrow key
- **THEN** the player's movement velocity SHALL NOT be changed by that key

#### Scenario: WASD moves player
- **WHEN** the player presses a WASD movement key
- **THEN** the player's movement velocity SHALL update in the matching direction

