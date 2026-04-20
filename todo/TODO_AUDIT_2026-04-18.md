# Neon Oubliette TODO Audit - 2026-04-18

Scope: unchecked items in `todo/TODO.md`. Completed items were treated as historical unless an unchecked duplicate depends on them.

Rubric:
- Necessary: needed for the next coherent playable loop, not just world simulation completeness.
- Fun: creates direct tension, discovery, agency, or satisfying feedback.
- Visible: the player can see it, use it, inspect it, or feel its consequences without reading logs.
- Feasible: likely to fit the current SDL/ECS architecture if scoped with active-region updates, caps, caching, and simple data structures.

Verdicts:
- Keep: good TODO as written.
- Refine: keep the intent, but rewrite scope or acceptance criteria.
- Defer: keep for later; not needed for the next loop.
- Merge: duplicate or dependent item should be combined with another TODO.
- Cut: low value or contradicted by the current design direction.

## Executive Findings

1. The TODO is over-indexed on invisible simulation. L3/L4, dialogue propagation, crises, and xeno systems need inspection panels, logs, alerts, map overlays, or concrete player verbs before more backend work.
2. The next playable milestone should be "scan, learn, steal/trade, evade." That makes inspection panels, wanted visuals, inventory management, reputation, and audibility more important than deeper city scaling or endgame.
3. Several items are duplicates or already partially implemented: structural degradation, enterable building state, maglift graph/stops/switching, crisis dashboard, sedition trigger, and wanted visuals.
4. Performance risk is manageable only if high-cardinality systems are scoped to active regions. Avoid city-wide all-pairs social, rumor, crime, faction, and xeno updates.
5. Some specs already say to cut or simplify systems, but the TODO still carries the heavier version. Provenance, hunger/thirst, organ-level biology, and generic markets should stay simplified.

## Must Fix In TODO

- Merge duplicate `Crisis Dashboard (V)` entries in Phase 13 and Phase 14.
- Merge duplicate `Level 1 Sedition trigger` entries in Phase 7 and Phase 14.
- Mark or rewrite `Structural health degradation over time` because structural decay is already implemented in L0/L3.
- Mark or rewrite `Enterable building state` because door interaction/interior state exists, but no interior gameplay exists.
- Mark or rewrite `Maglift Monorail graph with station stops` and `Maglift switching logic`; linear station graph and ping-pong switching already exist.
- Rewrite `Full 256x256 WU procedural generation`; current world bounds are -1000..1000 WU, and "256x256 WU" would be smaller than the current generated city. If the intent is 256x256 macro cells, say that explicitly.
- Replace "Binary serialization using Cereal" with "persistence format chosen after save surface is stable"; adding a dependency now is not necessary.

## Phase 7 - L3 Economic & Market

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Item provenance tracking, high-value goods only | Maybe | Medium | Low | Yes | Refine | Keep only for stolen/unique/quest goods and expose through Financial Forensics or item scan. Generic provenance was already cut. |
| Supply chain flows: Extraction -> Production -> Market | Maybe | Medium | Low | Maybe | Refine | Use district-level aggregate stock pulses, not per-item logistics. Must create shortages the player can exploit. |
| Property ownership and rent/mortgage logic | No | Low | Low | Yes | Defer | Useful for Sanctuary/housing later, but not before trade/crime/reputation. |
| Wage rate directives, L4 employer policy cascades to L3 wage rates | No | Medium | Low | Yes | Merge | Merge under Faction Directive system plus Financial Forensics. Backend-only wage changes are not enough. |

## Phase 7 - L4 Political & Factional

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Territorial ownership maps | Yes | High | Medium | Yes | Refine | Needed for crime, patrols, reputation, directives. Must have a visible overlay or district border coloring. |
| Faction Directive system | Yes | High | Medium | Yes | Refine | Keep as the L4 spine, but start with 3-5 directive types that alter patrols, prices, and access. |
| Reputation system per faction | Yes | High | Medium | Yes | Keep | Strong player-facing progression. Needs HUD/log feedback on reputation deltas. |
| Wanted level | Yes | High | High | Yes | Keep | Immediate loop value; should be visible before deeper L4. |
| Faction patrol routes driven by directives | Yes | High | High | Maybe | Refine | Use waypoint loops and active-region spawning. Avoid city-wide pathfinding. |
| Political directives cascade to L3 | Maybe | Medium | Low | Yes | Merge | Merge into directive implementation and make effects inspectable as price/wage/rent changes. |
| Level 1 Sedition trigger | Yes | High | High | Yes | Merge | Duplicate with Phase 14 routine item. Keep one player-facing sedition TODO. |

## Phase 8 - Sparse-Set Storage

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Replace unordered_map component storage with sparse-set | Conditional | No | No | Yes | Defer | Correctly marked conditional. Do only after profiling shows ECS lookup/cache pressure. |
| Verify O(1) operations | Conditional | No | No | Yes | Merge | Acceptance criteria for sparse-set replacement, not a separate player-facing TODO. |
| Benchmark view<CognitiveComponent>() before/after | Conditional | No | No | Yes | Keep | Keep as profiling gate if sparse-set work starts. |
| Ensure entity deletion still works | Conditional | No | No | Yes | Merge | Test criterion for sparse-set work. |

## Phase 9 - Dialogue & Information

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Audibility model - Glitch-Text HUD | Yes | High | High | Yes | Keep | Strong fit for current glyph/HUD renderer. Make overheard fragments useful. |
| NPC eavesdropping | Yes | High | High | Yes | Keep | Needed to make information systems player-facing. Scope to nearby NPCs. |
| Hearing augments | Maybe | High | High | Yes | Refine | Keep after baseline audibility. Start with one upgrade, not three full tiers. |
| Neural-Audio Recorder | Maybe | High | High | Yes | Refine | Keep if it feeds Dialogue Log/Intel Log. Avoid recording every generated line. |
| Grammar-based dialogue engine | Maybe | Medium | High | Maybe | Refine | Start as templates keyed by faction/rank/mood. Full grammar engine can wait. |
| Ink scripting integration for Gold Path | No | High | High | Maybe | Defer | Valuable for leaders/fixers, but premature before dialogue UI and factions. |
| Information type system | Yes | High | Medium | Yes | Keep | Needed for logs, trading, rumors, price tips. Keep compact enum. |
| Organic information propagation | Maybe | High | Low | Maybe | Refine | Active-region/chunk propagation only. Must produce discoverable leads, not invisible churn. |
| Knowledge trading and secrets | Yes | High | High | Yes | Keep | Core differentiator if implemented through barter/dialogue UI. |

## Phase 9 - Trade & Barter

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Leverage-and-greed trade model | Maybe | High | Medium | Yes | Refine | Implement after basic inventory/trade UI. Use simple modifiers from reputation, need, and scarcity. |
| Inventory management: pick up/drop/use | Yes | High | High | Yes | Keep | Required. Current inventory only handles survival counters. |
| NPC greed margins and social pressure | Maybe | Medium | Medium | Yes | Merge | Merge into trade model; no separate system until barter exists. |

## Phase 9 - Crime & Law Enforcement

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Simulated crime economy | Yes | High | Medium | Maybe | Refine | Start with theft/contraband events near player and market districts. Avoid autonomous city-wide crime at first. |
| Guard NPC patrol and response | Yes | High | High | Yes | Keep | Needed for wanted/sedition. Active-region patrols only. |
| Pursuit and arrest task chain | Yes | High | High | Yes | Refine | Keep a minimal chase/capture loop before complex task chains. |
| Corruption: bribery/favors/deals | Maybe | High | High | Yes | Defer | Fun, but depends on reputation, dialogue, and guards. |
| Clandestine drug labs with raids | No | Medium | Medium | Maybe | Defer | Needs interiors, crime, factions, and patrols first. |
| Notoriety/Wanted visuals and escalation | Yes | High | High | Yes | Merge | Merge with Wanted level; visual acceptance criteria belongs there. |

## Phase 9 - Urban Decay

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Structural health degradation over time | Maybe | Medium | Medium | Yes | Merge | Already implemented as structural decay. Rewrite as "make degradation inspectable and consequential." |
| Graffiti, waste, abandoned zone state | Maybe | Medium | High | Yes | Refine | Keep as visible faction/decay feedback. Avoid simulating individual waste objects at scale. |
| Squatting and rebuilding systems | No | High | High | Maybe | Defer | Better under Sanctuary/housing after interiors and ownership. |

## Phase 10 - Engineer's Legacy

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Engineer's Diary lore item | Yes | Medium | High | Yes | Keep | Cheap onboarding object at spawn. Useful to introduce scanning/tool premise. |
| Five Legacy Firmware tools at game start | Yes | High | High | Yes | Refine | Strong core verb set, but define the five tools before implementation. |
| Neural Battery and Signal Heat | Yes | High | High | Yes | Keep | Good resource/risk loop if HUD-visible. |
| Tool signature visibility increase on repeated death | Maybe | Medium | Medium | Yes | Defer | Depends on death loop and heat. |
| Neural Scrub action | Maybe | Medium | High | Yes | Defer | Keep after heat creates real pressure. |

## Phase 10 - Death Cascade

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Corpse entity and battery capacity reduction | Maybe | High | High | Yes | Refine | Corpse is good; permanent battery reduction is punitive. Gate it to high heat/high-tier tools. |
| Respawn in Slum Squat/Safe House at 10% health | Maybe | High | High | Yes | Keep | Strong survival consequence once housing exists. |
| Lose 25% accumulated information | Maybe | Medium | Medium | Yes | Refine | Needs Intel Log first. Prefer losing unstable tips over random permanent lore. |
| Drop items and 50% credits as corpse loot | Maybe | High | High | Yes | Keep | Fun retrieval tension. Requires inventory and corpse entity. |
| Wanted spike on restricted-zone death/increased Heat | Maybe | High | High | Yes | Keep | Good faction consequence. Depends on wanted/heat. |
| ReclamationDrones retrieve corpses | No | Medium | High | Maybe | Defer | Nice world flavor, not needed for first death loop. |
| Scavenger Thieves prioritize corpses | No | High | High | Maybe | Defer | Fun after corpse loot exists. Active-region only. |

## Phase 10 - Tool Masteries

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Tier 1 Legacy Firmware baseline | Yes | High | High | Yes | Merge | Same as "Five Legacy Firmware tools." Define as acceptance criteria. |
| Tier 2 Enhanced Diagnostics | Maybe | High | High | Yes | Defer | Good progression after scan panels exist. |
| Tier 3 Direct Actions | Maybe | High | High | Maybe | Defer | Powerful but risky. Needs clear cost/heat limits. |
| Firmware module upgrades | Maybe | High | High | Yes | Defer | Requires inventory/economy/reward loops. |

## Phase 11 - Full City

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Full 256x256 WU procedural generation with all 14 zones | No | Medium | High | Maybe | Refine | Clarify scale. Defer until active-region streaming/aggregation exists. |
| Chicago-style height gradients per zone | Maybe | Medium | High | Yes | Keep | Already partly present. Extend by zone when scaling. |
| AERODROME zone | No | Medium | High | Yes | Defer | Needs VTOL gameplay first. |
| TRANSIT HUB zone | Maybe | Medium | High | Yes | Refine | Useful because transit exists. Add one visible hub before full zone system. |
| CREMATORIUM district | No | Medium | High | Yes | Defer | Depends on death/corpse/reclamation loop. |

## Phase 11 - Interiors

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| High-fidelity interior generation with adjacency rules | Maybe | High | High | Maybe | Refine | Start with lazy, template-based interiors. High-fidelity city-wide interiors are not feasible now. |
| Functional prop sets | Yes | High | High | Yes | Refine | Keep small: containers, terminals, beds, doors. Props make interiors playable. |
| Enterable building state | Maybe | Medium | High | Yes | Merge | Already exists as player InteriorComponent toggle. Rewrite as "enterable buildings have visible interiors and exits." |

## Phase 11 - Vertical Traversal

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Staircase/elevator entities with Z-axis transition | Maybe | Medium | High | Maybe | Refine | Needed only when interiors have floors. Begin with discrete floor index, not full 3D physics. |
| Vertical layer system for multi-floor buildings | No | Medium | Medium | Maybe | Defer | Depends on interiors. Keep abstract floor layers. |

## Phase 11 - Futuristic Infrastructure

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| VTOL Hover Vessels | No | High | High | Maybe | Defer | Expensive unless purely ambient. Needs aerodrome and vertical traversal. |
| EMMV Auto-Pods | Maybe | Medium | High | Yes | Refine | Current EMMVs exist. Add driverless behavior only if player can hail/avoid them. |
| Maglift Monorail graph with station stops | Maybe | Medium | High | Yes | Merge | Already has linear stations/stops. Keep only if upgrading to branched graph. |
| Maglift switching logic | Maybe | Medium | Medium | Yes | Merge | Ping-pong terminal switching exists. Rewrite as branched track switching if needed. |

## Phase 12 - Factions

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Five faction entities with territory maps | Yes | High | Medium | Yes | Refine | Start with 2-3 factions if needed, but faction data is necessary for L4. |
| Faction aesthetic differentiation | Yes | High | High | Yes | Keep | Cheap and critical for readability. |
| Faction leader NPCs with Ink dialogue | No | High | High | Maybe | Defer | Needs dialogue UI and faction structure first. |
| Faction standing effects on prices/guards | Yes | High | High | Yes | Keep | Makes reputation visible and consequential. |

## Phase 12 - Xeno System

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Archon xeno with radius layer warping | No | High | High | Maybe | Defer | Great spectacle, but depends on mature layer inspection and combat/avoidance. |
| Aborrax xeno type | No | Medium | High | Maybe | Defer | Define gameplay role before implementation. |
| Behedicci xeno type | No | Medium | High | Maybe | Defer | Same as above. |
| Post-Human xeno type | No | Medium | High | Maybe | Defer | Same as above. |
| Life stage system | No | Medium | Medium | Maybe | Defer | Too deep before one xeno is fun. |

## Phase 12 - AGI Cores

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Five AGI Throne Room locations | No | High | High | Maybe | Defer | Endgame content. Needs factions, interiors, and city scale. |
| 5-layer security gauntlet per Throne Room | No | High | High | Maybe | Defer | Good endgame idea, but very expensive before core loops. |
| AGI encounter scripted via Ink | No | High | High | Maybe | Defer | Depends on Ink integration and narrative pipeline. |

## Phase 13 - Crisis & Emergence

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Collapse crisis | Maybe | High | High | Yes | Defer | Feasible after structural consequences and crisis UI. |
| Outbreak crisis | Maybe | High | High | Yes | Refine | Pathogens exist, so this is a strong first crisis candidate. Needs warnings and counterplay. |
| Market Crash crisis | No | Medium | Medium | Maybe | Defer | Needs deeper L3 and player-facing markets. |
| Coup crisis | No | High | Medium | Maybe | Defer | Needs factions/reputation/patrols. |
| Power Blackout crisis | Maybe | High | High | Yes | Refine | Power grid exists; make blackouts visibly affect lighting/doors/markets. |
| Information War crisis | No | Medium | Medium | Maybe | Defer | Needs information economy and logs. |
| Xenomorphic Bloom crisis | No | High | High | Maybe | Defer | Needs xeno baseline. |
| Cross-layer cascade logic | Maybe | High | Low | Maybe | Refine | Do per-crisis cascades, not a generic framework first. |
| Crisis Dashboard (V) | Maybe | High | High | Yes | Merge | Duplicate with Phase 14. Keep one dashboard item. |
| Directive Markets | Maybe | Medium | Medium | Yes | Refine | Interesting if surfaced as scouted policy/price forecasts. Depends on directives and intel. |

## Phase 14 - God Mode

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Detached camera cursor | Maybe | Medium | High | Yes | Refine | Useful dev/player inspection. Make it a scanner/drone mode if intended for gameplay. |
| Entity inspection at any distance | Maybe | Medium | High | Yes | Refine | Good debug/admin feature. For player mode, gate by tool range/signal heat. |
| Simulation speed control | No | Low | Medium | Yes | Defer | Useful debug control. Not core player fun unless framed as debug/god mode. |
| Layer visibility toggle | Yes | High | High | Yes | Keep | Crucial to make L0-L4 systems visible. |

## Phase 14 - Inspection Panels

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Surface Scan (i) | Yes | High | High | Yes | Keep | Highest priority UI; makes entities legible. |
| Biological Audit (I) | Yes | High | High | Yes | Keep | Current bio systems need player visibility. |
| Cognitive Profile (c) | Yes | High | High | Yes | Keep | Makes BDI/memory/relation work worthwhile. |
| Financial Forensics (f) | Yes | High | High | Yes | Keep | Makes L3 visible and supports trade/crime. |
| Structural Analysis (t) | Yes | Medium | High | Yes | Keep | Makes L0 decay/building systems visible. |

## Phase 14 - History & Logs

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Dialogue Log (L) | Yes | High | High | Yes | Keep | Required once eavesdropping exists. |
| Intel Log (n) | Yes | High | High | Yes | Keep | Required for information-as-currency and death consequences. |
| Crisis Dashboard (V) | Maybe | High | High | Yes | Merge | Duplicate with Phase 13. Keep one crisis UI item. |

## Phase 14 - The Routine

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Box Sorting mini-game | Maybe | Medium | High | Yes | Refine | Good intro if it teaches directives, inventory, scanning, and sedition. Cut if it becomes unrelated busywork. |
| L4 Political risk trigger for ignoring directives | Yes | High | High | Yes | Merge | Duplicate with Phase 7 sedition trigger. Keep one with visible warning/escalation. |

## Phase 14 - Persistence

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Binary serialization using Cereal | No | No | No | Maybe | Refine | Persistence is necessary later; Cereal specifically is not. Choose format after save surface stabilizes. |
| Save/load world state and registry | Maybe | Medium | High | Maybe | Refine | Necessary once sessions exceed prototype length. Start with player/state snapshot before whole world. |
| Persist free-list and destroyed-set | Maybe | No | No | Yes | Merge | Acceptance criteria for registry persistence. |

## Phase 15 - Sanctuary

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Claimable player housing | Maybe | High | High | Yes | Refine | Good medium-term anchor after interiors, death, and storage. |
| 5-layer defenses | No | High | High | Maybe | Defer | Strong fantasy, but too broad before basic housing works. |
| Housing Broker NPC interaction | No | Medium | High | Yes | Defer | Needs dialogue/trade and housing. |
| Storage across deaths | Maybe | High | High | Yes | Keep | Strongly supports death cascade and inventory. |

## Phase 16 - Endgame Scenarios

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Purge ending | No | High | High | Maybe | Defer | Keep as narrative direction, not active implementation TODO. |
| Avatar ending | No | High | High | Maybe | Defer | Same. |
| Synthesis ending | No | High | High | Maybe | Defer | Same. |
| Reset ending | No | High | High | Maybe | Defer | Same. |

## Phase 17 - Polish & Aesthetics

| Item | Necessary | Fun | Visible | Feasible | Verdict | Notes |
|---|---:|---:|---:|---:|---|---|
| Dynamic lighting: street lights/neon signs | Maybe | High | High | Maybe | Refine | Keep if implemented as cheap overlay/tints, not per-light expensive blending everywhere. |
| Weather systems: acid rain/smog/fog | Maybe | High | High | Yes | Refine | Acid rain/smog exist partly. Add visible overlays, sounds, and gameplay effects. |
| Interactive feedback: screen shake/sound/particles | Yes | High | High | Yes | Keep | Needed soon. Small feedback effects make systems feel real. |
| Thin-stroke icon library for HUD diagnostics | No | Low | High | Maybe | Cut | Current renderer is bitmap ASCII/glyph based. External icon integration is unnecessary now. |
| Neon Noir color palette enforced | Maybe | Medium | High | Yes | Refine | Keep as style guidance, but avoid blocking gameplay work on palette enforcement. |

## Recommended Next TODO Shape

Priority 1 - make existing simulation visible:
- Surface Scan, Biological Audit, Cognitive Profile, Financial Forensics, Structural Analysis.
- Layer visibility toggle.
- Wanted level visuals.
- Dialogue/Intel logs once audibility exists.

Priority 2 - make a repeatable loop:
- Inventory pick/drop/use.
- Audibility/eavesdropping with information types.
- Reputation and wanted level.
- Guard patrol/response and minimal pursuit.
- Faction territories and simple directives.

Priority 3 - add consequence and persistence:
- Corpse loot and respawn.
- Storage across deaths.
- Save/load for player, inventory, discovered intel, faction standing, and active world seed.

Priority 4 - scale only after profiling:
- Sparse-set ECS.
- Larger city/zone roster.
- Organic rumor propagation.
- Crises beyond outbreak/blackout.
- Xenos, AGI cores, and endgame.
