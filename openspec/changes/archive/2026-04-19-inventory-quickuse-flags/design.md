## Context

The current survival counters are still useful for HUD compatibility, but they are no longer enough to represent concrete items. The previous change introduced discrete bag slots and kept counters in sync during pickup, use, and drop.

## Decision

Add `useFirstInventoryItemOfType`, which scans occupied bag slots for the requested consumable type, selects that slot, applies the same item effect as manual item use, clears the slot, and decrements the matching counter. `main.cpp` will call this helper first for equipped consumable hotkeys and fall back to `ConsumableSystem::consumePlayerInventory` only when no matching discrete slot exists.

World `ItemComponent` receives a `flags` field. Pickup copies `ItemComponent::flags` to `CarriedItem::flags`; dropping copies flags back to the world item. Surface Scan and Financial Forensics render compact flag labels using shared helpers from `inventory.h`.

## Non-Goals

- No full inventory grid UI.
- No ownership/provenance tracking.
- No barter or market transaction UI.

## Risks

- Counter fallback can still consume a legacy counter without a discrete item. This is intentional to avoid breaking old initialization paths during the migration.
