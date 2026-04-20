# Requirements

### Requirement: RumorSystem
`RumorSystem::update()` SHALL run at the L2 tick gate (~2 Hz), after `MemoryFormationSystem`
and `RelationshipSystem` have already run that tick.

**Single pass — social trauma propagation:**
For each citizen A with a `SAW_VIOLENCE` memory recorded within the last 2.0 game-hours
(intensity ≥ 0.5):
- Iterate A's `RelationshipComponent` entries where affinity ≥ 0.2 (FRIEND tier or better).
- For each such friend B: if B is alive, has `CognitiveComponent`, and is within 30 WU of A:
  - If B does NOT already have a `SAW_VIOLENCE` or `HEARD_RUMOR` memory within the
    last 2.0 game-hours, record `HEARD_RUMOR` in B's memory ring with intensity = 0.4
    and apply PAD shift: pleasure −0.15, arousal +0.2 (clamped to [−1, 1]).
  - Citizens who received `HEARD_RUMOR` do NOT re-propagate (single-hop only).

No new components or data structures are required.

#### Scenario: First-hop trauma sharing
- **WHEN** a citizen witnessed violence (has `SAW_VIOLENCE` ≤ 2 game-hours ago)
  AND is within 30 WU of a friend (affinity ≥ 0.2)
  AND that friend has no recent violence memory
- **THEN** the friend SHALL receive a `HEARD_RUMOR` record (intensity 0.4) and
  a mild PAD shift (pleasure −0.15, arousal +0.2)

#### Scenario: Single-hop cap
- **WHEN** a citizen received `HEARD_RUMOR` (not `SAW_VIOLENCE`)
- **THEN** that citizen SHALL NOT propagate the rumor further

#### Scenario: Dedup guard
- **WHEN** a citizen already has `SAW_VIOLENCE` or `HEARD_RUMOR` within 2 game-hours
- **THEN** they SHALL NOT receive a duplicate rumor record from this tick

### Requirement: Infrastructure Unchanged
Biology, schedule, GDI, relationship, and memory formation systems SHALL NOT be modified.
Only `simulation_systems.h` (new class) and `main.cpp` (L2 wire-up) are touched.
