## ADDED Requirements

### Requirement: Clinic access ledger reflects local worker records
The system SHALL expose a clinic access ledger flag when an existing local worker suspicion or wage record affects municipal service access.

#### Scenario: Clinic scan shows flagged worker record
- **WHEN** a worker has a local suspicion or wage record
- **AND** the player inspects the clinic with ordinary inspection or `SPACE` Debugger scan
- **THEN** the readout SHALL include `CLINIC LEDGER: WORK RECORD FLAGGED`
- **AND** the AI playtest target detail SHALL expose the same clinic ledger state

#### Scenario: No worker record leaves clinic ledger clear
- **WHEN** no worker has a local suspicion or wage record
- **AND** the player inspects the clinic
- **THEN** the clinic readouts SHALL NOT include a flagged clinic ledger
- **AND** `G` Interference Torch SHALL NOT offer a clinic access spoof target

### Requirement: Clinic access can be locally spoofed
The system SHALL let `G` Interference Torch toggle exactly one clinic-local access spoof state while a worker record exists.

#### Scenario: Spoofing clinic access creates ghost clearance
- **WHEN** a worker has a local suspicion or wage record
- **AND** the player uses `G` Interference Torch on the clinic
- **THEN** the clinic access spoof state SHALL become active
- **AND** ordinary clinic inspection, `SPACE` Debugger scan, and AI playtest target detail SHALL include `CLINIC ACCESS: GHOST CLEARANCE`

#### Scenario: Restoring clinic access clears ghost clearance
- **WHEN** clinic access is spoofed
- **AND** the player uses `G` Interference Torch on the clinic again
- **THEN** the clinic access spoof state SHALL become inactive
- **AND** clinic readouts SHALL still show the flagged worker record if the underlying record still exists
- **AND** clinic readouts SHALL NOT include stale ghost clearance

#### Scenario: Clinic access spoof has narrow scope
- **WHEN** clinic access is spoofed or restored
- **THEN** the clinic SHALL remain non-enterable
- **AND** local suspicion, wage-record spoofing, route blockage, workplace audit traces, health, injury, medicine, doctors, appointments, public-works state, factions, and city-wide surveillance SHALL NOT be created or cleared by this action
