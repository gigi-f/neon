## ADDED Requirements

### Requirement: District-local interference persistence
The system SHALL persist active district-local interference state without leaking it to another district.

#### Scenario: Spoof in one district
- **WHEN** a signpost is spoofed in district A
- **THEN** district A route readouts SHALL show blocked flow
- **AND** district B route readouts SHALL remain clear

#### Scenario: Dependency disruption in one district
- **WHEN** the workplace/supply dependency is disrupted in district A
- **THEN** district A workplace, supply, dependency scan, and worker flow readouts SHALL show the disruption
- **AND** district B dependency readouts SHALL remain clear

### Requirement: District return restores saved interference
The system SHALL restore district-local interference when the player saves in another district and later returns.

#### Scenario: Save in district B with active district A interference
- **WHEN** district A has an active spoofed route, active dependency disruption, `LAID_LOW` witness trace, and wage spoof
- **AND** the player saves while currently in district B
- **THEN** loading SHALL keep the player in district B
- **AND** district B SHALL remain clear of district A interference
- **AND** riding transit back to district A SHALL reveal the original district A spoof, dependency disruption, witness trace, and wage spoof

### Requirement: AI signpost targeting stays district-local
The AI playtest surface SHALL target signposts in the player's current district when multiple districts exist.

#### Scenario: Warp after transit
- **WHEN** the AI playtest player rides transit from district A to district B
- **AND** uses `warp SIGNPOST`
- **THEN** the selected signpost SHALL belong to district B
- **AND** the snapshot SHALL expose a district B route tag
