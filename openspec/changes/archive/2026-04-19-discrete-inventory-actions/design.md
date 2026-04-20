## Context

`SurvivalInventoryComponent` currently exposes common consumables as counters, which keeps the HUD and quick-use hotkeys simple. The next inventory work should not remove those counters yet because the game already depends on them for visible survival feedback and equipped consumable behavior.

## Decision

Add `DiscreteInventoryComponent` alongside the existing counters. Each occupied slot stores item type, restore value, and basic item flags. Pickup can add an item record and still increment the matching survival counter so current HUD and hotkeys remain valid during the transition.

The helper layer in `inventory.h` owns deterministic inventory behavior:

- Find the nearest item within a pickup radius and store it in the first empty slot.
- Inspect the selected occupied slot as a small POD summary.
- Drop the selected item back into the world near the player.
- Use the selected item by applying the same biology effects as survival consumables and clearing that slot.

## Non-Goals

- No full inventory UI in this change.
- No barter, ownership, stolen-state, or faction provenance behavior.
- No persistence format changes.

## Risks

- Counters and discrete slots can temporarily represent the same pickup. This is intentional for compatibility, but future UI work should make counters a derived display or migrate quick-use consumption to discrete slots.
