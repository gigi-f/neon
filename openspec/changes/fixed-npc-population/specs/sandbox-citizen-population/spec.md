## ADDED Requirements

### Requirement: Citizens seed across sandbox roads
Startup citizen seeding SHALL distribute citizens across the sandbox road network instead of a single local cluster.

#### Scenario: Citizens appear around map
- **WHEN** sandbox generation completes
- **THEN** seeded citizens SHALL occupy multiple road regions with visible spatial spread

---

### Requirement: Citizens avoid occupied non-road tiles
Citizen startup placement SHALL reject candidate positions that overlap non-road occupied geometry.

#### Scenario: No non-road overlap at spawn
- **WHEN** each citizen spawn candidate is evaluated
- **THEN** the accepted position SHALL not intersect any non-road transform entity
- **AND** road overlap remains allowed
