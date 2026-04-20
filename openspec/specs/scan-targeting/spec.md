# scan-targeting Specification

## Purpose
Defines how player scan panels select an entity target, keeping inspection aligned with player-facing direction while preserving nearest-target fallback behavior.
## Requirements
### Requirement: Facing-aware scan target selection
Scan panel target selection SHALL prefer eligible entities in the player's current facing direction before considering eligible entities outside that direction.

#### Scenario: Forward entity beats closer side entity
- **WHEN** the player scans with one eligible entity in the facing direction and another closer eligible entity outside the facing direction
- **THEN** the scan target SHALL be the entity in the facing direction

#### Scenario: Nearest target remains fallback
- **WHEN** the player scans and no eligible entity is in the facing direction
- **THEN** the scan target SHALL be the nearest eligible entity within scan range

#### Scenario: All scan panels use the same target rule
- **WHEN** the player opens Surface Scan, Biological Audit, Cognitive Profile, Financial Forensics, or Structural Analysis
- **THEN** the selected target SHALL be chosen by the facing-aware scan target selection rule

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

### Requirement: Scan target marker state
The world marker for a selected scan target SHALL distinguish pre-fire focus from an active scan panel target.

#### Scenario: Pre-fire focus marker
- **WHEN** a scan target is selected before a scan panel is active
- **THEN** the world marker SHALL use a distinct focus-only style

#### Scenario: Active scan marker
- **WHEN** a scan panel is active for the selected scan target
- **THEN** the world marker SHALL use the active scan target style

