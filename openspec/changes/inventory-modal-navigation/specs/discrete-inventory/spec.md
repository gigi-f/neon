## ADDED Requirements

### Requirement: Inventory modal overview
The player SHALL be able to open a navigable inventory modal that lists every discrete inventory slot, highlights the current selection cursor, and displays icon/name details for occupied slots.

#### Scenario: Open inventory modal
- **WHEN** the player opens the inventory modal while carrying discrete inventory
- **THEN** the UI SHALL show all inventory slots in a modal layout
- **AND** the currently selected slot SHALL be visually highlighted

#### Scenario: Empty slot is shown clearly
- **WHEN** a slot is unoccupied in the modal
- **THEN** the slot SHALL be labeled as empty

---

### Requirement: Inventory modal hotkey visibility
The inventory modal SHALL show the assigned hotkey number for each occupied slot whose item type is currently bound in equipment hotkeys.

#### Scenario: Item has assigned hotkey
- **WHEN** a slot contains an item type mapped to an equipment hotkey
- **THEN** the modal SHALL display that hotkey number adjacent to the slot item details

#### Scenario: Item has no assigned hotkey
- **WHEN** a slot contains an item type with no mapped equipment hotkey
- **THEN** the modal SHALL show that no hotkey is assigned

---

### Requirement: Modal cursor navigation
While the inventory modal is open, directional input SHALL move the discrete inventory selection cursor across slots so use/equip/drop actions target the highlighted slot.

#### Scenario: Move cursor with directional keys
- **WHEN** the player presses directional keys with the modal open
- **THEN** the selected inventory slot SHALL move to the adjacent slot in that direction

#### Scenario: Use selected slot from modal
- **WHEN** the player triggers use/equip on the highlighted slot while the modal is open
- **THEN** existing selected-slot use/equip behavior SHALL execute for that slot
