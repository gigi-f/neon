# Requirements

### Requirement: ItemComponent
Each collectible entity SHALL carry an `ItemComponent` specifying type (FOOD, WATER, MEDICAL)
and a `restore_value` (default 40.0). Items SHALL also carry `GlyphComponent` for rendering.

| Type    | Glyph | Color          |
|---------|-------|----------------|
| FOOD    | `f`   | amber (220,160,50) |
| WATER   | `w`   | cyan (60,180,255)  |

### Requirement: Ambient Item Spawning
`AmbientSpawnSystem::spawnItems()` SHALL scatter FOOD and WATER items on PEDESTRIAN_PATH
tiles within the camera radius, subject to a global cap of 60 items. Spawn probability:
1% chance per path tile per spawn cycle (same 2-second ambient timer as vehicles/citizens).

#### Scenario: Global Cap
- **WHEN** total item entities ≥ 60
- **THEN** `spawnItems()` SHALL return immediately without spawning

### Requirement: Citizen Need-Seeking Behavior
`ConsumableSystem::update()` SHALL run at the L2 tick gate (~2 Hz) and assign item goals
to hungry/thirsty citizens.

#### Scenario: Hunger Goal Assignment
- **WHEN** a citizen's `hunger < 40` AND they have no current item goal
- **THEN** their `GoalComponent.target_entity` SHALL be set to the nearest FOOD item

#### Scenario: Thirst Goal Assignment
- **WHEN** a citizen's `thirst < 40` AND they have no current item goal (and not hunger-critical)
- **THEN** their `GoalComponent.target_entity` SHALL be set to the nearest WATER item

#### Scenario: Item Consumption on Arrival
- **WHEN** a citizen with an item goal is within 20 WU of the target item
- **THEN** the appropriate bio stat SHALL be restored by `restore_value` (clamped to 100)
- **AND** the item entity SHALL be destroyed
- **AND** the citizen's `GoalComponent.target_entity` SHALL be reset to `MAX_ENTITIES`

### Requirement: Player Item Pickup
- **WHEN** the player presses E (interact) while NOT in a vehicle
- **AND** a FOOD, WATER, or MEDICAL item is within 30 WU
- **THEN** the nearest such item SHALL be consumed (stats restored, entity destroyed)
- **AND** the vehicle-boarding interaction SHALL be suppressed for that keypress

### Requirement: Infrastructure Unchanged
Road, transit, intersection, glyph rendering, and biology systems SHALL NOT be modified.
