# simulation-alerts Specification

## Purpose
TBD - created by archiving change existing-system-alerts. Update Purpose after archive.
## Requirements
### Requirement: Bounded Alert Stack
The simulation SHALL maintain a bounded recent-alert stack for player-facing event notifications.

#### Scenario: Old alerts are evicted
- **WHEN** more alerts are pushed than the configured stack capacity
- **THEN** the oldest alerts SHALL be removed first.

#### Scenario: Alerts expire from the visible HUD stack
- **WHEN** an alert's display time-to-live reaches zero
- **THEN** it SHALL no longer be returned as a visible alert.

### Requirement: Weather Change Alert
The simulation SHALL emit an alert when the global weather state changes.

#### Scenario: Weather transitions to acid rain
- **WHEN** `TimeOfDayComponent.weather` changes from another state to `ACID_RAIN`
- **THEN** an alert SHALL be added with at least warning severity and weather category.

### Requirement: Flood Toggle Alert
The simulation SHALL emit an alert when the flooding hazard toggles.

#### Scenario: Flooding activates
- **WHEN** `TimeOfDayComponent.is_flooded` changes from false to true
- **THEN** an alert SHALL be added with flood category.

#### Scenario: Flooding clears
- **WHEN** `TimeOfDayComponent.is_flooded` changes from true to false
- **THEN** an alert SHALL be added with flood category.

### Requirement: Infection Worsening Alert
The pathogen system SHALL emit an alert when an active infection crosses a higher severity tier.

#### Scenario: Infection crosses symptomatic threshold
- **WHEN** a `PathogenComponent` severity rises from below 0.35 to at least 0.35
- **THEN** an infection alert SHALL be added once for that component at that tier.

### Requirement: Collapse Alert
The structural decay system SHALL emit an alert when a building collapses.

#### Scenario: Structure reaches zero integrity
- **WHEN** a `StructuralComponent` reaches zero integrity and receives `CollapsedComponent`
- **THEN** a danger alert SHALL be added with structure category.

### Requirement: HUD Alert Rendering
The HUD SHALL render the short visible alert stack without blocking existing debug, survival, scan, or layer overlay displays.

#### Scenario: Multiple recent alerts exist
- **WHEN** two or more non-expired alerts exist
- **THEN** the HUD SHALL render the newest visible alerts in a stable fixed area below the survival HUD.

