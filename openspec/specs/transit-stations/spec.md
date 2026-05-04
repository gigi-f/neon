# transit-stations Specification

## Purpose
Describe the paired station and ride-interior behavior that connects authored sandbox districts without introducing a transit network, vehicle simulation, schedules, traffic, or city-scale infrastructure.
## Requirements
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
- **AND** the station signal is in the boarding status
- **THEN** the player SHALL enter a transit interior
- **AND** the ride SHALL track origin station, destination station, elapsed time, door state, exterior position, and interior position

#### Scenario: Move inside transit
- **WHEN** the player is inside transit
- **AND** the player uses movement input
- **THEN** the player's transit interior position SHALL move within the transit car bounds
- **AND** the trip timer SHALL continue advancing

### Requirement: Scheduled train signal
The system SHALL expose an elapsed-time station signal for each paired transit station.

#### Scenario: Station status readout
- **WHEN** the player inspects or targets a station
- **THEN** the readout SHALL include the station's train status
- **AND** the readout SHALL include either the time until boarding or the time until departure
- **AND** the status SHALL advance from elapsed runtime rather than frame count

#### Scenario: Wait to board
- **WHEN** the player attempts to board outside the station's boarding status
- **THEN** the player SHALL stay at the station
- **AND** the action result SHALL explain how long to wait before boarding

#### Scenario: Board during signal window
- **WHEN** the player boards during the station's boarding status
- **THEN** the ride SHALL begin toward the paired destination district
- **AND** the action result SHALL name the next stop

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

#### Scenario: Save station timing
- **WHEN** the player saves while station signals are active
- **THEN** loading SHALL restore each station's signal elapsed time and cycle interval

#### Scenario: AI playtest snapshot
- **WHEN** AI playtest observes the world
- **THEN** station targets, transit prompts, current district, transit ride readout, and `T` map glyphs SHALL be visible in snapshots
- **AND** station target details SHALL report the train signal status and wait/board timing
