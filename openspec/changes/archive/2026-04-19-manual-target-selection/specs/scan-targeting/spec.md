## ADDED Requirements

### Requirement: Manual scan target cursor
When a scan panel is active, the player SHALL be able to move the selected scan target to another eligible in-range entity using arrow keys.

#### Scenario: Arrow key selects entity in that direction
- **WHEN** the scan panel has a selected target and another eligible entity is positioned in the pressed arrow direction
- **THEN** the selected scan target SHALL move to that entity

#### Scenario: Cursor respects active scan range
- **WHEN** an entity is positioned in the pressed arrow direction but outside the active scan tool range from the player
- **THEN** the selected scan target SHALL remain on the current in-range target

#### Scenario: Empty cursor can recover from player origin
- **WHEN** the scan panel is active with no current valid selected target
- **THEN** pressing an arrow key SHALL select the best eligible in-range entity in that direction from the player position
