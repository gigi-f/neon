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

### Requirement: Local suspicion ordinary inspection
The system SHALL expose active local suspicion through compact ordinary inspection lines on the witnessing worker and affected workplace.

#### Scenario: Worker inspection shows suspicion cause
- **WHEN** a worker has active local suspicion
- **THEN** ordinary worker inspection SHALL include `SUSPICION: MISSING PART` or `SUSPICION: ROUTE TAMPERING`

#### Scenario: Affected workplace inspection shows suspicion cause
- **WHEN** local suspicion is tied to a workplace
- **THEN** ordinary workplace inspection SHALL include the same compact suspicion cause

#### Scenario: No suspicion keeps readouts clear
- **WHEN** no local suspicion is active for the inspected worker or workplace
- **THEN** ordinary inspection SHALL NOT include `SUSPICION:`

### Requirement: Local suspicion Debugger inspection
The system SHALL expose local witness detail through `SPACE` Debugger inspection on the worker, affected workplace, and affected route while keeping active blockage and lingering suspicion distinct.

#### Scenario: Debugger inspects suspected worker
- **WHEN** the Debugger inspects a worker with active local suspicion
- **THEN** the result SHALL identify the local witness and suspected cause

#### Scenario: Debugger inspects affected workplace
- **WHEN** the Debugger inspects a workplace tied to active local suspicion
- **THEN** the result SHALL identify the local witness and suspected cause

#### Scenario: Debugger inspects affected route
- **WHEN** the Debugger inspects a path or signpost tied to active route-tampering suspicion
- **THEN** the result SHALL identify the local witness and suspected cause

#### Scenario: Restored flow remains distinct from suspicion
- **WHEN** route flow has been restored after witnessed tampering
- **THEN** Debugger route inspection SHALL NOT report active flow blockage
- **AND** it SHALL still report the lingering local suspicion

