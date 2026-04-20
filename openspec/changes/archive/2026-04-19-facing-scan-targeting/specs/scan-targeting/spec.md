## ADDED Requirements

### Requirement: Facing-aware scan target selection
Scan panel target selection SHALL prefer eligible entities in the player's current facing direction before considering eligible entities outside that direction.

#### Scenario: Forward entity beats closer side entity
- **WHEN** the player scans with one eligible entity in the facing direction and another closer eligible entity outside the facing direction
- **THEN** the scan target SHALL be the entity in the facing direction

#### Scenario: Nearest target remains fallback
- **WHEN** the player scans and no eligible entity is in the facing direction
- **THEN** the scan target SHALL be the nearest eligible entity within scan range

#### Scenario: All scan panels use the same target rule
- **WHEN** the player opens Surface Scan, Biological Audit, Cognitive Profile, Financial Forensics, or Structural Analysis
- **THEN** the selected target SHALL be chosen by the facing-aware scan target selection rule
