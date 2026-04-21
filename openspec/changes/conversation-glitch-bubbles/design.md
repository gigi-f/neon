## Design

### D1: Transient speech component
Add `SpeechBubbleComponent` to `components.h`:
- `text` (current line)
- `ttl` (seconds visible)

Component is assigned lazily by ConversationSystem when a line is emitted.

### D2: Conversation integration
When a conversation pair is accepted:
- assign/update `SpeechBubbleComponent` on both participants
- set shared text fragment and TTL (2-3s)

Speech TTL decays at L2 cadence inside ConversationSystem, so bubbles automatically expire.

### D3: Glitch rendering
Add world render helper in `main.cpp`:
- iterate entities with `SpeechBubbleComponent + TransformComponent`
- skip expired bubbles
- compute distance from player
- derive clarity factor from distance (close=clear, far=masked)
- apply deterministic per-frame character masking to generate glitch text
- draw text just above entity in screen space

### D4: Draw order
Render speech bubbles after entities and before overlays/panels so they are visible in play without obscuring HUD.

## Risks
- Text noise/clutter in dense crowds.
  - Mitigation: short TTL, per-entity single line only, distance-based readability.
