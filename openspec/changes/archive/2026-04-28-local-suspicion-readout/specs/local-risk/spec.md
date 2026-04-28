## ADDED Requirements

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
