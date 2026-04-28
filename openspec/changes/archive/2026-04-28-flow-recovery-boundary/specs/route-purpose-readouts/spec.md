## ADDED Requirements

### Requirement: Local route flow recovery readout
The system SHALL derive visible local recovery text from restored route signposts without introducing new repair commands or city-wide flow state.

#### Scenario: Restoring signpost shows recovered route flow
- **WHEN** a spoofed route signpost is restored
- **THEN** that signpost readout SHALL include `FLOW: CLEAR`
- **AND** that path inspection SHALL include `FLOW: CLEAR`
- **AND** neither readout SHALL include `FLOW: BLOCKED`

#### Scenario: Restoring supply route clears workplace flow disruption
- **WHEN** all signposts on the WORKPLACE to SUPPLY route are restored
- **THEN** workplace inspection SHALL no longer include `SUPPLY FLOW: DISRUPTED`
- **AND** it SHALL include `SUPPLY FLOW: CLEAR`

#### Scenario: New spoof replaces recovered flow state
- **WHEN** a recovered route signpost is spoofed again
- **THEN** signpost and path readouts SHALL show `FLOW: BLOCKED`
- **AND** they SHALL no longer show `FLOW: CLEAR`

### Requirement: Route flow persistence boundary
The system SHALL persist active spoofed route blockages through the current tiny save state and SHALL NOT add separate persistence for volatile recovery acknowledgement text.

#### Scenario: Active blockage survives save and load
- **WHEN** a route signpost is spoofed and the tiny save state is restored
- **THEN** the affected signpost and path readouts SHALL still include `FLOW: BLOCKED`

#### Scenario: Restored route loads as clear normal flow
- **WHEN** a route signpost is restored before tiny save state is restored
- **THEN** the affected signpost and path readouts SHALL not include stale `FLOW: BLOCKED`
- **AND** they SHALL expose normal clear route labels without requiring new recovery event state
