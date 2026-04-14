# Requirements

### Requirement: Distance-Based Building Height
The `CityGenerationSystem` SHALL determine building floor counts based on distance to the `URBAN_CORE` according to the following gradient (measured in World Units):
- **URBAN_CORE**: 30 to 150 floors
- **CORPORATE** (dist 40-80 WU): 15 to 50 floors
- **COMMERCIAL** (dist 80-120 WU): 5 to 20 floors
- **RESIDENTIAL** (dist 120-200 WU): 3 to 8 floors
- **SLUM** (dist 200+ WU): 1 to 4 floors
- **INDUSTRIAL**: 1 to 5 floors

#### Scenario: Residential Height Gradient
- **WHEN** a building is generated in a RESIDENTIAL zone at distance 160 WU from the core
- **THEN** the floor count is between 3 and 8 based on the gradient formula.

### Requirement: Building ID Uniqueness
The system SHALL calculate building `stable_id` using a 64-bit hash (e.g., of its world coordinates) to prevent collisions in high-density areas.

#### Scenario: Unique Stable IDs
- **WHEN** multiple buildings are generated in close proximity
- **THEN** each building is assigned a unique `stable_id` that is used to retrieve its interior.
