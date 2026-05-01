# district-worker-isolation Specification

## Purpose
TBD - created by archiving change per-district-worker-isolation. Update Purpose after archive.
## Requirements
### Requirement: Per-district worker placement
The system SHALL spawn configured fixed workers on district-local labor routes before using fallback paths.

#### Scenario: Two-district worker config
- **WHEN** a two-district transit sandbox config requests two fixed workers
- **THEN** each district SHALL receive one worker on a local housing-workplace route
- **AND** each worker SHALL remain on paths inside its own district as it moves

### Requirement: District-tagged readouts
The system SHALL add compact district tags to multi-district worker and inspection readouts while preserving one-district strings.

#### Scenario: Multi-district labels
- **WHEN** a readout refers to a worker, route, purpose-bearing site, wage impact, local notice, or local witness in a multi-district world
- **THEN** the readout SHALL include a short district tag such as `A:WORKER`, `B:ROUTE`, or `A:WAGE IMPACT`

#### Scenario: One-district compatibility
- **WHEN** the world has only one district
- **THEN** existing readout strings SHALL remain unprefixed

### Requirement: District-local suspicion and institutional clues
The system SHALL prevent a worker record in one district from affecting worker, wage, clinic, or audit readouts in another district.

#### Scenario: Suspicion in district A
- **WHEN** district A has an active local suspicion record
- **THEN** district A worker and affected workplace readouts SHALL show that record
- **AND** district B worker, workplace, wage, clinic ledger, and audit readouts SHALL remain clear

#### Scenario: Clinic access spoof in district A
- **WHEN** district A clinic access is spoofed from a district A worker record
- **THEN** district A worker scan SHALL show the clinic access mismatch
- **AND** district B worker scan SHALL NOT show that mismatch

### Requirement: District-local dependency disruption
The system SHALL scope dependency disruption state to the district of the affected dependency target.

#### Scenario: Disrupt district A dependency
- **WHEN** the player disrupts the workplace/supply dependency in district A
- **THEN** district A workplace and supply readouts SHALL show the disruption
- **AND** district B workplace and supply readouts SHALL remain clear

### Requirement: Two-district save/load
The system SHALL round-trip both district workers and district-local worker records through tiny save/load.

#### Scenario: Save worker records
- **WHEN** district A has a worker record and district B has independent worker state
- **THEN** saving and loading SHALL restore both district workers
- **AND** only the original district A worker SHALL restore the local record

