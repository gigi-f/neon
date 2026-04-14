# Requirements

### Requirement: Scale Separation for Transit
The `TransitSystem` and `CityGenerationSystem` SHALL differentiate between "Enterable" structures, "Infrastructure", and "Pedestrian Flow Zones" when scaling zones:
- **Enterable Structures** (Concourses, Terminals, Control Towers) SHALL use `createBuildingShell` and shrink in footprint based on the current lot size.
- **Infrastructure** (Platform safety areas, rail tracks, road segments, turnstiles, and scanners) SHALL remain at a fixed 1:1 World Unit scale regardless of lot density.
- **Pedestrian Flow Zones** (Wait areas, boarding zones) SHALL maintain a fixed scale to accommodate high pedestrian density during peak hours.

#### Scenario: Trans-Station Scaling
- **WHEN** an "Urban Transit Hub" is generated in a scaled URBAN_CORE lot
- **THEN** the concourse structure footprint shrinks according to the lot size, but the platform length, boarding zones, and rail tracks remain at their fixed 1:1 scale in World Units.
