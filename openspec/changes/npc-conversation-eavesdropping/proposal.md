## Why
The simulation already has social rank, relationships, schedules, market state, and rumor memory records, but NPCs do not currently perform explicit conversation exchanges. This blocks the information-gameplay loop because there is no concrete in-world conversational source for overheard fragments.

## What Changes
- Add an L2 `ConversationSystem` that selects nearby NPC pairs and produces deterministic conversation fragments from existing simulation state (schedule, market pressure, danger, social rank).
- Add compact runtime conversation state on NPCs (cooldown + last partner) to avoid spam and ping-pong repetition.
- Record conversation outcomes into cognitive memory as `HEARD_RUMOR` events for participants.
- Emit eavesdroppable intel snippets to the alert/intel pipeline when the player is near a conversation.
- Add deterministic tests for pair gating, cooldown behavior, and eavesdrop emission.

## Scope
In scope:
- NPC↔NPC conversation turn generation only (no full dialogue tree, no Ink integration).
- Text fragments are template-based and generated from existing world state.
- Intel exposure is ephemeral and uses existing HUD/log plumbing.

Out of scope:
- Dialogue Log UI (`L`) and persistent Intel Log database.
- Knowledge trading/barter of information.
- Faction leader scripted dialogue.
