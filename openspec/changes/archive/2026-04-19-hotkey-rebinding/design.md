## Context

`EquipmentComponent` already stores `hotkeys[0..9]` and the main input loop routes number presses through those slots. The missing piece is a player-facing way to update those mappings without waiting for the full inventory UI.

## Goals / Non-Goals

**Goals:**

- Let the player reassign numeric hotkeys `1..9` to the currently equipped item or scan tool.
- Keep `0` as the fixed empty equipment slot.
- Keep normal number presses as equip/use actions.
- Keep the behavior simple enough to test in the current counter-based inventory model.

**Non-Goals:**

- Add a full keybinding menu.
- Persist hotkey choices across launches.
- Add discrete inventory item stacks or drag/drop inventory controls.

## Decisions

- Use `Ctrl+1..9` as the rebinding chord. Plain number keys already mean equip/use, and `Shift+number` is layout-sensitive on keyboards where shifted numbers produce punctuation.
- Bind the target hotkey to `equipment.equipped`. This makes scan shortcuts, quick-use consumables, and `0` clearing work with one rule.
- Add a small helper in `equipment.h` for bounds-checked assignment so tests can verify the model without SDL event setup.

## Risks / Trade-offs

- Hotkeys reset on restart -> Acceptable for this slice because persistence is explicitly deferred in the TODO.
- There is no dedicated rebinding UI -> The full inventory UI is a later task; this exposes the behavior through an existing low-cost input path now.
