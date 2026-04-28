## ADDED Requirements

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
