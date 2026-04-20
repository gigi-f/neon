## Context

The existing loop has two separate input paths: `1/2/3` consume survival counters directly, while `i`, `Shift+i`, `c`, `f`, and `t` open scan panels. This keeps useful tools accessible, but it does not establish the inventory/equipment interaction the TODO calls for.

## Goals / Non-Goals

**Goals:**
- Introduce a single equipped slot for the player.
- Map default hotkeys to survival consumables and scan tools.
- Give scan tools per-tool ranges.
- Preserve existing scan shortcut keys while synchronizing them with the equipped slot.
- Keep the implementation compatible with the current counter-based inventory.

**Non-Goals:**
- Build the full inventory UI.
- Add user-editable hotkey rebinding.
- Add manual selection cursor movement.
- Add drop/inspect/discrete item stacks beyond the existing survival counters.

## Decisions

- Store the equipment component as a normal ECS component in `equipment.h`. It only needs the generic registry component storage and does not require deeper system changes.
- Keep hotkey defaults as an array indexed by numeric key. This makes the future rebind path straightforward without adding a settings UI now.
- Keep `1/2/3` as immediate quick-use actions for food, water, and medical supplies. They also equip the slot, so `E`/`Space` can repeat the active item.
- Keep legacy scan keys as shortcuts, but make them update the equipped slot and use `equipmentRange()`. This preserves current playability while moving scan actions onto the equipment model.

## Risks / Trade-offs

- Numeric scan hotkeys currently equip and immediately open the matching panel. That is fast for playtesting, but a future inventory UI may want select-only behavior.
- The full TODO remains open because manual target cursor, item rebinding UI, and discrete item inventory are not implemented in this slice.
