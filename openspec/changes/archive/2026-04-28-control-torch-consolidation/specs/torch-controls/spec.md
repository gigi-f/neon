## ADDED Requirements

### Requirement: Space performs Debugger inspection
The system SHALL use `SPACE` as the single inspection action, combining the existing ordinary target readout with the richer Debugger inspection result.

#### Scenario: Space inspects a target
- **WHEN** the player presses `SPACE` near a valid inspection target
- **THEN** the ordinary inspection target/result SHALL update
- **AND** the Debugger result SHALL update with the richer signal detail for that target

#### Scenario: Space has no target
- **WHEN** the player presses `SPACE` with no valid inspection target
- **THEN** the ordinary inspection result SHALL report no target
- **AND** the Debugger result SHALL report no signal

### Requirement: G performs Interference Torch infrastructure interference
The system SHALL use plain `G` for the reversible infrastructure interference action that previously required `Shift+G`.

#### Scenario: G targets a signpost
- **WHEN** the player presses `G` near a route signpost
- **THEN** the signpost SHALL toggle between spoofed and restored state
- **AND** the result SHALL identify the action as Interference Torch output

#### Scenario: G targets a dependency endpoint
- **WHEN** the player presses `G` near the workplace/supply dependency endpoint
- **THEN** the dependency SHALL toggle between disrupted and restored state
- **AND** the result SHALL identify the action as Interference Torch output

#### Scenario: Shift is not required
- **WHEN** prompts describe signpost or dependency interference
- **THEN** they SHALL reference `G`
- **AND** they SHALL NOT reference `Shift+G`
