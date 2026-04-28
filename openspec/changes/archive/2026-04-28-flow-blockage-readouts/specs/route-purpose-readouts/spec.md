## ADDED Requirements

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
