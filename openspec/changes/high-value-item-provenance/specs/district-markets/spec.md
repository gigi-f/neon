## MODIFIED Requirements

### Requirement: Player market trade
Player market buy and sell transactions SHALL preserve item provenance when traded goods are tracked.

#### Scenario: Market purchase creates tracked provenance when needed
- **WHEN** a player buys a market item whose flags require provenance tracking
- **THEN** the stored inventory item SHALL record the market as its source
- **AND** the player SHALL be recorded as the owner

#### Scenario: Market sale preserves tracked provenance before removal
- **WHEN** a player sells a tracked item
- **THEN** the transaction SHALL use the selected item's current provenance
- **AND** the sold inventory slot SHALL be cleared only after the trade succeeds
