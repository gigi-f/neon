## Why
Conversations now generate intel events, but there is no diegetic on-world visual for speaking NPCs. The TODO's audibility direction calls for live glitch-text near speakers so the player can notice conversations before opening logs.

## What Changes
- Add per-NPC transient speech-bubble state for active conversation lines.
- Populate speech bubble text whenever ConversationSystem forms a pair.
- Render live glitch-text bubbles above speaking NPCs in the world.
- Scale text clarity with player distance (near = clearer, far = glitchier).

## Scope
In scope:
- World-space speech bubble rendering for active conversation lines.
- Deterministic glitch masking of characters for partial readability.

Out of scope:
- Full Dialogue Log UI and persistence.
- Dialogue trees or authored branching content.
