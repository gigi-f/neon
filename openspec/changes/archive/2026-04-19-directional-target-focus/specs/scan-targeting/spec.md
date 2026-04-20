## MODIFIED Requirements

### Requirement: Manual scan target cursor
When a scan tool is equipped or a scan panel is active, the player SHALL be able to move the selected scan target to another eligible in-range entity using arrow keys. The selected scan target SHALL remain locked to that entity while it is alive and within the active scan tool range, and SHALL clear when the target leaves range.

#### Scenario: Arrow key selects entity in that direction
- **WHEN** a scan tool is equipped or a scan panel is active
- **AND** an eligible in-range entity is positioned in the pressed arrow direction from the current focus or player origin
- **THEN** the selected scan target SHALL move to the eligible entity most aligned with that direction

#### Scenario: Cursor respects active scan range
- **WHEN** an entity is positioned in the pressed arrow direction but outside the active scan tool range from the player
- **THEN** that entity SHALL NOT become the selected scan target

#### Scenario: Empty cursor can recover from player origin
- **WHEN** a scan tool is equipped or a scan panel is active with no current valid selected target
- **THEN** pressing an arrow key SHALL select the best eligible in-range entity in that direction from the player position

#### Scenario: Only selected target clears
- **WHEN** exactly one eligible target is in range
- **AND** that target is already selected
- **THEN** pressing any arrow key SHALL clear the selected scan target

#### Scenario: Locked target leaves range
- **WHEN** the selected scan target moves outside the active scan tool range
- **THEN** the selected scan target SHALL clear
