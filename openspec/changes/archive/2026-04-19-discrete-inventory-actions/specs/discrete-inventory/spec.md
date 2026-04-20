## ADDED Requirements

### Requirement: Discrete inventory entries
The player SHALL be able to carry individual item entries in a bounded inventory while the existing survival counters remain available.

#### Scenario: Nearby item is picked up
- **WHEN** the player picks up a nearby item
- **THEN** the item SHALL be stored as an occupied inventory entry
- **AND** the matching survival counter SHALL increase for compatibility with the existing HUD and hotkeys

#### Scenario: Inventory capacity is full
- **WHEN** every discrete inventory slot is occupied
- **THEN** pickup SHALL fail without destroying the world item

### Requirement: Discrete inventory inspection
The player SHALL be able to inspect a selected occupied inventory entry.

#### Scenario: Selected item is occupied
- **WHEN** the selected inventory slot contains an item
- **THEN** inspection SHALL expose its type, restore value, and flags

#### Scenario: Selected item is empty
- **WHEN** the selected inventory slot is empty
- **THEN** inspection SHALL report no item

### Requirement: Discrete inventory use and drop
The player SHALL be able to use or drop the selected occupied inventory entry.

#### Scenario: Selected item is used
- **WHEN** the selected inventory slot contains a consumable
- **THEN** the player's biological state SHALL be restored according to the item type
- **AND** the inventory slot SHALL become empty

#### Scenario: Selected item is dropped
- **WHEN** the selected inventory slot contains an item
- **THEN** a matching world item SHALL be created near the player
- **AND** the inventory slot SHALL become empty
