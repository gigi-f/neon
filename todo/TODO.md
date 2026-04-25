# Neon Oubliette Implementation Plan

This file is the singular source of truth for active implementation order. Completed slices are archived in `todo/completed_todo.md`.

Supersedes all previous TODO files, copied OpenSpec artifacts, archived specs, and broad simulation plans that were moved under `todo/`. The external planning reference is `/Users/gm1/.claude/plans/we-went-too-big-linear-map.md`; this file is the repo-local executable plan.

For longer-range direction, read `todo/EPOCHS.md` before adding new sessions. `todo/EPOCHS.md` is a proposal and north-star guide, not an implementation contract. If a proposed epoch or session jumps too far ahead, no longer fits the simulation-exploration vision, or would turn the game into city-builder/player-management work, push back in the implementation notes and suggest a smaller or different session direction before editing this TODO.

## Binding Rules

- Build from the sandbox outward. Do not reintroduce broad simulation systems until the smaller layer below is stable, visible, and tested.
- Every system must have a visible symptom in the game window or HUD.
- Every system must create or support a player verb.
- Every new component/system must be current-scope, tested, and wired into the UI or renderer before expanding scale.
- No ambient runtime spawning. Actor/entity counts come from explicit configuration or authored placement.
- Keep the build small. Do not keep compatibility stubs for deleted systems.
- If a feature needs an OpenSpec change, write only the current focused change. Do not restore archived/spec-dump folders under `todo/`.

## Session Rules

- Phases are grouped into cohesive sessions with one focused gameplay outcome.
- Session headings are ordered top-to-bottom; phase numbers remain the long-running implementation sequence.
- Each session must include `Gameplay outcome`, `Big Picture`, and `Logical next step` notes.
- Each session must name the logical next step that will become the focus of the following session.
- Prefer sessions of 2-4 small phases; a session may be shorter only if it delivers a complete player-facing outcome.
- If a proposed phase does not advance the current session outcome or prepare the named logical next step, defer it instead of circling on minutiae.
- If the big-picture connection is unclear, reframe or defer the phase before implementing it.
- When all phases in a session are complete, archive the completed phases and keep the next session outcome visible at the top of the active TODO.

## Current Runtime Shape

- The runtime has one player, one housing building, one workplace building, one supply building, derived lit pedestrian paths, and at most one fixed worker.
- The player can enter/exit housing, workplace, and supply with `E` and move in one-room interiors.
- The player can inspect nearby housing, workplace, supply, and path targets with `SPACE`.
- The player can pick up or drop the single carried object with `F`, including taking it from supply while empty-handed.
- The player can store carried supply inside housing with `E`, producing `SHELTER SUPPLY: 1/1`.
- The player can stock the workplace bench with carried supply, producing `WORK BENCH: STOCKED`.
- The fixed worker is config-driven and moves on the derived pedestrian path.
- The fixed worker can pick up the single supply object at the supply endpoint, showing `WORKER CARRYING: SUPPLY`.
- The fixed worker carries supply back along the workplace-supply path and stops at the workplace endpoint.
- The fixed worker can stock the same workplace bench state the player can stock.
- The fixed worker can work a stocked workplace bench into `WORK BENCH: OUTPUT READY`.
- The fixed worker can take the ready workplace output as a carried `PART`.
- The fixed worker can route the carried `PART` to housing and set `BUILDING IMPROVED: YES`.
- Derived pedestrian path endpoints have visible signposts that inspect as `TO ...` route labels.
- The player can save the tiny current-scope state with `F5`, reload it with `F9`, and see readable save/load status in the HUD.
- The single carried object is modeled with an item kind, currently `SUPPLY`, and player/worker readouts derive labels from that kind.
- The player can work a stocked workplace bench into `WORK BENCH: OUTPUT READY`.
- The player can take the ready workplace output as a carried `PART`.
- The player can deliver the carried `PART` inside housing to set `BUILDING IMPROVED: YES`.

## Session: Worker Motivation Readout

Gameplay outcome: the player can inspect the fixed worker and understand what the worker is trying to do and why, without being able to command them.

Big Picture: this keeps labor autonomous and legible. The worker should feel like part of an existing social machine with their own reason for moving through production, while the player remains an observer, interrupter, or opportunist rather than a manager.

Logical next step: after motivation and intent are readable, expose blocked loop states so the player can see when organic labor has been interrupted.

## Phase 31: Worker Routine State Readout

Goal: replace the worker's generic carry-only readout with one derived routine state.

- [ ] Add a helper that derives worker routine text from existing state, such as `GOING TO SUPPLY`, `DELIVERING SUPPLY`, `WORKING BENCH`, `DELIVERING PART`, or `DONE`.
- [ ] Use the derived routine text in worker inspection/readout surfaces.
- [ ] Keep routine state derived from current position, carried item, bench state, and building improvement state; do not add a task queue.
- [ ] Add tests for each current loop stage and for the zero-worker config path.

Acceptance:

- [ ] The player can tell what the fixed worker is doing without debug logs.
- [ ] The worker remains autonomous; no player command menu, assignment, priority, or schedule control is introduced.
- [ ] Existing movement and item behavior is unchanged.

## Phase 32: One Labor Reason Tag

Goal: give the fixed worker one inspectable reason for doing the route.

- [ ] Add one small worker motivation field or derived tag, such as `WAGE ROUTE`, `MANDATED SHIFT`, or `DEBT WORK`.
- [ ] Show the tag only through worker inspection/readout text.
- [ ] Keep the tag informational; it must not alter movement, production, or player controls in this phase.
- [ ] Save/load the tag only if it is actual mutable state; otherwise keep it derived/static and test the readout.

Acceptance:

- [ ] Worker labor has a visible social reason without becoming a management system.
- [ ] No wage economy, coercion model, factions, relationships, or schedules are introduced.
- [ ] The player can inspect the reason but cannot change it.

## Session: Loop Blockage Visibility

Gameplay outcome: the player can inspect the tiny production loop and understand why autonomous labor is stalled.

Big Picture: this completes the `Shared Tiny Loop` epoch by making interruption legible. The point is not to optimize the worker, but to let the player see how shared world state can be helped, stolen from, or disrupted.

Logical next step: after blocked states are readable, let the player intentionally interrupt the loop in one small way and show a local consequence.

## Phase 33: First Blocked Worker Labels

Goal: make the current worker readout name the first simple reason the loop is blocked.

- [ ] Derive simple blocked labels for current-scope cases: no supply object available, bench already occupied, output waiting, or building already improved.
- [ ] Show blocked labels in inspection/readout text only when they explain why the worker is not progressing.
- [ ] Do not add recovery rules, alerts, suspicion, dialogue, or a generalized job failure model.
- [ ] Add tests for each blocked label using existing world state.

Acceptance:

- [ ] The next session can focus on loop blockage visibility without first refactoring worker status text.
- [ ] All labels remain derived from existing current-scope state.
- [ ] No new worker behavior is introduced.

## Phase 34: Building And Bench Blockage Readouts

Goal: make blocked loop state visible from the relevant world objects, not only from the worker.

- [ ] Add inspection/readout details to the workplace bench for `EMPTY`, `STOCKED`, `OUTPUT WAITING`, and `BLOCKED BY CARRIER`.
- [ ] Add inspection/readout details to the target building for `NEEDS PART`, `IMPROVED`, or `ALREADY COMPLETE`.
- [ ] Keep these labels derived from existing bench, item, and building state.
- [ ] Add tests that workplace and housing readouts explain the blocked state without creating new behavior.

Acceptance:

- [ ] The player can inspect the place where the blockage occurs.
- [ ] No alerts, quest log, minimap marker, or task assignment UI is introduced.
- [ ] Player and worker actions remain unchanged.

## Phase 35: One Recoverable Blockage Case

Goal: prove that a blocked loop can resume when the player restores the missing condition.

- [ ] Pick one current-scope blockage, such as the player holding the needed item or the output waiting on the bench.
- [ ] Add or verify a deterministic test where the worker does not progress while blocked, then progresses after the player/world state clears the blockage.
- [ ] Show the blocked and resumed states through existing inspection/readout text.
- [ ] Do not add new recovery AI, pathfinding, timers, or worker planning.

Acceptance:

- [ ] Blockage is visible and reversible through current verbs.
- [ ] The worker remains autonomous and deterministic.
- [ ] No generalized job failure or recovery system is introduced.

## Session: Player Interruption

Gameplay outcome: the player can intentionally interfere with the autonomous worker loop once, and the world exposes the result locally.

Big Picture: this starts the `Organic Labor And Production` direction without making the player a manager. The player can exploit or disrupt a process that belongs to the city.

Logical next step: after one interruption has a local consequence, introduce a small inherited gadget surface so the player can observe hidden labor details instead of relying only on ordinary inspection.

## Phase 36: Expected Item Interruption

Goal: make one player theft/delay case explicit in the worker loop.

- [ ] Detect when the player takes or carries the item the worker currently needs next.
- [ ] Show a worker or site readout such as `WAITING FOR SUPPLY` or `EXPECTED PART MISSING`.
- [ ] Keep the consequence local and reversible when the item is returned, delivered, or consumed through current verbs.
- [ ] Add tests for worker waiting while the player holds the expected item and resuming after the condition clears.

Acceptance:

- [ ] The player can interfere by using existing pickup/drop/action verbs.
- [ ] No suspicion, crime, combat, faction, or surveillance system is introduced.
- [ ] The city process still behaves organically; the player does not assign or cancel work.

## Phase 37: One Local Consequence Readout

Goal: make interruption produce a readable local consequence without adding broader risk systems.

- [ ] Add one consequence label to the affected worker, bench, or building, such as `DELAYED`, `OUTPUT MISSING`, or `SHIFT STALLED`.
- [ ] Keep the label derived from the same blockage/interruption state.
- [ ] Ensure the label disappears or changes when the loop resumes or completes.
- [ ] Add tests for consequence text appearing and clearing.

Acceptance:

- [ ] Interruption has a visible symptom in the game window or inspection panel.
- [ ] No global wanted level, surveillance reaction, worker dialogue, or relationship system is introduced.
- [ ] The consequence remains tied to the tiny loop only.

## Phase 38: Production Consequence Summary

Goal: add one compact HUD or inspection summary that names the current production outcome.

- [ ] Add a short status line such as `LOOP: RUNNING`, `LOOP: BLOCKED`, or `LOOP: COMPLETE`.
- [ ] Derive the status from existing supply, bench, carried item, worker, and building-improvement state.
- [ ] Keep the summary informational; it must not add controls or task tracking.
- [ ] Add tests for the summary across running, blocked, and complete states.

Acceptance:

- [ ] The player can understand the tiny production loop at a glance.
- [ ] No quest system, objective tracker, economy dashboard, or management interface is introduced.
- [ ] The summary prepares later observation tools by giving them a simple state to reveal or enrich.

## Session: Gadget Inventory Stub

Gameplay outcome: the player has one explicit inherited gadget surface that can later reveal hidden system details.

Big Picture: this begins the `Illicit Observation Tools` epoch. The player is special because of illicit tools inherited from her engineer mother, not because she commands workers or production.

Logical next step: after the gadget is held/equipped and visible, add one scan mode that reveals hidden labor detail.

## Phase 39: One Inherited Gadget Item

Goal: represent the mother's gadget as a current-scope held/equipped tool without building an inventory.

- [ ] Add one explicit gadget state for the player, such as `MOTHER'S DEBUGGER` or `ILLICIT SCANNER`.
- [ ] Show the gadget in HUD text or inspection text when available.
- [ ] Keep it always available or config-seeded for now; do not add loot, slots, durability, batteries, or upgrades.
- [ ] Add tests for gadget presence/readout and zero-impact on current carryable item behavior.

Acceptance:

- [ ] The gadget is visible as part of the player-facing interface.
- [ ] The gadget does not replace `F` pickup/drop or `E` interaction semantics.
- [ ] No full inventory/equipment system is introduced.

## Phase 40: Gadget Use Prompt

Goal: add a clear first prompt for using the gadget without implementing scanning breadth.

- [ ] Add a single key or context prompt for the gadget that does not conflict with `E`, `F`, or `SPACE`.
- [ ] Keep the first use target-scoped to worker, bench, building, or route inspection range.
- [ ] Show a no-target/no-signal message when nothing valid is nearby.
- [ ] Add tests for prompt eligibility and no-target behavior.

Acceptance:

- [ ] The player has one new verb tied to the inherited gadget.
- [ ] The verb is observation-oriented, not command-oriented.
- [ ] No multi-tool wheel, hotbar, scanner suite, or combat use is introduced.

## Phase 41: Last Gadget Result Readout

Goal: reserve a tiny place to show the most recent gadget result.

- [ ] Add one stored last-result string or small component for the player's gadget result.
- [ ] Show it in the HUD or inspection panel after gadget use.
- [ ] Keep the result volatile/current-scope unless save/load becomes necessary in the next phase.
- [ ] Add tests for result replacement and empty/default display.

Acceptance:

- [ ] Gadget feedback is visible and inspectable.
- [ ] No journal, codex, map annotation, clue database, or quest log is introduced.
- [ ] The next session can fill the result with one hidden labor detail.

## Session: One Scan Mode

Gameplay outcome: the player can use the inherited gadget to reveal one hidden detail about the tiny labor loop.

Big Picture: this makes the player's special role concrete: she can observe hidden systems adults or institutions do not surface. The scan should reveal, decode, or infer; it should not command.

Logical next step: after one hidden detail can be revealed, decide whether to store scan memory, add one spoof/interference verb, or deepen building purposes based on what feels most playable.

## Phase 42: Worker Motivation Scan

Goal: let the gadget reveal a slightly richer worker reason than ordinary inspection.

- [ ] Use the gadget on the fixed worker to reveal one hidden labor detail, such as `DEBT WORK`, `PAY DOCKED IF STALLED`, or `ROUTE QUOTA: 1`.
- [ ] Keep ordinary inspection simpler than the scan result.
- [ ] Do not make the scan alter worker behavior.
- [ ] Add tests for scanned worker result, invalid target no-op, and ordinary inspection remaining unchanged.

Acceptance:

- [ ] The player learns something through the inherited gadget that they cannot learn through basic inspection.
- [ ] No surveillance network, faction record, economy simulation, or relationship model is introduced.
- [ ] The scan remains local to the tiny loop.

## Phase 43: Site Metadata Scan

Goal: let the gadget reveal one hidden field on a workplace, housing building, supply site, or route marker.

- [ ] Pick one site metadata field, such as `OWNER`, `AUTHORITY`, `PURPOSE`, or `ROUTE CARRIES`.
- [ ] Show the result through the existing gadget-result readout.
- [ ] Keep the metadata static or derived; do not add ownership mechanics yet.
- [ ] Add tests for valid target result and no-target behavior.

Acceptance:

- [ ] Building or route purpose becomes more legible without adding a new management layer.
- [ ] No factions, property market, public-works system, or city-scale dependency graph is introduced.
- [ ] The metadata points toward future building-purpose work.

## Phase 44: Scan Result Persistence Decision

Goal: decide whether scan results should persist now or remain volatile.

- [ ] Review whether the last scan result supports immediate play or needs save/load persistence.
- [ ] If persistence is useful, add it narrowly to the tiny save model with tests.
- [ ] If persistence is not useful yet, document that scan memory is intentionally deferred.
- [ ] Do not build a notebook, archive, memory palace, or clue graph.

Acceptance:

- [ ] The current scan feature has a clear persistence boundary.
- [ ] Save/load remains current-scope and deterministic.
- [ ] The next roadmap direction is chosen from actual play needs, not assumed breadth.

## Still Deferred

These remain intentionally out of scope until a smaller phase above creates a concrete need:

- Full building doors, multi-room interiors, furniture components, and pathfinding.
- Multi-slot inventory, equipment, scanner tools, item provenance, market barter, and survival counters.
- Biology, injury, pathogens, cognitive state, relationships, schedules, conversations, rumors, and eavesdropping.
- Roads, traffic, vehicles, transit, stations, power grid simulation, conduits, and city-scale infrastructure.
- Factions, wanted level, directive markets, law enforcement, crises, xenos, AGI cores, death cascade, sanctuary systems, and broad narrative simulation.
- Sparse-set ECS replacement unless profiling proves the current registry is blocking active work.

## Creative North Star

Keep the identity of Neon Oubliette, but do not let it drive premature implementation breadth:

- The city is a neon-lit oubliette: surveillance, abandonment, and artificial hope.
- The player is a ghost in the machine, rebuilding agency from minimal tools and fragile shelter.
- The five-layer simulation remains a long-term narrative framework, not a reason to rebuild 76 systems at once.
- Visible symptom first, player verb second, systemic depth third.
