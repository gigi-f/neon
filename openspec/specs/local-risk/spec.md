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

### Requirement: Local suspicion de-escalation
The system SHALL let the player de-escalate current-scope local suspicion through one matching local action while preserving an inspectable record.

#### Scenario: Player returns suspicious missing output
- **WHEN** local suspicion was caused by missing workplace output
- **AND** the player carries the suspected part inside the affected workplace
- **AND** the player uses `E`
- **THEN** the part SHALL return to the workplace output state
- **AND** local suspicion SHALL become de-escalated instead of being deleted

#### Scenario: Player corrects suspicious route tampering
- **WHEN** local suspicion was caused by route tampering
- **AND** the player restores the same affected signpost with `G` Interference Torch
- **THEN** the route SHALL remain mechanically clear
- **AND** local suspicion SHALL become de-escalated instead of being deleted

#### Scenario: Player hides suspicious item locally
- **WHEN** local suspicion was caused by missing workplace output
- **AND** the player carries the suspected part inside housing
- **AND** the player uses `E`
- **THEN** the item SHALL become hidden in the current housing scope
- **AND** local suspicion SHALL remain inspectable as hidden local concern

### Requirement: Local suspicion persistence
The system SHALL persist current-scope local suspicion records that affect inspection while leaving volatile acknowledgement text unsaved.

#### Scenario: Active suspicion round trips
- **WHEN** active local suspicion is saved and loaded
- **THEN** the same cause and affected target SHALL remain inspectable

#### Scenario: De-escalated suspicion round trips
- **WHEN** returned, corrected, or hidden local suspicion is saved and loaded
- **THEN** the quieter de-escalated readout SHALL remain inspectable
- **AND** the original immediate HUD notice SHALL NOT be recreated from save/load alone

#### Scenario: No-suspicion round trip remains clear
- **WHEN** no local suspicion exists
- **THEN** saving and loading SHALL NOT create suspicion readouts

### Requirement: Institutional log fragment
The system SHALL expose one deterministic workplace log fragment after qualifying local suspicion without activating a broader institution system.

#### Scenario: Debugger recovers local workplace fragment
- **WHEN** the Debugger inspects the affected workplace after local suspicion or de-escalation exists
- **THEN** the volatile Debugger result SHALL include a short institutional log fragment
- **AND** the local suspicion record SHALL mark the fragment as recovered

#### Scenario: No qualifying state reports no fragment
- **WHEN** the same workplace has no qualifying local suspicion or de-escalation state
- **THEN** Debugger inspection SHALL NOT report an institutional log fragment

#### Scenario: Recovered fragment adds local clue
- **WHEN** the institutional fragment has been recovered
- **THEN** affected worker or workplace inspection SHALL include a compact local clue
- **AND** no wanted level, faction response, pursuit, economy, or surveillance network SHALL be created

