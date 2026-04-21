## ADDED Requirements

### Requirement: NPC Conversation State
Citizens that can participate in conversation SHALL carry a `ConversationComponent` with runtime state:
- `cooldown` (seconds remaining)
- `last_partner` (`Entity`, default `MAX_ENTITIES`)

#### Scenario: Newly spawned citizen can converse
- **WHEN** a citizen NPC is spawned
- **THEN** they SHALL include `ConversationComponent` initialized with zero cooldown and no last partner.

### Requirement: L2 Conversation Pairing
The simulation SHALL run a conversation pass at L2 cadence and pair nearby NPCs for at most one conversation each tick.

Pairing rules:
- participants are within 35 WU
- both have cooldown <= 0
- each participant is used in at most one pair per L2 tick
- immediate repeat pairing with `last_partner` is avoided when alternatives exist

#### Scenario: Cooldown blocks repeat conversation
- **WHEN** two citizens converse and then are evaluated on the next L2 tick while cooldown > 0
- **THEN** they SHALL NOT produce another conversation event.

### Requirement: Conversation Fragments And Memory
Each accepted conversation pair SHALL generate one fragment from deterministic templates and apply conversational side effects to both participants.

Side effects:
- each participant records one `HEARD_RUMOR` memory with the other as source
- each participant receives a small PAD adjustment based on fragment category
- both participants receive non-zero cooldown and update `last_partner`

#### Scenario: Conversation records rumor memory
- **WHEN** a conversation occurs between NPC A and NPC B
- **THEN** A SHALL record `HEARD_RUMOR` sourced from B and B SHALL record `HEARD_RUMOR` sourced from A.

### Requirement: Player Eavesdropping Intel Alerts
If the player is within 70 WU of a conversation participant, the system SHALL emit an info-severity `INTEL` alert containing the overheard fragment summary.

#### Scenario: Nearby player hears conversation
- **WHEN** a conversation occurs and the player is inside listen radius
- **THEN** an `INTEL` alert SHALL be pushed for HUD/Intel display.
