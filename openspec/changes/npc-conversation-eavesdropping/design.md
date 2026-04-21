## Context
Current L2 stack already updates schedules, goals, relationships, rumor propagation, and cognition. We can introduce conversations as another L2 pass without changing core movement or spawn architecture.

## Design

### D1: Conversation component state
Add `ConversationComponent` on citizens to rate-limit chatter:
- `cooldown` (seconds) until this NPC can initiate/accept another conversation.
- `last_partner` (Entity) to avoid immediate re-pairing loops.

Assigned during citizen spawn.

### D2: Conversation pair selection
`ConversationSystem::update(...)` runs at L2 and processes citizens with:
`CitizenComponent + TransformComponent + CognitiveComponent + RelationshipComponent + ConversationComponent`.

Pairing constraints:
- within 35 WU
- affinity with partner >= 0.10 (fallback: near-neutral allowed if no relationship entry)
- both cooldowns <= 0
- partner != last_partner unless no alternative in radius

Each entity may participate in at most one conversation per L2 tick.

### D3: Fragment generation
Each conversation emits one short fragment string using template buckets derived from existing state:
- `SCHEDULE`: transit/work/leisure/sleep transition pressure
- `MARKET`: nearby market scarcity or expensive buying pressure
- `DANGER`: recent SAW_VIOLENCE / infection severity / low health
- `STATUS`: social-rank/prestige flavor line

No external text generation model is used.

### D4: Side effects
For each participant:
- record `MemoryEventType::HEARD_RUMOR` with intensity 0.35-0.6 and source=partner
- mild PAD update (+/- small arousal/pleasure nudges by fragment type)
- set cooldown to 6-10 seconds (jitter) and `last_partner`

If player is within 70 WU of either participant, emit an intel alert with conversation text summary.

### D5: Alert integration
Extend `SimulationAlertCategory` with `INTEL`.
Conversation eavesdrop alerts use INFO severity and INTEL category.

### D6: Update order
Insert ConversationSystem in L2 after `relationshipSystem.update(...)` and before `rumorSystem.update(...)`, so rumor propagation can consume newly recorded HEARD_RUMOR memories naturally.

## Risks
- Chatter spam in dense crowds.
  - Mitigation: cooldowns, one-conversation-per-entity-per-tick, proximity/affinity gating.
- Repetitive lines.
  - Mitigation: bucketed templates + partner suppression via `last_partner`.
