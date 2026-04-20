## ADDED Requirements

### Requirement: Scan target marker state
The world marker for a selected scan target SHALL distinguish pre-fire focus from an active scan panel target.

#### Scenario: Pre-fire focus marker
- **WHEN** a scan target is selected before a scan panel is active
- **THEN** the world marker SHALL use a distinct focus-only style

#### Scenario: Active scan marker
- **WHEN** a scan panel is active for the selected scan target
- **THEN** the world marker SHALL use the active scan target style
