# route-purpose-readouts Specification

## Purpose
Make derived pedestrian routes readable through path inspection, route signposts, and inherited debugger scans without adding a planner, minimap, traffic system, or extra route state.
## Requirements
### Requirement: Derived path route purpose
The system SHALL derive readable route purpose labels from pedestrian path endpoint roles.

#### Scenario: Housing to workplace path
- **WHEN** a pedestrian path connects HOUSING and WORKPLACE
- **THEN** the path readout SHALL include `LABOR ROUTE`

#### Scenario: Workplace to supply path
- **WHEN** a pedestrian path connects WORKPLACE and SUPPLY
- **THEN** the path readout SHALL include `SUPPLY ROUTE`

### Requirement: Signpost route purpose readout
The system SHALL include route purpose and carried-flow text in ordinary signpost inspection readouts, and spoofed signposts SHALL corrupt the carried-flow text while preserving destination and route identity.

#### Scenario: Normal signpost
- **WHEN** a route signpost points to an endpoint on a known route
- **THEN** its readout SHALL include destination, purpose, carried flow, signal state, and consequence

#### Scenario: Spoofed signpost
- **WHEN** a route signpost is spoofed
- **THEN** its readout SHALL keep the destination and purpose readable, SHALL show corrupted carried-flow text, and SHALL show the existing corrupted signal consequence

#### Scenario: Restored signpost
- **WHEN** a spoofed route signpost is restored
- **THEN** its readout SHALL clear corrupted carried-flow text and show restored clear signal text

### Requirement: Debugger route scan enrichment
The system SHALL expose richer route detail through the inherited debugger scan than ordinary inspection shows.

#### Scenario: Debugger scans a path
- **WHEN** the debugger scans a pedestrian path
- **THEN** the result SHALL include route purpose, expected cargo, and access detail

#### Scenario: Debugger scans a signpost
- **WHEN** the debugger scans a route signpost
- **THEN** the result SHALL include route purpose, expected cargo, access detail, and target role

### Requirement: Local flow consequence from spoofed route
The system SHALL derive local site-level flow consequence readouts from spoofed route signposts without introducing city-wide flow state.

#### Scenario: Supply route spoof disrupts workplace flow
- **WHEN** any signpost on the WORKPLACE to SUPPLY route is spoofed
- **THEN** workplace inspection SHALL include `SUPPLY FLOW: DISRUPTED`

#### Scenario: Restoring supply route clears workplace flow consequence
- **WHEN** all signposts on the WORKPLACE to SUPPLY route are restored
- **THEN** workplace inspection SHALL no longer include `SUPPLY FLOW: DISRUPTED`

#### Scenario: Unrelated route spoof does not disrupt supply flow
- **WHEN** a signpost on a route other than WORKPLACE to SUPPLY is spoofed
- **THEN** workplace inspection SHALL not include `SUPPLY FLOW: DISRUPTED`

### Requirement: Local route flow blockage readout
The system SHALL derive an explicit local flow blockage label from spoofed route signposts without introducing new blockage state.

#### Scenario: Spoofed signpost blocks its route readouts
- **WHEN** a route signpost on a pedestrian path is spoofed
- **THEN** that path inspection SHALL include `FLOW: BLOCKED`
- **AND** the spoofed signpost readout SHALL include `FLOW: BLOCKED`

#### Scenario: Restoring signpost clears route blockage readouts
- **WHEN** a spoofed route signpost is restored
- **THEN** that path inspection SHALL no longer include `FLOW: BLOCKED`
- **AND** the restored signpost readout SHALL no longer include `FLOW: BLOCKED`

#### Scenario: Unrelated routes remain clear
- **WHEN** a route signpost is spoofed
- **THEN** other route paths and signposts SHALL not show `FLOW: BLOCKED`

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

