## ADDED Requirements

### Requirement: Paired transit stations
The system SHALL create paired station entities only when transit is enabled and at least two authored districts exist.

#### Scenario: Two-district transit config
- **WHEN** transit is enabled for a two-district sandbox config
- **THEN** each district SHALL receive one visible station glyph
- **AND** the stations SHALL be paired by a shared transit link
- **AND** each station readout SHALL name its current district and destination district

#### Scenario: One-district baseline
- **WHEN** transit is disabled or fewer than two districts exist
- **THEN** no station entities SHALL be created
- **AND** the one-district sandbox baseline SHALL continue to build cleanly

### Requirement: Transit ride interior
The system SHALL treat boarding transit as entering an interior ride space that advances in real time.

#### Scenario: Board station
- **WHEN** the player is near a paired station
- **AND** the player presses `E`
- **THEN** the player SHALL enter a transit interior
- **AND** the ride SHALL track origin station, destination station, elapsed time, door state, exterior position, and interior position

#### Scenario: Move inside transit
- **WHEN** the player is inside transit
- **AND** the player uses movement input
- **THEN** the player's transit interior position SHALL move within the transit car bounds
- **AND** the trip timer SHALL continue advancing

### Requirement: Transit exit choices
The system SHALL let the player choose between looking out the window immediately or waiting for the destination stop.

#### Scenario: Look out window
- **WHEN** the player is inside transit before the doors open
- **AND** the player presses `E`
- **THEN** the player SHALL exit at the destination platform immediately
- **AND** any carried object SHALL remain carried

#### Scenario: Wait for doors
- **WHEN** the player waits inside transit until the ride reaches its stop interval
- **THEN** the doors SHALL open
- **AND** pressing `E` SHALL exit at the destination platform

### Requirement: Transit persistence and AI playtest visibility
The system SHALL persist and expose transit state through existing save and AI playtest surfaces.

#### Scenario: Save during ride
- **WHEN** the player saves while inside transit
- **THEN** loading SHALL restore the active ride, origin, destination, elapsed time, door state, and interior position

#### Scenario: AI playtest snapshot
- **WHEN** AI playtest observes the world
- **THEN** station targets, transit prompts, current district, transit ride readout, and `T` map glyphs SHALL be visible in snapshots
