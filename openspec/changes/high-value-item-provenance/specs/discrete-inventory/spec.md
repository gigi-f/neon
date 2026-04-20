## MODIFIED Requirements

### Requirement: Discrete inventory inspection
The player SHALL be able to inspect a selected occupied inventory entry, including provenance when the entry is tracked.

#### Scenario: Selected item is occupied
- **WHEN** the selected inventory slot contains an item
- **THEN** inspection SHALL expose its type, restore value, flags, and provenance metadata

#### Scenario: Selected item is empty
- **WHEN** the selected inventory slot is empty
- **THEN** inspection SHALL report no item

### Requirement: Discrete inventory use and drop
The player SHALL be able to use or drop the selected occupied inventory entry, and quick-use actions SHALL be able to consume the first carried item matching the requested consumable type.

#### Scenario: Selected item is dropped
- **WHEN** the selected inventory slot contains a tracked item
- **THEN** a matching world item SHALL be created near the player
- **AND** the world item SHALL preserve the carried item's provenance
- **AND** the inventory slot SHALL become empty
