## Context

`directionalScanTarget()` previously moved from the current target toward another entity in the pressed direction, but it returned the current target when no candidate existed. Arrow keys also depended on an active scan panel, so they were not usable as pre-fire focus control after scan shortcuts became equip-only.

## Goals / Non-Goals

**Goals:**

- Let arrow keys select focus before `Space` fires.
- Keep the lock attached to the entity while it remains alive and in tool range.
- Clear focus when directional cycling has no alternative target.

**Non-Goals:**

- Free-position cursor movement.
- Mouse targeting or path previews.
- Persistent target memory across equipped tool changes.

## Decisions

- Keep target focus as an entity id. This makes following moving targets free because the renderer and scan panel read the target entity's current transform every frame.
- Add explicit range validation helpers in `target_selection.h`. This keeps the lock lifecycle testable and prevents stale target ids from staying selected after range breaks.
- Make arrow keys use the equipped scan tool when no panel is open. This preserves `Space` as the fire action while allowing directional focus first.

## Risks / Trade-offs

- If no directional candidate exists among multiple in-range entities, focus clears instead of staying put -> This matches the requested "switch or clear" model and avoids false confidence about what arrow input did.
- Target focus can clear while the player moves away -> This is intentional; the target has left the active tool range.
