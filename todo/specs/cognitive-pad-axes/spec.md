# Requirements

### Requirement: CognitiveComponent
Every citizen NPC and the player SHALL carry a `CognitiveComponent` containing:
- PAD axes: `pleasure`, `arousal`, `dominance` (float, range -1.0 to +1.0, default 0.0)
- A fixed ring buffer of 16 `MemoryRecord` entries, with `mem_head` (write index) and
  `mem_size` (0–16 count)

#### Scenario: Component Assignment
- **WHEN** a citizen entity is spawned by `AmbientSpawnSystem`
- **THEN** it SHALL receive a default `CognitiveComponent` (all PAD axes = 0, empty memory)
- **AND WHEN** the player entity is created at startup
- **THEN** it SHALL also receive a `CognitiveComponent`

### Requirement: MemoryRecord
A `MemoryRecord` SHALL store: `event_type` (enum), `timestamp` (game_hour float),
`intensity` (0.0–1.0 float), and `source` (Entity). Event types: SAW_FOOD, ATE_FOOD,
DRANK_WATER, SAW_VIOLENCE, HEARD_RUMOR.

`CognitiveComponent::record(MemoryRecord)` SHALL write into the ring at `mem_head`,
advance head mod 16, and increment `mem_size` up to 16.

### Requirement: PAD Decay
`CognitiveSystem::update()` SHALL run at the L2 tick gate (~2 Hz).
Each PAD axis SHALL decay toward 0 at `0.05 × time_scale × dt` per call.

#### Scenario: Emotional Recovery
- **WHEN** `pleasure = 0.8` and sufficient time passes
- **THEN** `pleasure` SHALL approach 0.0 asymptotically (clamped, never crosses 0)

### Requirement: Biology → PAD Feedback
When a citizen or player has BiologyComponent with critically low stats, PAD SHALL shift:

| Condition           | Effect                                    |
|---------------------|-------------------------------------------|
| `hunger < 20`       | pleasure −0.02 × ts × dt, arousal +0.01 × ts × dt |
| `thirst < 20`       | pleasure −0.03 × ts × dt, arousal +0.02 × ts × dt |

All PAD values clamp to [-1.0, 1.0].

### Requirement: Arousal → Speed Influence
Citizens' base wander speed SHALL be modulated by their arousal axis:
`speed = 30.0 × (1.0 + 0.5 × arousal)` — range [15, 45] WU/s.

### Requirement: Consumption Memory Recording
- **WHEN** the player consumes a FOOD item (playerPickup succeeds on FOOD)
- **THEN** an ATE_FOOD MemoryRecord SHALL be recorded into the player's CognitiveComponent
  with `intensity = (restored_value / 100.0)` and `timestamp = current game_hour`
- **AND** player `pleasure` SHALL increase by 0.3, clamped to 1.0
- (Citizen consumption memory is deferred to a future event-broadcast spec)
