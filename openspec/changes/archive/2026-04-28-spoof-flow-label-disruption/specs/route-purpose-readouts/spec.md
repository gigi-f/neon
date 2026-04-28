## MODIFIED Requirements

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

## ADDED Requirements

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
