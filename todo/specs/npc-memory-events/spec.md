# Requirements

### Requirement: Violence Event Emission
When a citizen dies (health ≤ 0), `BiologySystem` SHALL emit a `ViolenceEvent`.

- A `ViolenceEvent { float x, y; }` struct SHALL be defined in `components.h`.
- `BiologySystem::update()` SHALL accept a `std::vector<ViolenceEvent>&` out-parameter and push `{t.x, t.y}` for each dying entity before it is destroyed.

### Requirement: Witness Memory Formation
`MemoryFormationSystem::update()` SHALL run at the L1 gate, immediately after `BiologySystem`.

- **WHEN** a `ViolenceEvent` position is within 80 WU of a citizen with a `CognitiveComponent`
- **THEN** the citizen SHALL call `cog.record({ SAW_VIOLENCE, game_hour, 0.8f, MAX_ENTITIES })`
- **AND** `cog.pleasure` SHALL decrease by 0.3, clamped to -1.0
- **AND** `cog.arousal` SHALL increase by 0.4, clamped to 1.0

#### Scenario: Proximity threshold
- **WHEN** a citizen is ≤ 80 WU from the death position, they witness it.
- **WHEN** a citizen is > 80 WU away, they are unaffected.
- One violence event may affect multiple nearby citizens simultaneously.
- All PAD values clamp to [-1.0, 1.0].

### Requirement: Event Buffer Lifetime
The `std::vector<ViolenceEvent>` SHALL be declared in `main.cpp` and cleared before each
L1 `BiologySystem` call. No violence events carry over between L1 ticks.
