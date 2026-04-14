# Requirements

### Requirement: Ambient Vehicle Spawning
The system SHALL spawn non-essential "ambient" vehicles in real-time within the camera viewport or nearby based on the local `RoadComponent.traffic_density` or `ZoningComponent` type. Spawning SHALL account for Axis-Aligned Bounding Box (AABB) overlap to prevent clipping.

#### Scenario: High Density Ambient Spawn
- **WHEN** the player enters an URBAN_CORE zone during peak simulation hours
- **THEN** the `AmbientSpawnSystem` SHALL increase the number of ambient vehicle entities using continuous AABB collision checks to simulate "bumper-to-bumper" traffic.

### Requirement: Off-Screen Despawning
To maintain performance, ambient vehicles and pedestrians SHALL be despawned once they exit a certain radius around the player or camera viewport.

#### Scenario: Memory Cleanup
- **WHEN** an ambient vehicle entity moves further than `MAX_SIM_RADIUS` from all active players
- **THEN** the entity SHALL be safely removed from the registry.
