## ADDED Requirements

### Requirement: District geometry does not overlap roadway geometry
Sandbox district building geometry SHALL not overlap any roadway rectangles.

#### Scenario: Housing geometry is road-safe
- **WHEN** sandbox generation completes
- **THEN** residential building rectangles SHALL not intersect any `RoadComponent` rectangle

---

### Requirement: Transit stop support geometry is road-safe
Transit stop support entities SHALL be placed off roadway geometry.

#### Scenario: Station support geometry avoids roads
- **WHEN** sandbox generation completes
- **THEN** `StationComponent`, `StaircaseComponent`, and `WaitingAreaComponent` rectangles SHALL not intersect any `RoadComponent` rectangle

---

### Requirement: Player-owned vehicle is not spawned on maglift track
The player-owned vehicle SHALL start away from the maglift track alignment.

#### Scenario: Player-owned vehicle starts off-track
- **WHEN** startup entity initialization completes
- **THEN** the player-owned vehicle transform SHALL not intersect the maglift track rectangle
- **AND** the player-owned vehicle SHALL not use `VehicleComponent::MAGLIFT`
