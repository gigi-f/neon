## ADDED Requirements

### Requirement: Sandbox microcity district baseline
The simulation SHALL initialize a deterministic sandbox map that includes all required district roles: housing, workplace, market, leisure area, and upper-class quarters.

#### Scenario: Required districts exist at startup
- **WHEN** a new simulation run starts
- **THEN** the world SHALL contain at least one authored district instance for each required role
- **AND** district roles SHALL be discoverable for debug/inspection output

---

### Requirement: Transit connects all required districts
The sandbox map SHALL provide transit connectivity between all required district roles so routine movement can be evaluated end-to-end.

#### Scenario: District reachability through transit graph
- **WHEN** the sandbox map is initialized
- **THEN** transit routes SHALL allow travel between each required district role pair, directly or via transfers

---

### Requirement: Procedural generation is removed for sandbox-first validation
Sandbox layout initialization SHALL be the world setup path for the sandbox validation phase.

#### Scenario: Sandbox layout is always used
- **WHEN** the simulation starts
- **THEN** the world SHALL load the deterministic sandbox layout instead of broad procedural generation

#### Scenario: Procedural path is unavailable
- **WHEN** developers attempt to use procedural world setup
- **THEN** the runtime SHALL not expose a procedural generation path during this phase
