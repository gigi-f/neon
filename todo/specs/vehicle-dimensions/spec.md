# Requirements

### Requirement: Real-Time Vehicle Dimensions
The system SHALL support continuous vehicle dimensions and collision bounding boxes (AABBs):
- **EMMVs (Micro-Mobility Pods)** SHALL have a length of 6 World Units (WU).
- **Maglift Cars** SHALL have a length of 12 WU.
- **Maglift Transports** SHALL be treated as a single rigid body or a visual sequence of rigid bodies totaling at least 36 WU (3 cars).
- **Omni-Shuttles** SHALL have a length of 10 WU.
Articulated joints are removed to streamline real-time pathfinding; large vehicles SHALL be modeled as long, rigid collision boxes.

#### Scenario: EMMV Bounding Box
- **WHEN** an EMMV is spawned on a road
- **THEN** it occupies a 6x3 WU Axis-Aligned Bounding Box (AABB) for collision.

#### Scenario: Maglift Transport Spawning
- **WHEN** a Maglift Transport is spawned on a rail line
- **THEN** it SHALL be represented by a single collision body (AABB) of 36 WU that follows the track sequence in real-time.

### Requirement: Road Width Standards
Primary and secondary roads SHALL be at least 6 WU wide to accommodate 2 lanes of traffic.

#### Scenario: Primary Road Width
- **WHEN** a `ROAD_PRIMARY` segment is generated
- **THEN** its width is at least 6 WU.
