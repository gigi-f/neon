# Neon Oubliette - Triaged TODO

This backlog is ordered by game-development dependency, not by simulation layer. The current goal is to turn the existing city simulation into a readable playable loop: scan, learn, trade or steal, trigger consequences, and survive.

Audit source: `todo/TODO_AUDIT_2026-04-18.md`.

## Milestone 0 - Sandbox Microcity Pivot

Priority: immediate. Replace broad procedural generation with a deterministic sandbox map that is a microcosm of the future city.

- [ ] Remove procedural city generation and replace it with a hand-authored sandbox layout profile
- [ ] Include required districts in the sandbox:
  - [x] Housing
  - [ ] Workplace
  - [ ] Market
  - [ ] Leisure area
  - [ ] Upper-class quarters
- [ ] Connect all required districts with transit so NPC routing is deterministic and observable
- [ ] Add startup validation (tests and/or assertions) that required districts and transit reachability exist
- [ ] Next phase: implement NPC routine loop on sandbox map
  - [ ] `sleep => transit => work => transit => leisure => home => repeat`
- [ ] After loop validation, tune player role interactions against this sandbox flow

---

## Completed Foundation

- [x] C++ / SDL2 project setup
- [x] Lightweight ECS with entity deletion and optimized views
- [x] Spatial hash grid collision
- [x] Simulation coordinator with staggered L0-L4 tick rates
- [x] Camera-following world renderer
- [x] Chicago-style procedural city blocks, roads, alleys, sidewalks, buildings
- [x] Traffic lights, stop-sign FIFO intersections, ambient vehicles, traffic density
- [x] Automated maglift transit with stations, boarding, stops, and terminal ping-pong
- [x] Pedestrian spawning, goal movement, transit waits, congestion avoidance
- [x] Day/night phases, weather states, ambient tint, spawn-rate modulation
- [x] ASCII/glyph renderer and on-screen debug/survival HUD
- [x] L0 environment: temperature, atmosphere, structural decay, power grid, hazards
- [x] L1 biology: needs, vitals, injuries, pathogens, survival items, inventory counters
- [x] L2 cognition/social: PAD, memory ring, GDI goals, relationships, schedules, social rank, rumors
- [x] L3 baseline economy: wallets, employers, wages, local markets

---

## Milestone 1 - Make The Simulation Visible

Priority: highest. Most existing systems are currently too invisible to drive player decisions.

### Inspection Panels

- [x] Surface Scan (`i`)
  - [x] Select nearest entity near the player
  - [x] Show entity id, type, glyph, distance, position, zone, and core tags
  - [x] Make the selected entity visually obvious while the panel is open
  - [x] Improve targeting to prefer entities in the player's facing direction
- [x] Biological Audit (`Shift+i`)
  - [x] Show health, hunger, thirst, fatigue, heart rate, oxygen, infection state, injury summary
  - [x] Fall back clearly when the target has no biological data
- [x] Cognitive Profile (`c`)
  - [x] Show PAD values, current desire/goal, schedule state, social rank, recent memory count
  - [x] Fall back clearly when the target has no cognitive data
- [x] Financial Forensics (`f`)
  - [x] Show wallet, employer, wage, market stock/prices, and power/economic tags
  - [x] Fall back clearly when the target has no financial data
- [x] Structural Analysis (`t`)
  - [x] Show building id, floors, material, integrity, collapse state, power supply/demand/powered state
  - [x] Fall back clearly when the target has no structural data

### Layer Readability

- [x] Layer visibility toggle
  - [x] Cycle overlays for Surface, L0 Environment, L1 Biology, L2 Social, L3 Economy, L4 Faction
  - [x] Use cheap glyph/color overlays only; no expensive full-screen per-pixel effects
- [x] Existing-system alerts
  - [x] On-screen notification when weather changes, flooding toggles, infection worsens, or a building collapses
  - [x] Keep a short recent-event stack for later Intel Log integration
- [x] Visual feedback polish
  - [x] Small screen shake or flash for impacts/collisions/hazard damage
  - [x] Simple sound hooks for pickup, consume, scan, warning, and denied action

---

## Milestone 2 - Build The Core Playable Loop

Priority: high. These turn visibility into decisions and consequences.

- [x] Tools are currently triggered by a key press, but should be equipped from the inventory (ie move cerebral scan to main equipped item)
  - [x] Items need a "range" depending on what they are.
  - [x] Players can use arrow keys to move their selection cursor to different entities before triggering an action.
  - [x] Ability to set hotkeys for items (e.g. 1-9 for items, 0 for nothing equipped)
  - [x] Items like water and food can be triggered directly from the inventory or set to a hotkey
  - [x] See planning documentation for more info.

### Inventory, Trade, And Items

- [ ] Inventory management beyond survival counters
  - [x] Pick up, drop, inspect, and use discrete items [requires inventory UI]
  - [x] Preserve current food/water/medical hotkeys as quick-use actions
  - [x] Add item tags for legal/illegal, unique/high-value, and faction relevance
- [x] Trade UI and simple barter
  - [x] Buy/sell at market entities
  - [x] Price affected by scarcity, reputation, and simple greed margin
  - [x] Expose trade result clearly before confirmation
- [x] High-value item provenance
  - [x] Track ownership/source only for stolen, unique, quest, or high-value goods
  - [x] Expose provenance through Surface Scan and Financial Forensics
- [x] Move tools to 1-5 keys, 6-9 for items, 0 for nothing equipped
- [x] We need a navigable inventory modal that shows icons of the items, a cursor, and the hotkey number next to the relevant item (if assigned)
- [ ] npcs should not spawn randomly, but should be born in housing units or at hospital

### Information Gameplay

- [ ] Audibility model - Glitch-Text HUD
  - [ ] Nearby NPC speech appears as partial text resolved by distance
  - [ ] Important fragments are color-coded by information type
  - [ ] Scope to active-region NPCs only
- [ ] NPC eavesdropping
  - [ ] Generate useful overheard fragments from schedule, market, rumor, faction, or danger state
  - [ ] Let overheard information seed the Intel Log
- [ ] Information type system
  - [ ] `RUMOR`, `PROPAGANDA`, `INTELLIGENCE`, `PRICE_TIP`
  - [ ] Store source, confidence, timestamp, and optional target entity/zone
- [ ] Dialogue Log (`L`)
  - [ ] Show recent overheard lines and captured fragments
- [ ] Intel Log (`n`)
  - [ ] Show stable discoveries, price tips, faction warnings, and player milestones
- [ ] Neural-Audio Recorder
  - [ ] Capture selected fragments into the Dialogue Log/Intel Log
  - [ ] Avoid recording every ambient line

### Housing And Interiors

- [ ] Enterable buildings with visible interiors and exits
  - [ ] Current `InteriorComponent` toggle exists; add actual interior view/play space
- [ ] Lazy template-based interiors
  - [ ] Generate only when entered
  - [ ] Use templates plus small random variation
- [ ] Functional prop set
  - [ ] Containers, terminals, beds, doors, storage
- [ ] Claimable housing
  - [ ] Squat -> Safe House -> Sanctuary tiers
- [ ] Housing broker NPC
  - [ ] Defer until dialogue/trade exists
- [ ] Basic staircase/elevator floor transitions
  - [ ] Use discrete floor index before any full vertical simulation

### Factions, Law, And Risk

- [ ] Faction territories
  - [ ] Start with 2-3 factions if five is too noisy
  - [ ] Show borders via overlay or district coloring
- [ ] Faction aesthetic differentiation
  - [ ] Distinct glyph colors/patrol markers/territory colors
- [ ] Reputation system
  - [ ] Personal standing per faction
  - [ ] Visible reputation deltas after crimes, trades, faction help, and restricted-zone behavior
- [ ] Wanted level
  - [ ] 0-5 escalating risk state
  - [ ] HUD-visible warning and escalation feedback
  - [ ] Merge notoriety visuals here
- [ ] Guard patrol and response
  - [ ] Active-region waypoint patrols
  - [ ] Respond to visible crimes and sedition triggers
  - [ ] Avoid city-wide continuous pathfinding
- [ ] Minimal pursuit/arrest loop
  - [ ] Chase, lose line/range, arrest/capture consequence
- [ ] Level 1 Sedition trigger
  - [ ] Ignoring directives in restricted zones raises warning, reputation loss, or wanted level
  - [ ] Single canonical TODO; no duplicate routine item
- [ ] Faction directives
  - [ ] Start with 3-5 directive types affecting patrols, access, market prices, or wages
  - [ ] Make L3 cascade visible in Financial Forensics and Intel Log

### Intro Routine

- [ ] Box Sorting mini-game only if it teaches core systems
  - [ ] Teaches scan, inventory, directive compliance, and sedition risk
  - [ ] Cut or replace if it becomes isolated busywork

---

## Milestone 3 - Consequences And Player Persistence

Priority: medium. Add once the core loop can produce meaningful losses and recoveries.

### Engineer's Legacy Tools

- [ ] Engineer's Diary lore item at spawn
  - [ ] Introduces scanning, signal heat, and why the player reads city layers
- [ ] Five Legacy Firmware tools
  - [ ] Define the five baseline tools before implementation
  - [ ] Tier 1 baseline is included here, not a separate TODO
- [ ] Neural Battery and Signal Heat
  - [ ] HUD-visible resource and detection risk
  - [ ] Scan/actions consume battery or raise heat
- [ ] Hearing augment Tier 1
  - [ ] One initial upgrade for audibility range/clarity
  - [ ] Later tiers deferred until baseline is fun

### Death And Recovery

- [ ] Corpse entity on death
  - [ ] Drop physical items and 50% on-hand credits
  - [ ] Make retrieval tension visible on map/log
  - [ ] Battery capacity reduction only when high heat or high-tier tools justify it
- [ ] Respawn in Slum Squat or Safe House
  - [ ] Respawn at low health with visible recovery goals
- [ ] Information loss
  - [ ] Lose unstable tips/fragments before permanent lore
  - [ ] Requires Intel Log first
- [ ] Restricted-zone death consequence
  - [ ] Wanted spike and signal heat increase when relevant
- [ ] Storage across deaths
  - [ ] Stash items/equipment in claimed housing or safe houses

### Persistence

- [ ] Choose persistence format after save surface stabilizes
  - [ ] Do not commit to Cereal until the saved state is known
- [ ] Save/load initial player-facing state
  - [ ] Player location, health, inventory, discovered intel, faction standing, wanted level, world seed
- [ ] Save/load ECS registry only after initial persistence works
  - [ ] Include free-list and destroyed-set correctness as acceptance criteria

---

## Milestone 4 - Expand Systems After The Loop Works

Priority: lower. These should be implemented only when they create visible gameplay.

### Economy

- [ ] Aggregate supply chain flows
  - [ ] District-level extraction -> production -> market stock pulses
  - [ ] Create shortages and price changes the player can exploit
- [ ] Property ownership/rent
  - [ ] Defer until housing and faction territory matter
- [ ] Directive markets
  - [ ] Forecast policy/price changes from scouted directives

### World Scale

- [ ] Clarify full-city scale target
  - [ ] Replace ambiguous "256x256 WU" with macro-cell or world-unit target
- [ ] Larger procedural city with active-region simulation and offscreen aggregation
- [ ] Extend zone roster only when each zone has a gameplay purpose
  - [ ] Transit Hub: useful soon because transit exists
  - [ ] Crematorium: depends on death/corpse loop
  - [ ] Aerodrome: depends on VTOL gameplay
- [ ] Chicago-style height gradients per expanded zone
- [ ] EMMV Auto-Pods
  - [ ] Add only if the player can hail, avoid, steal, or inspect them
- [ ] Branched maglift graph/switching
  - [ ] Current linear stations and terminal switching are complete; expand only for route choice

### Crisis Events

- [ ] Outbreak crisis
  - [ ] First crisis candidate because pathogens already exist
  - [ ] Needs warnings, counterplay, and Crisis Dashboard
- [ ] Power blackout crisis
  - [ ] Power grid exists; make effects visible on lighting, doors, markets, and patrols
- [ ] Collapse crisis
  - [ ] Requires structural consequences and inspection support
- [ ] Crisis Dashboard (`V`)
  - [ ] Single canonical dashboard TODO
  - [ ] Show active threats, trend, affected zones, and recommended player action
- [ ] Add other crises only after their base systems are fun
  - [ ] Market Crash
  - [ ] Coup
  - [ ] Information War
  - [ ] Xenomorphic Bloom
- [ ] Cross-layer cascade logic per crisis
  - [ ] Avoid building a generic cascade framework first

### Social And Dialogue Depth

- [ ] Template-based dialogue profiles
  - [ ] Start with rank/faction/mood templates before grammar engine
- [ ] Organic information propagation
  - [ ] Active-region or social-chunk propagation only
  - [ ] Must produce discoverable leads
- [ ] Knowledge trading and secrets
  - [ ] Requires barter, logs, and information types
- [ ] Ink integration for leaders/fixers
  - [ ] Defer until dialogue UI and faction leaders exist

### Crime Depth

- [ ] Simulated crime economy
  - [ ] Start with theft/contraband near player and markets
  - [ ] Avoid autonomous city-wide crime until profiling supports it
- [ ] Corruption: bribery, favors, backroom deals
  - [ ] Depends on dialogue, reputation, and guards
- [ ] Clandestine labs and raids
  - [ ] Depends on interiors, crime, factions, patrols
- [ ] Graffiti, waste, abandoned zone state
  - [ ] Use as visible faction/decay feedback
  - [ ] Avoid simulating individual waste objects at scale

---

## Milestone 5 - Endgame, Xenos, And Large-Scope Content

Priority: deferred. Keep as direction, not near-term implementation.

### Xenos

- [ ] Define one xeno gameplay role before adding multiple types
- [ ] Archon radius-based layer warping
- [ ] Aborrax type
- [ ] Behedicci type
- [ ] Post-Human type
- [ ] Life stages only after one xeno is fun

### AGI And Endings

- [ ] Five faction/AGI throne room locations
- [ ] 5-layer security gauntlet per throne room
- [ ] AGI encounters scripted after Ink pipeline exists
- [ ] Endings
  - [ ] Purge
  - [ ] Avatar
  - [ ] Synthesis
  - [ ] Reset

### Sanctuary Depth

- [ ] 5-layer Sanctuary defenses
  - [ ] Jammers, DNA locks, guard NPCs, false walls, L4 shell companies
  - [ ] Defer until basic housing/storage is fun

### Conditional Performance Work

- [ ] Profile before sparse-set storage replacement
- [ ] Replace `unordered_map<Entity, T>` with sparse-set only if profiling justifies it
- [ ] Benchmark `view<CognitiveComponent>()` before/after
- [ ] Verify entity deletion/free-list behavior under the new storage

### Deferred Polish

- [ ] Dynamic lighting via cheap overlays/tints for street lights and neon signs
- [ ] Weather presentation: visible acid rain, smog, fog overlays and gameplay cues
- [ ] Neural Scrub action after signal heat creates real pressure
- [ ] Tier 2/Tier 3 tool upgrades after baseline tools are fun
- [ ] Firmware modules after inventory/economy reward loops exist
- [ ] Reclamation drones after corpse retrieval exists
- [ ] Scavenger thieves after corpse loot exists
- [ ] Cut thin-stroke external icon library unless the renderer moves beyond ASCII/glyph HUD
- [ ] Keep "Neon Noir" color palette as style guidance, not a gameplay blocker
