## Context

`EquipmentComponent` already tracks the current equipped slot and numeric hotkey assignments. `DiscreteInventoryComponent` currently stores only `ItemComponent::FOOD`, `WATER`, and `MEDICAL`, so scan tools still behave like global actions even when the HUD calls them equipment.

## Goals / Non-Goals

**Goals:**

- Make scan tools occupy carried inventory slots.
- Keep the current hotkey and equipped-use controls working.
- Keep consumable quick-use behavior unchanged.
- Allow the player to equip the selected inventory tool before using it.
- Make `Space` the explicit action key for firing an equipped scan tool.
- Show scan range before firing and keep arrow keys reserved for cursor/focus.

**Non-Goals:**

- Full inventory UI, persistence, tool durability, or drag-and-drop rebinding.
- Removing direct legacy scan keys entirely; they remain compatibility shortcuts that select matching carried tools.
- Tool pickup balance beyond the baseline spawn loadout.

## Decisions

- Extend `ItemComponent::Type` with scan tool item types. This keeps world items, carried entries, inspection, pickup, and drop on one existing path instead of adding a parallel inventory item model.
- Add explicit mapping helpers between `EquipmentSlot` and `ItemComponent::Type`. This avoids ad hoc switch logic in input handling and lets tests cover the inventory/equipment contract.
- Gate scan use on inventory ownership. Numeric hotkeys can still select configured equipment, but a scan fails with denied feedback when the matching tool is not carried.
- Seed the baseline tools directly into the player's inventory. This satisfies the current TODO without requiring tool world placement or a shop flow before trade exists.

## Risks / Trade-offs

- Adding enum values broadens code that previously assumed three consumables -> Mitigated by adding helper predicates and default non-consumable behavior.
- Direct scan keys still exist -> Mitigated by making them equip-only shortcuts routed through the same carried-tool check.
- Baseline tools consume five bag slots -> Acceptable for this slice because it makes the inventory/equipment relationship visible immediately.
