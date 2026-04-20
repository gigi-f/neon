## MODIFIED Requirements

### Requirement: Discrete inventory use and drop
The player SHALL be able to use or drop the selected occupied inventory entry, and quick-use actions SHALL be able to consume the first carried item matching the requested consumable type.

#### Scenario: Selected item is used
- **WHEN** the selected inventory slot contains a consumable
- **THEN** the player's biological state SHALL be restored according to the item type
- **AND** the inventory slot SHALL become empty

#### Scenario: Selected item is dropped
- **WHEN** the selected inventory slot contains an item
- **THEN** a matching world item SHALL be created near the player
- **AND** the inventory slot SHALL become empty

#### Scenario: Consumable quick-use finds matching carried item
- **WHEN** the player quick-uses food, water, or medical supplies
- **AND** a matching discrete inventory item is carried
- **THEN** that carried item SHALL be consumed and removed from its slot
