## ADDED Requirements

### Requirement: Intel Alert Category
The simulation alert model SHALL support an `INTEL` category for player-overheard information fragments.

#### Scenario: Eavesdrop fragment is categorized as intel
- **WHEN** an overheard NPC conversation is surfaced to the player
- **THEN** the emitted alert SHALL use category `INTEL`.
