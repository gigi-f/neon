# local-risk Specification

## Purpose
Track small, local worker suspicion from witnessed player interference before any city-wide surveillance, wanted level, faction response, or pursuit system exists.
## Requirements
### Requirement: Witnessed output theft creates local suspicion
The system SHALL record a current-scope local suspicion event when a worker witnesses the player taking expected workplace output.

#### Scenario: Ready worker witnesses missing part
- **WHEN** the workplace bench has output ready
- **AND** a fixed worker is ready to collect that output or close enough to the player/workplace
- **AND** the player takes the output
- **THEN** local suspicion SHALL be recorded with cause `MISSING PART`
- **AND** the HUD/status readout SHALL tell the player the action was noticed

#### Scenario: Unwitnessed output pickup remains unchanged
- **WHEN** no worker can witness the output pickup
- **AND** the player takes the output
- **THEN** no local suspicion SHALL be recorded
- **AND** the normal carried part and bench state behavior SHALL remain unchanged

### Requirement: Witnessed route tampering creates local suspicion
The system SHALL record a current-scope local suspicion event when a worker near the affected route witnesses Interference Torch signpost spoofing.

#### Scenario: Worker witnesses route tampering
- **WHEN** a fixed worker is on the signpost's route and close enough to the signpost
- **AND** the player uses `G` Interference Torch to spoof that signpost
- **THEN** local suspicion SHALL be recorded with cause `ROUTE TAMPERING`
- **AND** the HUD/status readout SHALL tell the player the action was noticed

#### Scenario: Restoring route does not erase suspicion
- **WHEN** local suspicion was created by witnessed route tampering
- **AND** the player uses `G` Interference Torch to restore the signpost
- **THEN** the route flow blockage SHALL clear
- **AND** the local suspicion SHALL remain recorded

#### Scenario: No global surveillance state
- **WHEN** local suspicion is recorded
- **THEN** the system SHALL NOT create wanted level, faction, pursuit, or city-wide surveillance state
