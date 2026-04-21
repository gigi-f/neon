## Context

`DiscreteInventoryComponent` already stores 12 slots with a `selected` cursor index, and gameplay already supports using/equipping or dropping the selected item. The missing layer is UI affordance: players cannot see all carried items or hotkey mapping in one place.

## Goals / Non-Goals

**Goals:**

- Provide a toggleable modal that displays all inventory slots and cursor state.
- Keep navigation deterministic and keyboard-first using existing arrow key input.
- Surface hotkey assignment per item without introducing new equipment storage.
- Reuse existing use/equip/drop actions when modal is open.

**Non-Goals:**

- No drag-and-drop or mouse inventory interactions.
- No stack splitting, sorting, filtering, or paging.
- No persistence/schema changes for inventory data.

## Decisions

- Use `TAB` to open/close the inventory modal.
  - Rationale: avoids conflict with existing scan/equipment bindings and gives a predictable toggle.

- Reuse `DiscreteInventoryComponent::selected` as the modal cursor state.
  - Rationale: keeps behavior consistent with existing use/drop functions and avoids duplicate selection state.

- Add `hotkeyIndexForInventoryType(...)` helper in `equipment.h` for lookup.
  - Rationale: hotkey resolution should stay close to equipment mapping logic and be unit-tested independently.

- While modal is open, arrow keys navigate the modal cursor and do not move scan target focus.
  - Rationale: avoids ambiguous dual-use input and keeps UI interaction explicit.

## Risks / Trade-offs

- Modal consumes arrow keys, so scan-target cycling is temporarily blocked while open. This is intentional for clear control focus.
- Short labels are used inside fixed-size slots; long item names may be visually clipped at small font scale.
