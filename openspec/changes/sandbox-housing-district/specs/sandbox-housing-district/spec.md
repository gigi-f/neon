## ADDED Requirements

### Requirement: Housing subsection sandbox district
The world SHALL initialize with a deterministic housing district in the sandbox map.

#### Scenario: Housing district exists at startup
- **WHEN** the simulation starts
- **THEN** at least one building SHALL be zoned residential for the housing district

---

### Requirement: Housing district transit access
The housing district SHALL be connected to sandbox transit infrastructure.

#### Scenario: Transit station indices are initialized
- **WHEN** sandbox world generation completes
- **THEN** station indices 0 and 1 SHALL exist
- **AND** at least one station SHALL be connected to housing via pedestrian pathing geometry

---

### Requirement: Housing subsection startup validation
Startup SHALL validate housing-subsection invariants.

#### Scenario: Validation failure aborts startup
- **WHEN** required housing or transit invariants are missing
- **THEN** startup SHALL fail with a clear validation error
