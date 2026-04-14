# Neon Oubliette — Master TODO

## Phase 1: Foundation (Complete)
- [x] Basic C++ / SDL2 Setup
- [x] Lightweight ECS implementation
- [x] Basic Playable Area with Grid
- [x] Player Movement (WASD)
- [x] Basic AABB Collision & Obstacles
- [x] Asset pipeline preparation (stb_image, Player PNG)
- [x] Art Direction & Asset List documented

## Phase 2: World Navigation & Camera (Complete)
- [x] Implement Camera System to follow the player
- [x] Render world relative to camera position
- [x] Expand the playable grid area to be larger than the screen
- [x] Layered rendering for depth (ground vs elevated)

## Phase 3: Infrastructure & Stateful Vehicles (Complete)
- [x] Define Road segments and rendering (Primary, Secondary, Alleys)
- [x] Implement Stateful Vehicles (owner, home location, type)
- [x] Implement Vehicle Interaction (Enter/Exit vehicles, player driving)
- [x] Vehicle movement logic (Player controlled vs basic velocity)
- [x] Implement simple Traffic Lights (RED/YELLOW/GREEN cycles)

## Phase 4: City Generation & Urban Planning (Complete)
- [x] Procedural block and lot subdivision (Chicago-style)
- [x] Procedural building placement and rendering
- [x] Distance-based building height gradients
- [x] Unique building IDs (64-bit stable IDs)
- [x] Super-block building clusters (zero-gap adjacencies)
- [x] Pedestrian alleys and narrow paths
- [x] Scale separation for transit infrastructure (Mag-tracks)

---

## Phase 5: ECS Infrastructure & Scale (Current — Blockers)
*Must be done before Phase 6+ work. Without these, entity counts will stall performance.*

- [x] **Entity Deletion** (`ecs.h`)
    - [x] Add `virtual remove(Entity)` to `IComponentArray` interface
    - [x] Add `free_list` + `live` set to `Registry`
    - [x] Implement `Registry::destroy(Entity)` — removes from all arrays, recycles ID
    - [x] Modify `Registry::create()` to pull from free list first
    - [x] `Registry::alive(Entity)` and `entity_count()` helpers
- [x] **View Optimization** (`ecs.h`)
    - [x] Expose `size()` and `keys()` on `ComponentArray<T>` via `IComponentArray`
    - [x] Rewrite `view<ComponentTypes...>()` to iterate smallest component set
- [x] **Spatial Hash Grid for Collision** (`spatial_grid.h`, `simulation_systems.h`)
    - [x] `SpatialGrid` with cell size = `macro_cell_size` (40 WU)
    - [x] `rebuild()` from SolidComponent entities at start of each `MovementSystem::update()`
    - [x] `checkCollision()` queries grid candidates, then does exact AABB check
- [x] **Simulation Coordinator** (`simulation_coordinator.h`, `main.cpp`)
    - [x] Tick-rate gates: L0 (60 Hz), L2 (2 Hz), L3 (1 Hz), L4 (0.2 Hz)
    - [x] `coordinator.advance()` in game loop; citizens gated at L2
- [x] **Debug HUD** (`main.cpp`)
    - [x] FPS, entity count, camera position, frame number via window title (updates at 1 Hz)
- [x] **Extract World Bounds** (`components.h`, `simulation_systems.h`)
    - [x] Add `world_min` / `world_max` fields to `WorldConfigComponent`
    - [x] Replace hardcoded `-1000 to 1000` in `simulation_systems.h` world boundary clamp

---

## Phase 6: Ambient Traffic & Transit Systems
- [ ] **Ambient Traffic System**
    - [x] Real-time vehicle dimensions and rigid body collision (EMMVs, Transports with SolidComponent)
    - [x] Wire `despawnVehicles()` now that entity deletion exists
    - [x] Camera-radius spawn/despawn with configurable radius
    - [x] Intersection turn queuing (FIFO logic for unprotected intersections)
    - [x] Stop-sign FIFO intersection management system
    - [x] Traffic density propagation along road segments
- [ ] **Transit & Elevated Rails**
    - [ ] Automated transit vehicle schedules and station stops
    - [ ] Player boarding/disembarking from automated transit
    - [ ] Magnetic track switching logic for Maglifts
    - [ ] Maglift Monorail pathfinding along track graph
- [ ] **Pedestrian Flow**
    - [ ] NPC citizen spawning on sidewalks/pedestrian paths
    - [ ] Simple goal-based walking (wandering + nearest destination)
    - [ ] Stop clustering for public transit waits
    - [ ] Sidewalk congestion and flow avoidance
- [ ] **Time of Day**
    - [ ] Basic DAWN/DAY/DUSK/NIGHT cycle
    - [ ] Visual color shift per time state (ambient tint)
    - [ ] Spawn rate modulation by time of day (vehicles, pedestrians)

---

## Phase 7: Deep Simulation Layers (L0 – L4)

### L0: Physics & Environment
- [ ] Cell-based temperature simulation (heat island near URBAN_CORE)
- [ ] Pressure and ventilation modeling (interior vs. exterior zones)
- [ ] Structural integrity and material decay rates
- [ ] Acid rain acceleration of structural decay
- [ ] Power grid simulation (Nodes, conduits, arterial supply lines)
- [ ] Environmental hazard events (flooding placeholder, smog, acid rain)

### L1: Biology & Metabolism
- [ ] `BiologyComponent` — health, hunger, thirst, fatigue fields
- [ ] NPC metabolic decay over time (hunger/thirst)
- [ ] Injury statuses (Internal Bleeding, Broken Limb, etc.)
- [ ] Organ health and vital signs
- [ ] Pathogen/infection propagation via proximity and shared spaces
- [ ] Pathogen upward cascade to L2 (trauma, mood impact)
- [ ] FOOD, WATER, MEDICAL item categories with L1 status effects
- [ ] Inventory UI for survival items

### L2: Cognitive & Social (BDI)
- [ ] `CognitiveComponent` — PAD axes (Pleasure, Arousal, Dominance)
- [ ] BDI memory ring buffer (fixed `std::array<MemoryRecord, 16>` per NPC)
- [ ] Goal-Desire-Intention agent architecture
- [ ] NPC memory formation from witnessed events
- [ ] Relationship graph between agents (affinity, trust)
- [ ] Social hierarchy positioning
- [ ] Emotional state influence on behavior choices
- [ ] NPC schedule state machine: SLEEPING / WORKING / LEISURE / TRANSIT
- [ ] Witness trauma propagation to other NPCs (rumor seeding)

### L3: Economic & Market
- [ ] `EconomicComponent` — credits, employer entity, daily wage
- [ ] Individual NPC wallets and credit balances
- [ ] Local supply/demand markets per district
- [ ] Item provenance tracking (high-value goods only)
- [ ] Supply chain flows: Extraction → Production → Market
- [ ] Property ownership and rent/mortgage logic
- [ ] Wage payment loop (L4 employer directives → L3 credits)

### L4: Political & Factional
- [ ] Territorial ownership maps (Ownership Lines per zone)
- [ ] Faction Directive system — zone-wide policy changes
- [ ] Reputation system: personal standing (-100 to +100) per faction
- [ ] Wanted level (0–5 star escalating guard response)
- [ ] Faction patrol routes driven by directives
- [ ] Political directives cascade to L3 (economic foreknowledge)
- [ ] Level 1 Sedition trigger (ignoring directives in restricted zones)

---

## Phase 8: Simulation Coherence — Sparse-Set Storage
*Conditional: profile first after Phase 7 NPC work reveals cache pressure.*
- [ ] Replace `unordered_map<Entity, T>` in `ComponentArray<T>` with sparse-set (dense array + sparse index)
- [ ] Verify all insert/get/has/remove operations maintain O(1)
- [ ] Benchmark `view<CognitiveComponent>()` before/after
- [ ] Ensure entity deletion still works correctly with new storage

---

## Phase 9: Interaction & Gameplay Systems

### Dialogue & Information
- [ ] Audibility model — Glitch-Text HUD (missing letters resolved by proximity)
- [ ] NPC eavesdropping — real-time dialogue reveals free intelligence/lore
- [ ] Hearing augments (Tier 1–3, passive vs. neural drain)
- [ ] Neural-Audio Recorder (fragment capture)
- [ ] Grammar-based dialogue engine (6 speech profiles)
- [ ] Ink scripting integration for Gold Path (Leaders, Fixers)
- [ ] Information type system: RUMOR, PROPAGANDA, INTELLIGENCE, PRICE_TIP
- [ ] Organic information propagation (NPC-to-NPC rumor spread)
- [ ] Knowledge trading and secrets (information as currency)

### Trade & Barter
- [ ] Leverage-and-greed trade model
- [ ] Inventory management (Pick up, Drop, Use items)
- [ ] NPC greed margins and social pressure mechanics

### Crime & Law Enforcement
- [ ] Simulated crime economy (Theft, Mugging, Contraband)
- [ ] Guard NPC patrol and response system (CrimeReportEvents)
- [ ] Pursuit and arrest task chain
- [ ] Corruption: bribery, favors, backroom deals
- [ ] Clandestine drug labs with raid mechanics
- [ ] Notoriety/Wanted level visuals and escalation

### Urban Decay
- [ ] Structural health degradation over time (L0 cascade)
- [ ] Graffiti, waste accumulation, abandoned zone state
- [ ] Squatting and rebuilding systems

---

## Phase 10: Lore & Narrative Systems

### Player: The Engineer's Legacy
- [ ] Engineer's Diary lore item (found at spawn point)
- [ ] Five investigative "Legacy Firmware" tools — initialized at game start (Tier 1 / Baseline)
- [ ] Neural Battery resource meter and Signal Heat detection risk
- [ ] Tool signature visibility increase on repeated death (passive Heat ramp)
- [ ] Neural Scrub action to reset Heat signature

### The Death Cascade
- [ ] L0: Corpse entity spawned; battery capacity permanent reduction (5%) if high-tier tools equipped
- [ ] L1: Respawn in random Slum Squat / Safe House at 10% health
- [ ] L2: 25% of accumulated information (Rumors/Intelligence) lost on death
- [ ] L3: All physical items + 50% on-hand credits dropped as lootable corpse entity
- [ ] L4: Wanted level spike if died in restricted zone; increased tool Heat
- [ ] ReclamationDrones — retrieve corpses and transport to CREMATORIUM
- [ ] Scavenger Thieves prioritize lootable corpse entities

### Tool Masteries
- [ ] Tier 1 (Legacy Firmware) — all five tools functional at baseline
- [ ] Tier 2 (Enhanced Diagnostics) — deeper layer reads, wider scan radius
- [ ] Tier 3 (Direct Actions) — ability to influence simulation state, not just observe
- [ ] Firmware module upgrades (purchasable, lootable)

---

## Phase 11: World Scaling & City Generation

### Full City
- [ ] Full 256×256 WU procedural generation with all 14 zone types
- [ ] Chicago-style height gradients per zone
- [ ] Zone type: AERODROME (VTOL, hover vessel platforms)
- [ ] Zone type: TRANSIT HUB
- [ ] Zone type: CREMATORIUM district

### Interiors
- [ ] High-fidelity interior generation with architectural adjacency rules
- [ ] Functional prop sets (Desks, Racks, Containers, Terminals)
- [ ] Enterable building state (door interaction)

### Vertical Traversal
- [ ] Staircase / elevator entities with Z-axis transition
- [ ] Vertical layer system for multi-floor buildings

### Futuristic Infrastructure
- [ ] VTOL Hover Vessels at Aerodrome zones
- [ ] EMMV Auto-Pods (driverless)
- [ ] Maglift Monorail graph with station stops
- [ ] Maglift switching logic

---

## Phase 12: Factions, Xenos & AGI

### Factions
- [ ] Five faction entities (AGI-aligned) with territory maps
- [ ] Faction aesthetic differentiation in rendering
- [ ] Faction leader NPCs (Gold Path Ink scripted dialogue)
- [ ] Faction standing effects on shop prices, guard behavior

### Xeno System
- [ ] Archon xeno type — behavior + radius-based layer warping
- [ ] Aborrax xeno type
- [ ] Behedicci xeno type
- [ ] Post-Human xeno type
- [ ] Life stage system (7 stages affecting xeno stats/behavior)

### AGI Cores
- [ ] Five AGI Throne Room locations (one per faction territory)
- [ ] 5-layer security gauntlet per Throne Room
- [ ] AGI encounter scripted via Ink

---

## Phase 13: Crisis & Emergence
- [ ] Crisis type: Collapse (structural cascade)
- [ ] Crisis type: Outbreak (L1 pathogen mass spread)
- [ ] Crisis type: Market Crash (L3 economic failure)
- [ ] Crisis type: Coup (L4 faction power shift)
- [ ] Crisis type: Power Blackout (L0 grid failure)
- [ ] Crisis type: Information War (L2/L4 propaganda flood)
- [ ] Crisis type: Xenomorphic Bloom (xeno population spike)
- [ ] Cross-layer cascade logic for each crisis type
- [ ] Crisis Dashboard (`V`) — city-wide threat monitoring
- [ ] Directive Markets — economic foreknowledge from scouted laws

---

## Phase 14: Interface & Meta Systems

### God Mode (`G`)
- [ ] Detached camera cursor for free world exploration
- [ ] Entity inspection at any distance
- [ ] Simulation speed control (+/-)
- [ ] Layer visibility toggle (show/hide L0-L4 overlays)

### Inspection Panels
- [ ] Surface Scan (`i`) — basic info (name, zone, entity type)
- [ ] Biological Audit (`I`) — vitals, needs, injury status
- [ ] Cognitive Profile (`c`) — current goal, emotional state, memory summary
- [ ] Financial Forensics (`f`) — credit balance, employer, transaction history
- [ ] Structural Analysis (`t`) — building health, material, decay rate

### History & Logs
- [ ] Dialogue Log (`L`) — recent overheard conversations
- [ ] Intel Log (`n`) — player's personal history and milestones
- [ ] Crisis Dashboard (`V`) — city-wide threat monitoring

### The Routine (Gameplay Loop Anchor)
- [ ] Box Sorting mini-game (intro directive task)
- [ ] L4 Political risk trigger for ignoring directives (Level 1 Sedition)

### Persistence
- [ ] Binary serialization using Cereal
- [ ] Save/Load world state and entity registry
- [ ] Entity free-list and destroyed-set persisted correctly

---

## Phase 15: The Sanctuary
- [ ] Claimable player housing (Squat → Safe House → Sanctuary tiers)
- [ ] 5-layer defenses: Jammers, DNA-locks, guard NPCs, false walls, L4 shell companies
- [ ] Housing Broker NPC interaction
- [ ] Storage for items/equipment across deaths

---

## Phase 16: Endgame Scenarios
- [ ] **Purge** — eliminate the five AGIs; city devolves to chaos
- [ ] **Avatar** — merge with the dominant AGI; become the new Oubliette god
- [ ] **Synthesis** — broker peace between factions via the five Throne Rooms
- [ ] **Reset** — find mother's Root Code and wipe the AGI substrate

---

## Phase 17: Polish & Aesthetics
- [ ] Day/Night cycle with dynamic lighting (street lights, neon signs)
- [ ] Weather systems (acid rain, smog, fog)
- [ ] Rich ASCII/glyph-based entity representation per entity type
- [ ] Interactive feedback (screen shake, sound effects, particles)
- [ ] Open-source thin-stroke icon library integration (Lucide / Phosphor) for HUD diagnostics
- [ ] "Neon Noir" color palette enforced across all rendering (#0A0A0F background)
