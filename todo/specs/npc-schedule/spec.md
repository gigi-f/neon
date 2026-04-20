# Requirements

### Requirement: ScheduleComponent
Every citizen entity SHALL carry a `ScheduleComponent` with:
- `state`: `ScheduleState` enum (SLEEPING, WORKING, LEISURE, TRANSIT), default LEISURE
- `work_start` / `work_end`: float game-hour bounds (default 8.0 / 18.0)
- `sleep_start` / `sleep_end`: float game-hour bounds (default 22.0 / 6.0)

~30% of spawned citizens SHALL be assigned randomized schedules:
- Night workers (~15%): work_start=20.0, work_end=5.0, sleep_start=9.0, sleep_end=17.0
- Early birds (~15%): work_start=6.0, work_end=14.0

#### Scenario: Component Assignment
- WHEN a citizen is spawned by AmbientSpawnSystem
- THEN it SHALL receive a ScheduleComponent (mostly default; ~30% randomized)

### Requirement: ScheduleSystem
`ScheduleSystem::update()` SHALL run at the L2 tick gate (~2 Hz).
For each citizen with ScheduleComponent, compute the new state from game_hour:

| Condition                           | State    |
|-------------------------------------|----------|
| Within sleep window                 | SLEEPING |
| ±1.5h from work_start or work_end   | TRANSIT  |
| Within work window                  | WORKING  |
| Otherwise                           | LEISURE  |

Sleep window wraps midnight: if sleep_start > sleep_end,
the window spans [sleep_start, 24.0) ∪ [0.0, sleep_end).

### Requirement: GDISystem Integration
`GDISystem::update()` SHALL gate desire scoring based on ScheduleComponent:
- SLEEPING: skip scoring entirely — freeze intent to NONE, commitment=1.0
- TRANSIT: add +0.5 to transit_u (commuting citizens actively seek stations)
- WORKING: set transit_u = 0.0 (already at work, no commute desire)
- LEISURE: scoring unchanged

### Requirement: CitizenAISystem Integration
`CitizenAISystem::update()` SHALL respect schedule state:
- SLEEPING: set vx=vy=0, skip all wander/goal steering for this entity
- All other states: existing logic unchanged

### Requirement: Visual Cue
Citizens' GlyphComponent color SHALL reflect schedule state on transition:
- SLEEPING: dim blue-grey (r=80, g=90, b=120)
- WORKING:  white-yellow  (r=220, g=220, b=170)
- LEISURE:  cyan          (r=150, g=200, b=255)
- TRANSIT:  bright orange (r=255, g=170, b=60)

Color update occurs inside ScheduleSystem when state changes.
