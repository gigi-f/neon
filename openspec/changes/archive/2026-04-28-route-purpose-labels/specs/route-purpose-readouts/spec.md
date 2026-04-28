## ADDED Requirements

### Requirement: Derived path route purpose
The system SHALL derive readable route purpose labels from pedestrian path endpoint roles.

#### Scenario: Housing to workplace path
- **WHEN** a pedestrian path connects HOUSING and WORKPLACE
- **THEN** the path readout SHALL include `LABOR ROUTE`

#### Scenario: Workplace to supply path
- **WHEN** a pedestrian path connects WORKPLACE and SUPPLY
- **THEN** the path readout SHALL include `SUPPLY ROUTE`

### Requirement: Signpost route purpose readout
The system SHALL include route purpose and carried-flow text in ordinary signpost inspection readouts.

#### Scenario: Normal signpost
- **WHEN** a route signpost points to an endpoint on a known route
- **THEN** its readout SHALL include destination, purpose, carried flow, signal state, and consequence

#### Scenario: Spoofed signpost
- **WHEN** a route signpost is spoofed
- **THEN** its readout SHALL keep the destination, purpose, and carried-flow text readable while showing the existing corrupted signal consequence

### Requirement: Debugger route scan enrichment
The system SHALL expose richer route detail through the inherited debugger scan than ordinary inspection shows.

#### Scenario: Debugger scans a path
- **WHEN** the debugger scans a pedestrian path
- **THEN** the result SHALL include route purpose, expected cargo, and access detail

#### Scenario: Debugger scans a signpost
- **WHEN** the debugger scans a route signpost
- **THEN** the result SHALL include route purpose, expected cargo, access detail, and target role
