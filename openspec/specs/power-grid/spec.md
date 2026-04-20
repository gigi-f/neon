# power-grid Specification

## Purpose
TBD - created by archiving change power-grid-simulation. Update Purpose after archive.
## Requirements
### Requirement: Power Node and Conduit Entities
The system SHALL represent the power grid using node and conduit components attached to ECS entities.

#### Scenario: Grid Setup
- **WHEN** the world generation runs
- **THEN** power node entities are placed and connected via conduit entities

### Requirement: Power Grid Simulation Update
The system SHALL simulate power distribution periodically, determining whether each node receives sufficient power.

#### Scenario: Sufficient Power Supply
- **WHEN** total power supply on a connected sub-grid exceeds or equals the total demand
- **THEN** all consumer nodes on that sub-grid are marked as powered

#### Scenario: Insufficient Power Supply (Brownout)
- **WHEN** total power supply is less than the total demand
- **THEN** some or all nodes on the sub-grid are marked as unpowered based on priority or distance

