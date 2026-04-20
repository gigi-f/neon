## Context

`ItemComponent` and `CarriedItem` already preserve type, restore value, flags, and a source entity. Item flags can identify illegal, unique, high-value, and faction-relevant items, and scan panels already display those flags. Market buy/sell uses helper functions that create or remove inventory entries without a full item-entity inventory.

## Goals / Non-Goals

**Goals:**

- Track provenance only when an item is illegal/stolen, unique, faction-relevant, quest-like, or high-value.
- Keep common FOOD/WATER/MEDICAL commodities untracked unless flagged.
- Make provenance visible in scan panels and inventory inspection data.
- Preserve provenance through pickup/drop and market purchase/sale helpers.

**Non-Goals:**

- No full legal system, evidence inventory, ownership registry, or faction reputation effects.
- No persistence format changes.
- No multi-item market inventory; market stock remains aggregate counts.

## Decisions

- Add an embedded `ItemProvenance` aggregate to `ItemComponent` and `CarriedItem`.
  - Rationale: the project uses simple POD-like ECS components and aggregate initialization. Embedding avoids registry lookups and keeps inventory copy/drop paths straightforward.

- Add `ITEM_FLAG_QUEST` as the quest-like tracking signal.
  - Rationale: the TODO names quest goods, but no existing flag represents them. This keeps quest provenance opt-in.

- Track provenance when flags include illegal, unique, high-value, faction-relevant, quest, or when `provenance.stolen` is already true.
  - Rationale: this matches the TODO while treating the current faction-relevant flag as the closest existing quest-adjacent category.

- Mark picked-up tracked items as stolen when they have a live owner different from the player.
  - Rationale: this creates a concrete first stolen-goods path without implementing guards or crime reporting yet.

- Use compact scan text: `PROV:...`, `OWNER:...`, `SOURCE:...`.
  - Rationale: scan panels have limited height, so provenance must fit next to existing flags.

## Risks / Trade-offs

- Entity IDs are volatile and not persistence-safe. This is acceptable until the save surface stabilizes.
- Market sale removes the item from player inventory, so provenance is not stored in market stock yet. The sale path still preserves/recomputes provenance before removal for tests and future hooks.
