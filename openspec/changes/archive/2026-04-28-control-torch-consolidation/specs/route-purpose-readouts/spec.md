## MODIFIED Requirements

### Requirement: Debugger route scan enrichment
The system SHALL expose richer route detail through `SPACE` Debugger inspection than ordinary inspection shows.

#### Scenario: Debugger inspects a path
- **WHEN** the Debugger inspects a pedestrian path
- **THEN** the result SHALL include route purpose, expected cargo, and access detail

#### Scenario: Debugger inspects a signpost
- **WHEN** the Debugger inspects a route signpost
- **THEN** the result SHALL include route purpose, expected cargo, access detail, and target role
