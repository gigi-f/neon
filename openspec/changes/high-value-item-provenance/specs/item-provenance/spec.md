## ADDED Requirements

### Requirement: Lightweight provenance records
Items SHALL support lightweight provenance metadata for goods that are stolen, unique, quest-like, faction-relevant, or high-value.

#### Scenario: Common commodity remains untracked
- **WHEN** an item has only the legal flag and no provenance metadata
- **THEN** provenance SHALL be reported as untracked

#### Scenario: Important flagged item is tracked
- **WHEN** an item has `ITEM_FLAG_UNIQUE`, `ITEM_FLAG_HIGH_VALUE`, `ITEM_FLAG_FACTION_RELEVANT`, `ITEM_FLAG_QUEST`, or `ITEM_FLAG_ILLEGAL`
- **THEN** provenance SHALL be reported as tracked

#### Scenario: Foreign-owned tracked item becomes stolen on pickup
- **WHEN** the player picks up a tracked item with an owner that is not the player
- **THEN** the carried item provenance SHALL mark the item as stolen
- **AND** the previous owner SHALL remain visible in provenance metadata

#### Scenario: Player-owned tracked item stays clean on pickup
- **WHEN** the player picks up a tracked item already owned by the player
- **THEN** the carried item provenance SHALL remain tracked
- **AND** the carried item SHALL NOT be marked stolen

### Requirement: Provenance display
Tracked item provenance SHALL be visible in player-facing inspection surfaces.

#### Scenario: Surface Scan shows provenance
- **WHEN** Surface Scan targets a tracked item entity
- **THEN** the scan panel SHALL include compact provenance text

#### Scenario: Financial Forensics shows provenance
- **WHEN** Financial Forensics targets a tracked item entity
- **THEN** the scan panel SHALL include compact provenance text
