# Requirements

### Requirement: RelationshipComponent
Every citizen NPC SHALL carry a `RelationshipComponent` with a fixed array of up to 8
relationship entries. Each entry tracks:
- `other`: Entity (MAX_ENTITIES = vacant slot)
- `affinity`: float, range [-1.0, 1.0] (negative = adversarial, positive = friendly)

Affinity tier is derived at query time:
| Affinity range | Tier |
|----------------|------|
| ≥ 0.6 | INTIMATE |
| ≥ 0.2 | FRIEND |
| ≥ 0.0 | ACQUAINTANCE |
| ≥ −0.4 | STRANGER |
| < −0.4 | ENEMY |

#### Scenario: Component Assignment
- **WHEN** a citizen entity is spawned by `AmbientSpawnSystem`
- **THEN** it SHALL receive a default `RelationshipComponent` (count = 0, all entries vacant)

### Requirement: RelationshipSystem
`RelationshipSystem::update()` SHALL run at the L2 tick gate (~2 Hz).
Two passes per update:

**Pass 1 — Proximity formation / reinforcement**
- For each pair of citizens (i < j) within 30 WU of each other:
  - If no entry exists for the other: create a new entry with affinity = 0.02
  - If an entry exists: affinity += 0.02, clamped to 1.0
  - Update is applied bidirectionally (both citizens)
- If a citizen's 8-slot table is full, the weakest positive slot is evicted to make room.
  Enemy slots (negative affinity) are never evicted.

**Pass 2 — Passive decay**
- Every active slot: affinity moves toward 0 by 0.003 per tick
- Slots where |affinity| < 0.01 are forgotten (removed)

#### Scenario: Relationship formation through proximity
- **WHEN** two citizens remain within 30 WU for repeated L2 ticks
- **THEN** their mutual affinity SHALL increase from 0 toward 1.0 asymptotically

#### Scenario: Relationship decay through absence
- **WHEN** two citizens are no longer near each other
- **THEN** their affinity SHALL decay toward 0 and eventually be forgotten

### Requirement: Behavioral Steering (CitizenAISystem Integration)
`CitizenAISystem::update()` SHALL apply social forces from the citizen's relationship table
after resolving goal/wander velocity:

- **Friend attraction** (affinity > 0.4, target > 40 WU away): additive pull vector
  toward the friend, weight = 0.3 × affinity, normalized and scaled by 0.4 × citizen speed
- **Enemy repulsion** (affinity < −0.4): additive push vector away from the enemy,
  weight = 0.5 × |affinity|, normalized and scaled by 0.4 × citizen speed

Stale entity references (dead entities) SHALL be skipped via `registry.alive()` guard.

#### Scenario: Friends cluster over time
- **WHEN** two citizens develop affinity > 0.4
- **THEN** they SHALL gravitationally attract when separated by more than 40 WU,
  causing small social clusters to form organically

### Requirement: Infrastructure Unchanged
Road, transit, biology, schedule, and GDI systems SHALL NOT be modified.
The sleeping-citizen early-return in CitizenAISystem (ScheduleState::SLEEPING) SHALL
continue to fire before social steering, keeping sleeping citizens stationary.
