# Neon Oubliette Completed TODO

This file archives completed implementation slices moved out of `todo/TODO.md`.

## Current Baseline

- [x] Runtime is reduced to the clean sandbox: one player and one generated housing building.
- [x] Source tree is reduced to focused files: ECS, components, `main.cpp`, world builder, geometry helpers, STB font loading, and two tests.
- [x] Legacy component/system surfaces are removed from active source and tests: ambient spawning, NPCs, vehicles, roads, doors, power, inventory, equipment, scan panels, biology, pathogens, conversations, schedules, relationships, economy, markets, transit, feature flags, and old system scaffolds.
- [x] World rendering has a clean glyph-or-fallback path: fitted glyphs when the font loads, rectangle fallback only when it does not.
- [x] Validation is green: CMake configure/build, `rtk test ctest --test-dir build --output-on-failure`, and cclsp diagnostics on remaining edited C++ files.

## Phase 1: Count-Driven World Builder

Goal: make the one-building sandbox tunable without restoring the old procedural city.

- [x] Add `src/world_config.h`.
- [x] Define explicit world counts: macro zone count, micro-zone counts, and building counts.
- [x] Provide `makeSandboxConfig()` that defaults to one macro, one SLUM/housing micro-zone, and one HOUSING building.
- [x] Rewire `buildWorld()` to consume `WorldConfig` while preserving the current `buildWorld(registry, 1, 1, seed)` compatibility path only if it stays small.
- [x] Add tests proving default config creates one housing building.
- [x] Add tests proving increasing the housing count creates exactly that many housing buildings without overlap.
- [x] Keep `main.cpp` calling the sandbox config path.

Acceptance:

- [x] Temporarily setting housing count to 3 creates 3 visible housing buildings, then reverting returns to 1.
- [x] No roads, doors, NPCs, vehicles, items, power, inventory, or scan systems return in this phase.
- [x] `cmake --build build`, `rtk test ctest --test-dir build --output-on-failure`, and cclsp diagnostics pass.

## Phase 2: Placement Basics And Housing Interaction

Goal: make the first generated building useful before adding more city content.

- [x] Strengthen placement validation: building containment, building-building non-overlap, and player spawn outside solids.
- [x] Add a minimal interaction prompt when the player is near the housing building.
- [x] Add a minimal enter/exit state for the housing unit only if it is visible and reversible.
- [x] Add a small HUD line for current location/state: outside, near housing, inside housing.
- [x] Add tests for validation failures and interaction range helpers.

Acceptance:

- [x] Player cannot spawn inside or walk through the housing unit.
- [x] The housing unit has one obvious player-facing interaction.
- [x] The interaction does not require restoring legacy door/interior systems wholesale.

## Phase 3: Second Building Type And Derived Paths

Goal: add one new purpose at a time and derive infrastructure from explicit needs.

- [x] Add a `WORKPLACE` building type only after Phase 1 and Phase 2 are stable.
- [x] Add connection specs for HOUSING -> WORKPLACE pedestrian access.
- [x] Add `src/infrastructure_solver.h` that derives only the required pedestrian path between existing configured buildings.
- [x] Add the minimal path component/rendering needed for that derived path.
- [x] Add tests for path creation only when both endpoint building types exist.
- [x] Give workplace the same minimal `E` enter/exit interaction pattern as housing.

Acceptance:

- [x] Config with HOUSING=1 and WORKPLACE=1 creates one visible path.
- [x] Config with HOUSING=1 and WORKPLACE=0 creates no path.
- [x] No traffic, transit, power, or ambient population logic is introduced.

## Phase 4: First Player Loop

Goal: create a repeatable verb loop before rebuilding simulation depth.

Candidate loop: leave housing, inspect one nearby thing, return or act on it.

- [x] Choose one investigation verb and implement it narrowly.
- [x] Prefer a simple Surface/Structure readout over full inventory/equipment/scanner architecture.
- [x] Add one HUD/panel view that explains what the player learned.
- [x] Add tests for target selection/range only after the UI behavior is defined.

Acceptance:

- [x] The player can intentionally trigger the verb.
- [x] The result is visible without reading debug logs.
- [x] The implementation does not recreate the deleted scan/equipment stack unless that stack is redesigned for the current scope.

## Phase 5: Fixed Actors Only

Goal: reintroduce people only when the map has a reason for them.

- [x] Add at most one authored/fixed NPC type after housing, workplace, and one player verb exist.
- [x] Keep population count explicit in config.
- [x] Do not add ambient spawning, despawn churn, schedules, relationships, pathogens, or conversations in the first actor pass.
- [x] Give the actor one visible behavior and one testable rule.

Acceptance:

- [x] Actor count is deterministic and config-driven.
- [x] Actor behavior is visible in the game window.
- [x] The actor cannot grow into an uncontrolled simulation surface.

## Phase 6: Inspectable Worker

Goal: let the existing player verb see the first actor before adding any actor systems.

- [x] Add `WORKER` to the inspection target taxonomy.
- [x] Let `SPACE` inspect the fixed worker when nearby.
- [x] Show a simple HUD readout for the worker: role, movement route, and fixed-count status.
- [x] Add tests for worker inspection range and target priority.

Acceptance:

- [x] The player can intentionally inspect the worker without opening any new UI stack.
- [x] Inspecting the worker never creates conversation, relationship, schedule, biology, or pathogen state.
- [x] `cmake --build build`, `rtk test ctest --test-dir build --output-on-failure`, and cclsp diagnostics pass.

## Phase 7: Worker Acknowledgement Interaction

Goal: add one actor-facing interaction without rebuilding conversations.

- [x] Add a minimal `E` prompt when the player is near the worker.
- [x] Add one reversible acknowledgement state, such as `WORKER ACKNOWLEDGED`, visible in the HUD.
- [x] Keep the state local and deterministic; do not add dialogue trees, memory, affinity, rumors, or relationship scores.
- [x] Add tests for acknowledgement range and state toggle.

Acceptance:

- [x] The player has exactly one intentional interaction with the worker.
- [x] The interaction has a visible HUD symptom.
- [x] The worker count and movement behavior remain unchanged.

## Phase 8: Housing Interior Placeholder

Goal: make entering housing more meaningful without doors, furniture systems, or pathfinding.

- [x] Add a minimal interior readout shown only while inside housing.
- [x] Add one authored housing detail, such as `SLEEPING MAT`, as text state rather than a spawned furniture system.
- [x] Let `SPACE` inspect the current housing interior state while inside housing.
- [x] Add tests for inside-housing inspection output selection.

Acceptance:

- [x] Entering housing changes what the player can inspect.
- [x] No door graph, room graph, furniture component hierarchy, or pathfinding is introduced.
- [x] The outside housing building remains unchanged.

## Phase 9: Workplace Interior Placeholder

Goal: make entering workplace distinct from housing before adding jobs or schedules.

- [x] Add a minimal interior readout shown only while inside workplace.
- [x] Add one authored workplace detail, such as `WORK BENCH`, as text state rather than a furniture system.
- [x] Let `SPACE` inspect the current workplace interior state while inside workplace.
- [x] Add tests for inside-workplace inspection output selection.

Acceptance:

- [x] Housing and workplace interiors read differently.
- [x] No job assignment, schedule, economy, market, or production system is introduced.
- [x] The worker remains fixed-count and path-bound.

## Phase 10: One Carried Object

Goal: introduce inventory pressure with one object, not an inventory system.

- [x] Add exactly one authored carryable object in the world, placed by config.
- [x] Add `F PICK UP` and `F DROP` for that object.
- [x] Show carried/not-carried state in the HUD.
- [x] Add tests for pickup range, single-object ownership, and drop placement outside solids.

Acceptance:

- [x] The player can carry at most one object.
- [x] No bag slots, equipment slots, item flags, market barter, survival counters, or scanner tools are introduced.
- [x] The carried object is visible when dropped and visible in HUD when carried.

## Phase 11: Enter Shelter with item

Goal: ability to enter building without dropping item

- [x] Currently the player must drop a held item before entering a building
- [x] Move drop to a new key (for example, f for fling)
- [x] Add tests for item held inside housing.

Acceptance:

- [x] Player can enter a building while holding an item
- [x] Player can drop an item with the f key

## Phase 12: One Explicit Path Upgrade

Goal: improve infrastructure only when a current verb needs it.

- [x] Add one path state, such as `ROUGH` or `LIT`, to the existing pedestrian path.
- [x] Show the path state in inspection readouts.
- [x] Add one visible render difference for the path state.
- [x] Add tests that the state exists only on derived pedestrian paths.

Acceptance:

- [x] The infrastructure upgrade is visible and inspectable.
- [x] No traffic, vehicles, transit, road hierarchy, power simulation, conduits, or stop lights are introduced.
- [x] The path remains non-solid and derived from housing-workplace need.

## Phase 13: One More Building Purpose

Goal: add a third building role only after the player loop uses the first two.

- [x] Choose one role with a direct player-loop purpose: `SUPPLY`.
- [x] Add explicit config counts for that role.
- [x] Add placement, validation, glyph rendering, and one inspection readout.
- [x] Derive only the one needed pedestrian connection from existing buildings, with supply reached through workplace.
- [x] Add tests for role count, non-overlap, and conditional path creation.

Acceptance:

- [x] The new role is visible, inspectable, and connected only when configured.
- [x] No zone distribution tables, procedural city generation, market system, hospital system, or ambient population logic is introduced.
- [x] The default sandbox can still be reduced back to housing-only by config.

## Phase 14: Save And Reload Tiny State

Goal: persist the small sandbox only after the loop has state worth preserving.

- [x] Save player position, current building state, carried object state, and worker route progress.
- [x] Reload the same small state deterministically.
- [x] Add a clear failure path if the save file is missing or invalid.
- [x] Add tests for serialize/deserialize round trip of the tiny state model.

Acceptance:

- [x] Save/load covers only current-scope state.
- [x] No global simulation snapshot, ECS reflection, content database, or migration layer is introduced.
- [x] Runtime still starts cleanly without a save file.

## Phase 15: One Supply Pickup Reason

Goal: make the supply building useful without adding inventory systems.

- [x] Add one supply-interior action that gives the player the existing single carried object if they are empty-handed.
- [x] Show a clear HUD prompt only while inside supply and not carrying anything.
- [x] Keep the carried-object model at exactly one object and one carrier.
- [x] Add tests for supply-only pickup, no duplicate object creation, and no pickup while already carrying.

Acceptance:

- [x] Supply has a direct player-loop purpose.
- [x] No bag slots, item stacks, shops, barter, crafting, or market system is introduced.
- [x] Save/load continues to preserve the carried-object state.

## Phase 16: One Shelter Drop-Off

Goal: create a tiny reason to carry supply back home.

- [x] Let the player drop off the carried object while inside housing.
- [x] Track one visible shelter stock value, such as `SHELTER SUPPLY: 0/1`.
- [x] Clear the carried object after a successful drop-off and keep the world object deterministic.
- [x] Add tests for housing-only drop-off, HUD-readable state, and save/load round trip of the stock value.

Acceptance:

- [x] Carrying something from supply to housing produces a visible result.
- [x] No generalized inventory, base building, resource economy, or survival drain is introduced.
- [x] The stock value remains a single current-scope field.

## Phase 17: One Workplace Use For Supply

Goal: give the workplace a tiny state change before adding jobs or production.

- [x] Let the player deliver the carried object while inside workplace.
- [x] Toggle one workplace state, such as `BENCH STOCKED`, visible in inspection readouts.
- [x] Keep the worker count and route behavior unchanged.
- [x] Add tests for workplace-only delivery, readout change, and save/load round trip of the workplace state.

Acceptance:

- [x] Workplace has one player-facing state affected by carrying supply.
- [x] No production queue, economy, job assignment, schedule, or crafting tree is introduced.
- [x] Existing path and worker tests remain deterministic.

## Phase 18: One Worker Supply Pickup

Goal: let the fixed worker perform the same first supply action as the player without adding NPC inventory.

- [x] Give the fixed worker one explicit carry state, such as `WORKER CARRYING: NONE/SUPPLY`.
- [x] Let the worker take the existing single supply object only when at the supply endpoint and the object is not carried/stored elsewhere.
- [x] Hide/reserve the same world object while the worker carries it, so there is still one object and one carrier.
- [x] Make the worker inspection readout show carried/not-carried state.
- [x] Add tests for supply-endpoint-only pickup, no pickup while player carries it, and no duplicate object creation.

Acceptance:

- [x] The worker can pick up the same supply object model the player uses.
- [x] No generalized NPC inventory, needs system, task queue, or item spawning is introduced.
- [x] Existing player pickup, shelter stock, and save/load behavior remain deterministic.

## Phase 19: One Worker Supply Delivery Route

Goal: make the worker visibly carry supply from supply to workplace using the existing route graph.

- [x] When carrying supply, route the fixed worker from supply toward workplace using the existing derived pedestrian paths.
- [x] Keep the worker path-bound and endpoint-driven; if a workplace-supply path is missing, the worker does not teleport or create a fallback path.
- [x] Show a simple carried-supply visual difference on the worker glyph or inspection readout while in transit.
- [x] Add tests for supply-to-workplace movement, path-missing no-op behavior, and preserved fixed worker count.

Acceptance:

- [x] The worker visibly takes the carried supply toward the workplace.
- [x] No pathfinding graph search, schedules, shift logic, or ambient worker spawning is introduced.
- [x] Player movement, player carrying, and existing worker transition tests remain deterministic.

## Phase 20: One Worker Bench Drop-Off

Goal: let the worker complete the same workplace delivery action as the player.

- [x] Reuse the Phase 17 workplace bench state for worker delivery; do not create a separate NPC-only bench flag.
- [x] Let the worker put carried supply on the bench only when at/inside the workplace endpoint.
- [x] Clear the worker carried state after successful bench drop-off and keep the single supply object deterministic.
- [x] Update workplace/worker inspection readouts so the completed delivery is visible.
- [x] Add tests for workplace-only worker drop-off, bench-state reuse, no duplicate object creation, and save/load round trip of worker carry/bench state.

Acceptance:

- [x] The worker can pick up supply, bring it to workplace, and stock the same bench state the player can stock.
- [x] No production queue, economy, job assignment, schedule, crafting tree, or multi-item NPC inventory is introduced.
- [x] Player and worker delivery use the same current-scope workplace state.

## Phase 21: One Route Signpost

Goal: make the tiny travel loop easier to read in the world.

- [x] Add one visible signpost or marker at each derived pedestrian path endpoint.
- [x] Inspecting a marker names the connected role, such as `TO WORKPLACE` or `TO SUPPLY`.
- [x] Keep markers derived from existing paths and non-solid.
- [x] Add tests for marker count, role labels, non-overlap with solid buildings, and housing-only absence.

Acceptance:

- [x] The player can understand where paths lead without opening debug logs.
- [x] No minimap, route planner, quest marker system, or pathfinding UI is introduced.
- [x] Markers disappear when the corresponding configured path is absent.

## Completed Session: Legible State And Item Semantics

Gameplay outcome: progress is understandable and durable, and the single carried object is modeled by kind instead of hardcoded supply text.

Big Picture: this is the small foundation for later save migration, item taxonomy, inventory expansion, NPC item handling, and readable player progress without committing to those larger systems yet.

Logical next step: once persistence and item labels are stable, make the stocked workplace bench create a new carried objective.

## Phase 22: One Save Slot Menu Prompt

Goal: make persistence legible without building a menu system.

- [x] Add a startup HUD line when no save file exists.
- [x] Add a successful-load HUD line that includes player location and carried/not-carried state.
- [x] Add a failed-load HUD line for missing or invalid save files.
- [x] Add tests for status text selection from save results.

Acceptance:

- [x] Save/load status is understandable from the game window.
- [x] No multiple save slots, save browser, settings screen, migration layer, or autosave system is introduced.
- [x] Runtime still starts cleanly without a save file.

## Phase 23: One Item Kind Label

Goal: make the single carried object extensible before adding any new inventory surface.

- [x] Add a current-scope item kind field for the existing single carryable object, starting with `SUPPLY`.
- [x] Replace hardcoded carried-object HUD/readout text with kind-derived labels.
- [x] Keep the one-object, one-carrier rule unchanged for player and worker carrying.
- [x] Save/load the object kind as part of the tiny save model.
- [x] Add tests for kind labels while dropped, player-carried, worker-carried, and saved/reloaded.

Acceptance:

- [x] Existing supply behavior is unchanged except for going through the generic item-kind path.
- [x] No bag slots, stacks, equipment slots, item provenance, crafting recipes, or market values are introduced.
- [x] Unknown or invalid saved item kinds fail safely through the existing invalid-save path.

## Completed Session: Player-Completed Production Loop

Gameplay outcome: the player can turn supply into one finished object at the workplace, then apply that result to a visible building improvement.

Big Picture: this proves the smallest useful production chain across supply, workplace, and a target building, setting up later crafting, infrastructure, capital goods, public works, and building upgrades while keeping the first pass deterministic.

Logical next step: after the player loop is complete, let the fixed worker mirror the same loop through shared state instead of adding new simulation breadth.

## Phase 24: One Bench Work Action

Goal: let the stocked workplace bench do one visible thing before adding production systems.

- [x] Add a workplace-interior `E` action when the bench is stocked and no output is ready.
- [x] Convert `WORK BENCH: STOCKED` into a single visible state such as `WORK BENCH: OUTPUT READY`.
- [x] Keep the action immediate and deterministic; do not add timers, queues, skill checks, recipes, or job assignment.
- [x] Update inspection and HUD prompt text so the action is discoverable only in the workplace interior.
- [x] Add tests for stocked-only work, repeated-use no-op behavior, readout changes, and save/load round trip.

Acceptance:

- [x] The player can intentionally turn one stocked bench into one ready output.
- [x] The bench still has at most one current-scope state transition active at a time.
- [x] Worker supply delivery remains deterministic and does not auto-process the bench in this phase.

## Phase 25: One Finished Object Pickup

Goal: turn the bench output into a carried objective without creating an inventory system.

- [x] Add one finished item kind, such as `PART`, using the same single carried-object model.
- [x] Let the player take the ready workplace output only while empty-handed.
- [x] Clear the bench output state when the finished item is taken.
- [x] Prevent finished items from being accepted by supply-only housing stock and bench-stock actions.
- [x] Add tests for output-only pickup, no duplicate object creation, item-kind HUD labels, and kind-specific action rejection.

Acceptance:

- [x] In this phase, the player can carry either `SUPPLY` or the one finished item, but not both at once.
- [x] The workplace output is visible before pickup and represented by the existing carried-object model after pickup.
- [x] No general item list, loot table, crafting tree, or production queue is introduced.

## Phase 26: One Building Improvement

Goal: close the first production loop by applying the finished object to one building.

- [x] Let the player deliver the finished item while inside the first target building, housing for this phase.
- [x] Add one visible building improvement state, such as `MAT REPAIRED`, `BUILDING IMPROVED`, or another housing-local first example.
- [x] Clear the carried finished item after successful delivery.
- [x] Make the target building inspection/readouts show the improvement state.
- [x] Add tests for target-building-only delivery, wrong-item rejection, readout changes, and save/load round trip.

Acceptance:

- [x] Supply can move through supply -> workplace -> target building as a visible, player-completable loop.
- [x] The improvement is a single flag, not a base-building, ownership, production, infrastructure, comfort, health, or survival system.
- [x] Existing housing stock remains distinct from the new improvement state.

## Phase 27: Worker Return Route

Goal: make the fixed worker's delivery loop readable after a bench drop-off.

- [x] After stocking the workplace bench, route the empty-handed worker back toward the supply endpoint.
- [x] Show a simple worker readout state such as `WORKER ROUTE: RETURNING TO SUPPLY`.
- [x] Keep routing on the existing workplace-supply path and stop cleanly if that path is absent.
- [x] Do not let the return trip create, consume, or duplicate items.
- [x] Add tests for post-drop-off return movement, missing-path no-op behavior, and preserved worker count.

Acceptance:

- [x] The worker can complete supply -> workplace -> supply movement without a schedule system.
- [x] The return route is visible in movement or inspection.
- [x] No task queue, shift timing, needs model, or ambient worker spawning is introduced.

## Completed Session: Worker Parity On The Tiny Loop

Gameplay outcome: the fixed worker can visibly repeat the same supply-to-workplace-to-building-improvement loop the player can complete, using the same item and building state.

Big Picture: this turns one visible player loop into reusable NPC action primitives for future jobs, schedules, route intent, and cooperative automation without adding those broader simulation systems now.

Logical next step: after worker parity lands, choose one new player-facing pressure point, such as a constrained multi-item carry pass, a second fixed worker role, or a new building purpose, instead of deepening the current loop further.

## Phase 28: Worker Bench Work Parity

Goal: let the fixed worker perform the same one-step bench work the player can perform.

- [x] Let the worker trigger the Phase 24 bench work only when at the workplace endpoint and the bench is stocked.
- [x] Reuse the same bench output state the player creates; do not add an NPC-only output flag.
- [x] Keep worker processing immediate and deterministic.
- [x] Make worker/workplace readouts reveal that the bench output is ready.
- [x] Add tests for worker-only bench work, no work without stocked bench, and no duplicate output creation.

Acceptance:

- [x] Player and worker bench work share one state path.
- [x] The worker does not create recipes, wages, schedules, skills, or production queues.
- [x] Player actions still work when the worker is absent by config.

## Phase 29: Worker Finished Object Handling

Goal: extend worker parity one carried step without building NPC inventory.

- [x] Let the worker take the ready workplace output using the same single carried-object kind model.
- [x] Route the worker toward housing only while carrying the finished item.
- [x] Keep the finished item reserved/hidden while worker-carried, just like carried supply.
- [x] Add worker inspection text for the finished carried kind.
- [x] Add tests for output pickup, housing-directed movement, no pickup while player carries an item, and save/load round trip.

Acceptance:

- [x] The worker can carry one finished item through the same one-carrier rule as the player.
- [x] Routing remains path-bound and deterministic.
- [x] No generalized NPC inventory, home assignment, delivery queue, or pathfinding graph search is introduced.

## Phase 30: Worker Building Delivery

Goal: allow the fixed worker to complete the same building-improvement loop as the player.

- [x] Let the worker deliver the finished item only when at the current target-building endpoint, housing for this phase.
- [x] Reuse the Phase 26 building improvement state; do not add a worker-only building flag.
- [x] Clear the worker carried item after successful delivery.
- [x] Make housing/worker readouts show the completed improvement.
- [x] Add tests for endpoint-only delivery, state reuse, no duplicate object creation, and save/load round trip.

Acceptance:

- [x] Player and worker can complete the same tiny production-delivery-improvement loop through shared state.
- [x] The worker remains a single fixed actor with no needs, schedule, dialogue, faction, or relationship state.
- [x] The loop still works when worker count is zero, because player verbs remain primary.

## Completed Session: Worker Motivation Readout

Gameplay outcome: the player can inspect the fixed worker and understand what the worker is trying to do and why, without being able to command them.

Big Picture: this keeps labor autonomous and legible. The worker feels like part of an existing social machine with its own reason for moving through production, while the player remains an observer, interrupter, or opportunist rather than a manager.

Logical next step: after motivation and intent are readable, expose blocked loop states so the player can see when organic labor has been interrupted.

## Phase 31: Worker Routine State Readout

Goal: replace the worker's generic carry-only readout with one derived routine state.

- [x] Add a helper that derives worker routine text from existing state, such as `GOING TO SUPPLY`, `DELIVERING SUPPLY`, `WORKING BENCH`, `DELIVERING PART`, or `DONE`.
- [x] Use the derived routine text in worker inspection/readout surfaces.
- [x] Keep routine state derived from current position, carried item, bench state, and building improvement state; do not add a task queue.
- [x] Add tests for each current loop stage and for the zero-worker config path.

Acceptance:

- [x] The player can tell what the fixed worker is doing without debug logs.
- [x] The worker remains autonomous; no player command menu, assignment, priority, or schedule control is introduced.
- [x] Existing movement and item behavior is unchanged.

## Phase 32: One Labor Reason Tag

Goal: give the fixed worker one inspectable reason for doing the route.

- [x] Add one small worker motivation field or derived tag, such as `WAGE ROUTE`, `MANDATED SHIFT`, or `DEBT WORK`.
- [x] Show the tag only through worker inspection/readout text.
- [x] Keep the tag informational; it must not alter movement, production, or player controls in this phase.
- [x] Save/load the tag only if it is actual mutable state; otherwise keep it derived/static and test the readout.

Acceptance:

- [x] Worker labor has a visible social reason without becoming a management system.
- [x] No wage economy, coercion model, factions, relationships, or schedules are introduced.
- [x] The player can inspect the reason but cannot change it.

## Completed Session: Loop Blockage Visibility

Gameplay outcome: the player can inspect the tiny production loop and understand why autonomous labor is stalled.

Big Picture: this completes the `Shared Tiny Loop` epoch by making interruption legible. The point is not to optimize the worker, but to let the player see how shared world state can be helped, stolen from, or disrupted.

Logical next step: after blocked states are readable, let the player intentionally interrupt the loop in one small way and show a local consequence.

## Phase 33: First Blocked Worker Labels

Goal: make the current worker readout name the first simple reason the loop is blocked.

- [x] Derive simple blocked labels for current-scope cases: no supply object available, bench already occupied, output waiting, or building already improved.
- [x] Show blocked labels in inspection/readout text only when they explain why the worker is not progressing.
- [x] Do not add recovery rules, alerts, suspicion, dialogue, or a generalized job failure model.
- [x] Add tests for each blocked label using existing world state.

Acceptance:

- [x] The next session can focus on loop blockage visibility without first refactoring worker status text.
- [x] All labels remain derived from existing current-scope state.
- [x] No new worker behavior is introduced.

## Phase 34: Building And Bench Blockage Readouts

Goal: make blocked loop state visible from the relevant world objects, not only from the worker.

- [x] Add inspection/readout details to the workplace bench for `EMPTY`, `STOCKED`, `OUTPUT WAITING`, and `BLOCKED BY CARRIER`.
- [x] Add inspection/readout details to the target building for `NEEDS PART`, `IMPROVED`, or `ALREADY COMPLETE`.
- [x] Keep these labels derived from existing bench, item, and building state.
- [x] Add tests that workplace and housing readouts explain the blocked state without creating new behavior.

Acceptance:

- [x] The player can inspect the place where the blockage occurs.
- [x] No alerts, quest log, minimap marker, or task assignment UI is introduced.
- [x] Player and worker actions remain unchanged.

## Phase 35: One Recoverable Blockage Case

Goal: prove that a blocked loop can resume when the player restores the missing condition.

- [x] Pick one current-scope blockage, such as the player holding the needed item or the output waiting on the bench.
- [x] Add or verify a deterministic test where the worker does not progress while blocked, then progresses after the player/world state clears the blockage.
- [x] Show the blocked and resumed states through existing inspection/readout text.
- [x] Do not add new recovery AI, pathfinding, timers, or worker planning.

Acceptance:

- [x] Blockage is visible and reversible through current verbs.
- [x] The worker remains autonomous and deterministic.
- [x] No generalized job failure or recovery system is introduced.

## Completed Session: Player Interruption

Gameplay outcome: the player can intentionally interfere with the autonomous worker loop once, and the world exposes the result locally.

Big Picture: this starts the `Organic Labor And Production` direction without making the player a manager. The player can exploit or disrupt a process that belongs to the city.

Logical next step: after one interruption has a local consequence, introduce a small inherited gadget surface so the player can observe hidden labor details instead of relying only on ordinary inspection.

## Phase 36: Expected Item Interruption

Goal: make one player theft/delay case explicit in the worker loop.

- [x] Detect when the player takes or carries the item the worker currently needs next.
- [x] Show a worker or site readout such as `WAITING FOR SUPPLY` or `EXPECTED PART MISSING`.
- [x] Keep the consequence local and reversible when the item is returned, delivered, or consumed through current verbs.
- [x] Add tests for worker waiting while the player holds the expected item and resuming after the condition clears.

Acceptance:

- [x] The player can interfere by using existing pickup/drop/action verbs.
- [x] No suspicion, crime, combat, faction, or surveillance system is introduced.
- [x] The city process still behaves organically; the player does not assign or cancel work.

## Phase 37: One Local Consequence Readout

Goal: make interruption produce a readable local consequence without adding broader risk systems.

- [x] Add one consequence label to the affected worker, bench, or building, such as `DELAYED`, `OUTPUT MISSING`, or `SHIFT STALLED`.
- [x] Keep the label derived from the same blockage/interruption state.
- [x] Ensure the label disappears or changes when the loop resumes or completes.
- [x] Add tests for consequence text appearing and clearing.

Acceptance:

- [x] Interruption has a visible symptom in the game window or inspection panel.
- [x] No global wanted level, surveillance reaction, worker dialogue, or relationship system is introduced.
- [x] The consequence remains tied to the tiny loop only.

## Phase 38: Production Consequence Summary

Goal: add one compact HUD or inspection summary that names the current production outcome.

- [x] Add a short status line such as `LOOP: RUNNING`, `LOOP: BLOCKED`, or `LOOP: COMPLETE`.
- [x] Derive the status from existing supply, bench, carried item, worker, and building-improvement state.
- [x] Keep the summary informational; it must not add controls or task tracking.
- [x] Add tests for the summary across running, blocked, and complete states.

Acceptance:

- [x] The player can understand the tiny production loop at a glance.
- [x] No quest system, objective tracker, economy dashboard, or management interface is introduced.
- [x] The summary prepares later observation tools by giving them a simple state to reveal or enrich.

## Completed Session: Gadget Inventory Stub

Gameplay outcome: the player has one explicit inherited gadget surface that can later reveal hidden system details.

Big Picture: this begins the `Illicit Observation Tools` epoch. The player is special because of illicit tools inherited from her engineer mother, not because she commands workers or production.

Logical next step: after the gadget is held/equipped and visible, add one scan mode that reveals hidden labor detail.

## Phase 39: One Inherited Gadget Item

Goal: represent the mother's gadget as a current-scope held/equipped tool without building an inventory.

- [x] Add one explicit gadget state for the player, such as `MOTHER'S DEBUGGER` or `ILLICIT SCANNER`.
- [x] Show the gadget in HUD text or inspection text when available.
- [x] Keep it always available or config-seeded for now; do not add loot, slots, durability, batteries, or upgrades.
- [x] Add tests for gadget presence/readout and zero-impact on current carryable item behavior.

Acceptance:

- [x] The gadget is visible as part of the player-facing interface.
- [x] The gadget does not replace `F` pickup/drop or `E` interaction semantics.
- [x] No full inventory/equipment system is introduced.

## Phase 40: Gadget Use Prompt

Goal: add a clear first prompt for using the gadget without implementing scanning breadth.

- [x] Add a single key or context prompt for the gadget that does not conflict with `E`, `F`, or `SPACE`.
- [x] Keep the first use target-scoped to worker, bench, building, or route inspection range.
- [x] Show a no-target/no-signal message when nothing valid is nearby.
- [x] Add tests for prompt eligibility and no-target behavior.

Acceptance:

- [x] The player has one new verb tied to the inherited gadget.
- [x] The verb is observation-oriented, not command-oriented.
- [x] No multi-tool wheel, hotbar, scanner suite, or combat use is introduced.

## Phase 41: Last Gadget Result Readout

Goal: reserve a tiny place to show the most recent gadget result.

- [x] Add one stored last-result string or small component for the player's gadget result.
- [x] Show it in the HUD or inspection panel after gadget use.
- [x] Keep the result volatile/current-scope unless save/load becomes necessary in the next phase.
- [x] Add tests for result replacement and empty/default display.

Acceptance:

- [x] Gadget feedback is visible and inspectable.
- [x] No journal, codex, map annotation, clue database, or quest log is introduced.
- [x] The next session can fill the result with one hidden labor detail.

## Completed Session: One Scan Mode

Gameplay outcome: the player can use the inherited gadget to reveal one hidden detail about the tiny labor loop.

Big Picture: this makes the player's special role concrete: she can observe hidden systems adults or institutions do not surface. The scan reveals, decodes, or infers; it does not command.

Logical next step: scan memory is intentionally deferred because the last-result HUD is enough for current play. The next session should add one small spoof/interference verb.

## Phase 42: Worker Motivation Scan

Goal: let the gadget reveal a slightly richer worker reason than ordinary inspection.

- [x] Use the gadget on the fixed worker to reveal one hidden labor detail, such as `DEBT WORK`, `PAY DOCKED IF STALLED`, or `ROUTE QUOTA: 1`.
- [x] Keep ordinary inspection simpler than the scan result.
- [x] Do not make the scan alter worker behavior.
- [x] Add tests for scanned worker result, invalid target no-op, and ordinary inspection remaining unchanged.

Acceptance:

- [x] The player learns something through the inherited gadget that they cannot learn through basic inspection.
- [x] No surveillance network, faction record, economy simulation, or relationship model is introduced.
- [x] The scan remains local to the tiny loop.

## Phase 43: Site Metadata Scan

Goal: let the gadget reveal one hidden field on a workplace, housing building, supply site, or route marker.

- [x] Pick one site metadata field, such as `OWNER`, `AUTHORITY`, `PURPOSE`, or `ROUTE CARRIES`.
- [x] Show the result through the existing gadget-result readout.
- [x] Keep the metadata static or derived; do not add ownership mechanics yet.
- [x] Add tests for valid target result and no-target behavior.

Acceptance:

- [x] Building or route purpose becomes more legible without adding a new management layer.
- [x] No factions, property market, public-works system, or city-scale dependency graph is introduced.
- [x] The metadata points toward future building-purpose work.

## Phase 44: Scan Result Persistence Decision

Goal: decide whether scan results should persist now or remain volatile.

- [x] Review whether the last scan result supports immediate play or needs save/load persistence.
- [x] Persistence is not useful yet; no tiny save model change was added.
- [x] If persistence is not useful yet, document that scan memory is intentionally deferred.
- [x] Do not build a notebook, archive, memory palace, or clue graph.

Acceptance:

- [x] The current scan feature has a clear persistence boundary.
- [x] Save/load remains current-scope and deterministic.
- [x] The next roadmap direction is chosen from actual play needs, not assumed breadth.

## Completed Session: One Spoof Action

Gameplay outcome: the player can use the inherited gadget to subtly alter one local label or signal in the tiny loop.

Big Picture: this begins interference without control. The city and workers still act from their own rules, but the player can exploit a weak point in what the system reads.

Logical next step: after one spoof works, add one small worker-facing diversion or delay that comes from the corrupted signal.

## Phase 45: Spoof Candidate Selection

Goal: choose exactly one current-scope thing the debugger can spoof without adding broad systems.

- [x] Pick one candidate: route signpost label, workplace bench access state, carryable item tag, or building entry/readout flag.
- [x] Make the chosen candidate inspectable before and after spoofing.
- [x] Keep spoofing local to the tiny loop; do not add a hacking suite, minigame, cooldown, battery, or inventory expansion.
- [x] Add tests that identify the selected target and reject unrelated targets.

Acceptance:

- [x] There is one clearly bounded interference target.
- [x] The player is exploiting a signal, not commanding an NPC.
- [x] The target choice prepares a visible consequence in the next phase.

## Phase 46: Spoof Verb And Feedback

Goal: add one debugger action that changes the selected local signal and shows immediate feedback.

- [x] Add a single key path or context branch for spoofing that does not replace `G` scan or `E/F/SPACE` semantics.
- [x] Show spoof success or failure through the existing gadget/result HUD surface.
- [x] Ensure spoofing changes only the selected local signal.
- [x] Add tests for successful spoof, no-target failure, and unchanged unrelated loop state.

Acceptance:

- [x] The player has one non-command interference verb.
- [x] Feedback is visible without a new panel or management interface.
- [x] No worker AI rewrite, faction system, or surveillance reaction is introduced.

## Phase 47: Local Spoof Consequence

Goal: make the spoof produce one local, inspectable consequence.

- [x] Derive one consequence from the spoofed state, such as a delayed route, confused signpost, blocked bench, or altered item destination.
- [x] Show the consequence in inspection/HUD text.
- [x] Ensure the consequence can clear or be overwritten by a small current-scope rule.
- [x] Add tests for consequence appearing, remaining local, and clearing/overwriting.

Acceptance:

- [x] Interference creates a readable local outcome.
- [x] The city still behaves organically; the player does not issue instructions.
- [x] No global risk, wanted level, camera network, economy, or relationship model is introduced.

## Completed Session: One Diversion Outcome

Gameplay outcome: a spoofed local signal causes one small worker-facing diversion or delay that the player can observe.

Big Picture: interference matters because autonomous systems read corrupted signals and react according to their own simple rules. The player is exploiting the loop, not giving orders.

Logical next step: after one diversion changes behavior, make the consequence easier to inspect.

## Phase 48: Worker Notices Spoofed Route Signal

Goal: make the fixed worker readout acknowledge the spoofed route signal when it is relevant to their path.

- [x] Add a derived helper that detects whether the worker's assigned path has a spoofed signpost.
- [x] Show a worker readout label such as `ROUTE SIGNAL CONFUSED` while the relevant signpost is spoofed.
- [x] Keep this as readout-only in this phase; do not alter movement yet.
- [x] Add tests for relevant spoofed signpost, unrelated/no signpost, and ordinary readout clearing after restore.

Acceptance:

- [x] The worker appears to notice a corrupted signal without being commanded.
- [x] No dialogue, suspicion, relationship, surveillance, or faction system is introduced.
- [x] The readout remains local to the tiny loop.

## Phase 49: One Spoof Delay Rule

Goal: make the spoof cause one small, deterministic delay in worker progress.

- [x] Choose a narrow delay rule, such as pausing worker movement while their current path has a spoofed signpost.
- [x] Keep pickup, bench work, item delivery, and building improvement logic unchanged outside the delayed movement condition.
- [x] Show the delay through existing worker/loop readouts.
- [x] Add tests that worker position/progress does not advance while spoofed and resumes after restore.

Acceptance:

- [x] The spoof creates a real local outcome.
- [x] The worker follows their own rule; no player command menu or assignment behavior is introduced.
- [x] No pathfinding rewrite, route planner, global risk system, or economy model is introduced.

## Phase 50: Diversion Recovery Boundary

Goal: make the delayed route recover cleanly and define the persistence boundary.

- [x] Verify or add a clear recovery path when the signpost is restored.
- [x] Decide whether spoofed signpost state belongs in tiny save now or remains volatile.
- [x] Spoofed signpost state is persisted in tiny save because it now affects worker movement.
- [x] Add tests for recovery and the chosen persistence boundary.

Acceptance:

- [x] The route diversion can end without resetting the world.
- [x] Save/load behavior is explicit and deterministic for current scope.
- [x] The next session can focus on consequence inspection or building-purpose depth based on actual play needs.

## Completed Session: One Local Consequence

Gameplay outcome: the route diversion leaves one readable local consequence after it affects worker movement.

Big Picture: risk and consequence should first come from the local system being tampered with, not from global surveillance or faction state. The city reacts because a signal was corrupted.

Logical next step: after one consequence exists, make it easier for the player to inspect what happened without adding a quest log or management screen.

## Phase 51: Consequence Source Label

Goal: make the affected local object name why the consequence exists.

- [x] Add one derived consequence source label to the spoofed signpost, worker, or route path.
- [x] Use wording like `CAUSE: SPOOFED SIGNPOST` or `SOURCE: CORRUPTED ROUTE SIGNAL`.
- [x] Keep the label derived from current spoof/diversion state; do not add a general event log.
- [x] Add tests for source label appearing and clearing.

Acceptance:

- [x] The player can connect the consequence to the spoofed signal.
- [x] No investigation journal, quest tracker, faction record, or surveillance feed is introduced.
- [x] The source remains local to the tiny loop.

## Phase 52: One Recovery Symptom

Goal: show that the local consequence can ease when the player restores the signal.

- [x] Add or verify one readable recovery label such as `ROUTE SIGNAL CLEAR` or `DELAY CLEARED`.
- [x] Ensure the label appears only after the spoof state is restored or overwritten.
- [x] Keep recovery deterministic and current-scope.
- [x] Add tests for consequence state before spoof, during spoof, and after restore.

Acceptance:

- [x] The player can see that interference can be undone locally.
- [x] Recovery does not require resetting the world or issuing worker commands.
- [x] No global risk decay or suspicion clock is introduced.

## Phase 53: Local Consequence Boundary

Goal: define what this first consequence does not affect.

- [x] Add tests or readout assertions proving bench state, building improvement state, and item ownership are unchanged by the consequence.
- [x] Document in the TODO or code comments only if needed that this is a route-signal consequence, not a broad alarm system.
- [x] Keep future risk hooks out until there is a repeated local pattern.
- [x] Add one regression test guarding against accidental global behavior.

Acceptance:

- [x] The consequence has a clear mechanical boundary.
- [x] The player remains an interferer, not a production manager.
- [x] Future local-risk work has a concrete, tested base.

## Completed Session: Consequence Inspection

Gameplay outcome: the player can inspect the affected objects and understand the spoof, diversion, and recovery state without reading debug output.

Big Picture: simulation exploration depends on legible systems. Before adding more systems, the current one should explain itself through world-facing text.

Logical next step: after interference is legible, broaden the world by making building purposes more explicit.

## Phase 54: Signpost Consequence Readout

Goal: make the signpost inspection readout carry the complete current-scope consequence.

- [x] Extend signpost readout to include signal state, consequence state, and recovery state.
- [x] Keep the readout compact enough for the HUD/readout line.
- [x] Do not add a modal, map overlay, or route-management panel.
- [x] Add tests for normal, spoofed, delayed, and recovered signpost text.

Acceptance:

- [x] The affected signpost is the first place to inspect the problem.
- [x] The player can understand the consequence without hidden debug logs.
- [x] The feature remains local to the current route.

## Phase 55: Worker Consequence Readout

Goal: make worker inspection reflect the local route consequence from the worker's point of view.

- [x] Add one derived worker label such as `CONFUSED BY ROUTE SIGNAL` or `WAITING ON ROUTE SIGNAL`.
- [x] Show it only when the worker's current path is affected.
- [x] Keep ordinary labor reason and carrying readouts intact.
- [x] Add tests for affected and unaffected worker readouts.

Acceptance:

- [x] The worker appears autonomous and reactive.
- [x] The player does not receive a command interface.
- [x] No dialogue, relationship, or suspicion model is introduced.

## Phase 56: Compact Consequence Summary

Goal: give the HUD one compact summary for the current local interference state.

- [x] Extend the existing loop summary or gadget result surface with a short state such as `INTERFERENCE: ROUTE`.
- [x] Keep the summary informational; it must not become an objective tracker.
- [x] Ensure the summary clears when the local consequence clears.
- [x] Add tests for summary text across normal, spoofed, delayed, and recovered states.

Acceptance:

- [x] The player can see at a glance whether their interference still matters.
- [x] No quest log, management dashboard, or city alert stack is introduced.
- [x] The next session can move to building purpose without leaving the current loop opaque.

## Completed Session: Building Purpose Model

Gameplay outcome: buildings have generic purposes that can describe housing, workplace, supply, and future sites without shelter-specific assumptions.

Big Picture: `building` should cover shelter, production, commercial use, public infrastructure, storage, institutions, and capital equipment. Purpose should become data the player can inspect, not a city-builder control surface.

Logical next step: after purposes are generic and inspectable, add one commercial or capital-purpose site.

## Phase 57: Generic Building Purpose Field

Goal: add a small purpose model that can describe current building roles and future site types.

- [x] Add or extend a building-purpose helper/data shape for current roles: housing, workplace, supply.
- [x] Keep existing `MicroZoneRole` behavior intact unless a narrow refactor is required.
- [x] Avoid adding ownership, zoning policy, rents, taxes, or economic simulation.
- [x] Add tests that current roles resolve to stable purpose labels.

Acceptance:

- [x] Building purpose becomes explicit data or a clearly named derived helper.
- [x] Existing interactions continue to work.
- [x] Future building types can be added without duplicating shelter-only language.

## Phase 58: Purpose-Aware Building Readouts

Goal: make basic building inspection describe what the site is for.

- [x] Update housing, workplace, and supply inspection/readouts to include purpose language.
- [x] Keep action prompts unchanged.
- [x] Ensure the debugger scan can reuse or enrich the same purpose information.
- [x] Add tests for purpose-aware readouts on all current building roles.

Acceptance:

- [x] Buildings are more legible as social/economic sites.
- [x] The player does not gain management controls.
- [x] Readouts prepare for commercial and public sites.

## Phase 59: Generic Building Status Language

Goal: reduce shelter-specific wording where the current mechanic is really building improvement or site condition.

- [x] Rename or wrap player-facing readout helpers so they say `BUILDING`, `SITE`, or another generic term where appropriate.
- [x] Keep housing-specific text only where the meaning is truly domestic.
- [x] Do not perform a broad file-wide rename unless needed for the player-facing behavior.
- [x] Add regression tests for current housing improvement and workplace bench readouts.

Acceptance:

- [x] The code and UI no longer imply all production outcomes are shelter-only.
- [x] Existing tiny-loop behavior is unchanged.
- [x] Future building-purpose sessions have clearer entry points.

## Completed Session: Commercial Or Capital Site

Gameplay outcome: the map contains one non-domestic, non-workplace site that represents commercial or capital purpose.

Big Picture: the city should include places that exist for exchange, storage, finance, machinery, or institutional capital, not only survival shelter and production benches.

Logical next step: after one commercial/capital site exists, add one public-purpose site for contrast.

## Phase 60: One Commercial Site Role

Goal: add one current-scope commercial or capital-purpose site type.

- [x] Pick a narrow role such as `MARKET`, `DEPOT`, `MACHINE SHOP`, or `CAPITAL NODE`.
- [x] Place one site in the authored sandbox without overlapping existing buildings or paths.
- [x] Give it a glyph, color, and basic inspection label.
- [x] Add placement/non-overlap tests.

Acceptance:

- [x] The map has a new kind of building purpose.
- [x] No market prices, buying/selling, ownership, rent, or economy simulation is introduced.
- [x] The site is visible and inspectable.

## Phase 61: Commercial Site Scan Metadata

Goal: let the inherited debugger reveal one hidden commercial/capital metadata field.

- [x] Add one scan result such as `PURPOSE: EXCHANGE`, `CAPITAL EQUIPMENT`, or `ACCESS: RESTRICTED`.
- [x] Show the result through the existing gadget result HUD.
- [x] Keep metadata static or derived.
- [x] Add tests for valid scan result and no-target behavior.

Acceptance:

- [x] The player learns something beyond ordinary inspection.
- [x] No financial system, faction owner, shop UI, or transaction verb is introduced.
- [x] The site points toward later economic/social context.

## Phase 62: Commercial Site Boundary

Goal: make clear what the commercial site can and cannot do in this pass.

- [x] Ensure `E`, `F`, worker delivery, and building improvement do not accidentally treat the site as housing/workplace/supply.
- [x] Add explicit no-op or unavailable readouts where needed.
- [x] Add tests for unchanged player and worker loop behavior.
- [x] Keep the site as observation context only for now.

Acceptance:

- [x] The new site does not break the tiny production loop.
- [x] The player can observe it but not manage it.
- [x] Future sessions can choose one real interaction later.

## Completed Session: Public Site With Context

Gameplay outcome: the map contains one public-purpose site with visible authority/ownership context that contrasts with existing private/commercial sites.

Big Picture: public infrastructure should enter as a visible place with social context attached from the start. Splitting site placement, tagging, and inspection across separate sessions produces horizontal layers with no new player interaction. A vertical slice delivers the site, its context, and its boundary in one pass.

Logical next step: after a public site with context exists, connect it to the existing loop through one dependency edge.

## Phase 63: One Public Site With Authority Context

Goal: add one public-purpose site with purpose tag, authority context, inspection, and scan metadata in a single phase.

- [x] Pick a narrow public role such as `CLINIC`, `PUMP`, `PUBLIC TERMINAL`, or `MUNICIPAL NODE`.
- [x] Place one site in the authored sandbox with visible glyph/color.
- [x] Add a site context tag field supporting labels such as `HOUSEHOLD`, `PRIVATE`, `MUNICIPAL`, `COMMERCIAL`. Assign tags to all current sites deterministically.
- [x] Show the context tag in ordinary inspection readout, such as `AUTHORITY: MUNICIPAL` or `CONTEXT: COMMERCIAL`.
- [x] Add one debugger scan field revealing a richer detail, such as `SERVICE: PUBLIC` or `ACCESS: RATIONED`.
- [x] Add placement, non-overlap, inspection, scan, and tag-assignment tests.

Acceptance:

- [x] Public-purpose site exists in the sandbox with visible glyph and readable context.
- [x] All existing sites have context tags visible in inspection.
- [x] The debugger reveals richer public context than ordinary inspection.
- [x] No health system, utility grid, transit network, public works simulation, faction, ownership transfer, or economy is introduced.

## Phase 64: Public Site And Context Tag Boundaries

Goal: prove the new site and context tags do not accidentally create mechanics.

- [x] Add tests proving context tags do not alter collision, entry, carryables, worker route behavior, or save/load unless explicitly supported.
- [x] Ensure `E`, `F`, worker delivery, and building improvement do not accidentally treat the public site as housing/workplace/supply.
- [x] Add explicit no-op or unavailable readouts for unsupported interactions at the public site.
- [x] Keep all tag behavior deterministic and current-scope.

Acceptance:

- [x] Tags remain player-facing context only; future mechanics opt in deliberately.
- [x] The public site does not break the tiny production loop.
- [x] The code remains small and current-scope.

## Completed Session: One Dependency Chain

Gameplay outcome: one site visibly depends on another, and the player can use the debugger to inspect or disrupt that dependency.

Big Picture: city flow should start with visible edges, not hidden graphs. The first dependency should be both readable and interactable — the player learns what flows exist and can interfere with one.

Logical next step: after one dependency edge exists and can be disrupted, make route purpose labels explain what flows across paths.

## Phase 65: One Dependency Edge With Inspection

Goal: represent and display one dependency between existing sites.

- [x] Choose one edge such as `WORKPLACE DEPENDS ON SUPPLY` or `CLINIC DEPENDS ON SUPPLY`.
- [x] Store or derive the edge in a small helper/data shape.
- [x] Show the dependency in ordinary inspection on involved sites, such as `DEPENDS ON: SUPPLY` or `SUPPORTS: WORKPLACE`.
- [x] Let the debugger scan reveal a richer dependency detail, such as `FLOW: MATERIAL` or `REQUIRED FOR: BENCH STOCK`.
- [x] Add tests for dependency resolution, missing-target behavior, source and target readouts, and scan enrichment.

Acceptance:

- [x] One dependency is explicit, inspectable from both ends, and enriched by the debugger.
- [x] No city-scale dependency graph, planner, or optimization UI is introduced.
- [x] Existing loop mechanics remain unchanged.

## Phase 66: Dependency Disruption Via Debugger

Goal: let the player use the debugger to interfere with the dependency edge.

- [x] Add a debugger verb (reuse `Shift+G` spoof pattern or extend it) that can disrupt the dependency, such as spoofing the dependency target, marking it severed, or confusing the flow label.
- [x] Show the disruption through inspection/readout on the affected site and dependency edge.
- [x] Derive one consequence from the disruption, such as worker unable to resolve next destination, or site showing `DEPENDENCY: DISRUPTED`.
- [x] Ensure the disruption is reversible through the same verb or a restore action.
- [x] Add tests for disruption trigger, consequence readout, worker impact (if any), reversal, and boundary (unrelated sites/routes unchanged).

Acceptance:

- [x] The player has a new interference opportunity tied to the dependency, not just a label to read.
- [x] The disruption is local and reversible.
- [x] No global risk, wanted level, or surveillance reaction is introduced.

## Completed Session: Route Purpose And Flow

Gameplay outcome: paths and signposts explain what they connect and carry, and spoofing a signpost now disrupts flow labels rather than only blocking movement.

Big Picture: route purpose turns movement infrastructure into readable city flow. Enriching the existing spoof verb to affect flow labels gives the player a deeper interference tool without adding new UI.

Logical next step: after route purpose and enriched spoofing exist, add one explicit flow blockage with visible consequence and recovery.

## Phase 67: Path And Signpost Purpose Labels

Goal: derive and display purpose labels for paths and signposts.

- [x] Use connected building/site roles to derive path labels such as `LABOR ROUTE`, `SUPPLY ROUTE`, or `PUBLIC ACCESS`.
- [x] Extend signpost readout with route purpose in addition to destination, such as `TO WORKPLACE; CARRIES: LABOR`.
- [x] Add one debugger scan result for routes, such as `EXPECTED CARGO: SUPPLY/PART` or `ACCESS: WORKER ONLY`.
- [x] Ensure spoofed signpost text still reads coherently with purpose labels.
- [x] Add tests for purpose derivation, signpost readouts (normal and spoofed), and scan enrichment.

Acceptance:

- [x] Paths and signposts have readable purpose derived from map topology.
- [x] The debugger reveals hidden route detail beyond ordinary inspection.
- [x] No route planner, minimap, or traffic system is introduced.

## Phase 68: Spoof Enrichment — Flow Label Disruption

Goal: make signpost spoofing disrupt the flow label, not just block movement.

- [x] When a signpost is spoofed, change or corrupt the route purpose label visible in inspection, such as `ROUTE: CONFUSED` or `CARRIES: ???`.
- [x] Derive a flow-level consequence: worker or site readout shows flow disruption, such as `SUPPLY FLOW: DISRUPTED` on the dependent site.
- [x] Keep the existing movement-blocking behavior and add the flow-label disruption on top.
- [x] Ensure restoring the signpost clears the flow disruption readout.
- [x] Add tests for spoofed flow labels, site-level flow consequence, restoration, and boundary (unrelated flows unchanged).

Acceptance:

- [x] Spoofing now has a richer effect: movement block plus flow-label corruption.
- [x] The player can see flow disruption propagate to a dependent site.
- [x] No new controls are introduced; the existing `Shift+G` verb gains depth.

## Completed Session: Flow Interruption And Recovery

Gameplay outcome: the player can create a visible flow blockage through an existing action, see the consequence on affected sites, and restore the flow through a current-scope recovery action.

Big Picture: flow systems should emerge from concrete interruptions the player can perform. Recovery should make systems feel organic and reversible.

Logical next step: after local flow interruption and recovery work, consider local risk only if the player-facing loop needs stakes.

## Phase 69: One Flow Blockage With Visible Consequence

Goal: create one explicit flow blockage from an existing player verb, with readable consequences on affected objects.

- [x] Use an existing action such as carrying expected supply, holding finished part, or spoofing a route signpost.
- [x] Derive one blockage label from that state, such as `FLOW: BLOCKED` or `SUPPLY FLOW: INTERRUPTED`.
- [x] Show the blockage on the affected site, path, signpost, or worker inspection/readout.
- [x] Add tests proving the blockage is local (unrelated sites and routes remain normal), reversible, and deterministic.

Acceptance:

- [x] The player can create a visible blockage using existing verbs.
- [x] The blockage is inspectable from the relevant objects.
- [x] No new job system, economy, crisis state, alert feed, or objective marker is introduced.
- [x] The blockage has a small blast radius; no cascading city-wide effects.

## Phase 70: Flow Recovery And Persistence Boundary

Goal: define recovery, make it visible, and settle the persistence question.

- [x] Use a current action to clear the blockage: dropping/returning supply, restoring a signpost, delivering a part, or clearing a held output.
- [x] Show recovery through the same objects that showed the blockage, such as `FLOW: CLEAR` or restored normal labels.
- [x] Ensure stale blockage text clears when the condition is resolved.
- [x] If blockage/recovery state is derived from already-saved data, add tests proving save/load restores it correctly. If new persistence is needed, add only the smallest required state. If persistence is premature, document the deferral in a TODO note.
- [x] Add tests for blocked -> recovered transitions, readout clearing, and the chosen persistence boundary.

TODO note: active route blockage remains derived from saved spoofed signpost state. Recovery acknowledgement (`FLOW: CLEAR`) is local volatile readout state after a restore action; saving/loading a restored route returns to normal clear route labels without adding event-history persistence.

Acceptance:

- [x] The player can restore the flow without resetting the world.
- [x] Recovery is visible through inspection without a separate log or journal.
- [x] Save/load behavior is explicit and deterministic for current scope.
- [x] No repair minigame, management command, or generalized event/history system is introduced.
- [x] The roadmap can move toward local risk or hidden-system clues without ambiguity.

## Completed Session: Witnessed Interruption

Gameplay outcome: nearby interference can be noticed locally before any surveillance system exists.

Big Picture: risk starts with people and systems already present in the tiny loop. The worker should notice concrete interference with their expected route or output, not trigger a global wanted model.

Logical next step: expose the witnessed state through local worker/building readouts, then give the player a small way to respond.

## Phase 71: Add witnessed output theft.

- [x] If the player takes the ready workplace `PART` while the fixed worker is nearby or assigned to collect it, record a local suspicion event tied to the worker/workplace pair.
- [x] Surface an immediate HUD/status line so the player can tell the action was noticed.
- [x] Acceptance: unwitnessed pickup remains unchanged; witnessed pickup marks only current-scope local suspicion; tests cover both cases.

## Phase 72: Add witnessed route tampering.

- [x] If the player uses `G` Interference Torch to spoof a signpost while the worker is close enough to be affected by the route, record a local suspicion event with a route-tampering cause.
- [x] Using `G` Interference Torch to restore the signpost clears the flow blockage but does not silently erase that it was witnessed.
- [x] Acceptance: no global surveillance, faction, wanted-level, or NPC pursuit state is introduced; tests cover witnessed route tampering and restoration.

## Completed Session: Local Suspicion Readout

Gameplay outcome: local suspicion is readable, bounded, and tied to the place where the player caused it.

Big Picture: suspicion should be inspectable like every other hidden-system hint. The player should understand who noticed, what they noticed, and what object or route is involved.

Logical next step: give the player one current-scope way to reduce the local concern without adding a broad stealth system.

## Phase 73: Add worker and workplace suspicion inspection.

- [x] `SPACE` on the worker and affected workplace shows a compact local suspicion line such as `SUSPICION: MISSING PART` or `SUSPICION: ROUTE TAMPERING`.
- [x] The line identifies only current-scope cause and target; it does not infer motives, crimes, or citywide consequences.
- [x] Acceptance: readouts clear when no local suspicion exists; tests cover worker and building readout boundaries.

## Phase 74: Add Debugger view of local suspicion.

- [x] `SPACE` on the worker, affected workplace, or affected route reveals the local witness record and the current suspected cause through Debugger inspection.
- [x] The Debugger distinguishes active flow blockage from lingering local suspicion.
- [x] Acceptance: volatile last-result HUD behavior remains unchanged; tests cover Debugger output for theft, tampering, and no-suspicion cases.

## Completed Session: Hiding Or De-escalation

Gameplay outcome: the player has one small response to local suspicion besides walking away.

Big Picture: this should be a tiny player-facing pressure valve, not a stealth overhaul. Returning, hiding, or correcting the specific anomaly is enough for now.

Logical next step: once de-escalation exists, decide exactly which suspicion state should persist through save/load.

## Phase 75: Allow returning a suspicious missing output.

- [x] If suspicion came from taking the workplace `PART`, carrying the `PART` back to the workplace lets `E` return it to the expected output/bench state.
- [x] Returning the item changes the suspicion readout to a de-escalated local state instead of deleting all evidence instantly.
- [x] Acceptance: normal part delivery to housing still works; tests cover return versus housing delivery.

## Phase 76: Allow correcting suspicious route tampering.

- [x] If suspicion came from a `G` Interference Torch route spoof, using `G` Interference Torch to restore the signpost while the worker is not currently blocked changes the suspicion readout to a de-escalated local state.
- [x] The route remains mechanically clear and readable as `FLOW: CLEAR`.
- [x] Acceptance: restoring an unrelated signpost does not de-escalate the event; tests cover route-scoped correction.

## Phase 77: Add one bounded hiding option.

- [x] Let the player hide the carried suspicious item inside housing with `E`, marking it hidden from the current worker's immediate concern while keeping it inspectable and Debugger-readable.
- [x] The hidden state should be local to the tiny current scope and avoid inventory expansion.
- [x] Acceptance: hidden item state is visible through housing inspection/Debugger inspection; tests cover hiding only when carrying the suspected item.

## Completed Session: Suspicion Persistence

Gameplay outcome: tiny save/load preserves the local risk state that matters and drops only volatile acknowledgement text.

Big Picture: persistence should prove the boundary before larger surveillance exists. Active local suspicion and de-escalated local suspicion are mechanical; one-frame HUD explanations are not.

Logical next step: after local suspicion survives a save boundary, start adding small traces of larger institutions without making them active systems.

## Phase 78: Persist active local suspicion.

- [x] Extend tiny save/load so active witnessed theft or route-tampering suspicion survives reload with the same cause and target.
- [x] Preserve only current-scope fields needed by readouts and mechanics.
- [x] Acceptance: active local suspicion round-trips; volatile HUD last-result text still does not become persistent state.

## Phase 79: Persist de-escalated local suspicion boundary.

- [x] Save/load preserves de-escalated local suspicion as a quieter readout while not re-triggering the original witnessed HUD/status event.
- [x] Corrected flow and returned/hidden item states remain mechanically consistent after reload.
- [x] Acceptance: tests cover active suspicion, de-escalated suspicion, and no-suspicion round trips.

## Completed Session: Institutional Log Fragment

Gameplay outcome: the player can uncover one local trace of a larger system without activating that larger system yet.

Big Picture: hidden systems should be foreshadowed through repeated local hooks. A log fragment can point toward audits, debt, or surveillance while staying deterministic and attached to existing buildings.

Logical next step: choose the next foreshadowed system only after the player has a repeated reason to care about the fragment.

## Phase 80: Add one workplace log fragment.

- [x] Add a deterministic institutional log fragment to the current workplace that references local output, route, or bench anomalies.
- [x] The fragment appears only after a relevant local suspicion or de-escalation state exists.
- [x] Acceptance: the fragment is tied to existing workplace state and does not add faction AI, economy simulation, or surveillance networks.

## Phase 81: Add a player verb to recover the fragment.

- [x] Let the player use `SPACE` on the affected workplace to recover the log fragment into the volatile Debugger result.
- [x] The recovered text should be short, literal, and separate from live world speech.
- [x] Acceptance: the same target without a qualifying local state reports no fragment; tests cover qualifying and non-qualifying scans.

## Phase 82: Add a local consequence clue.

- [x] If the player recovers the fragment, affected workplace/worker inspection can show one compact clue such as `AUDIT TRACE: LOCAL ONLY`.
- [x] The clue should foreshadow institutional control without adding audits as an active system.
- [x] Acceptance: clue state remains current-scope, deterministic, and covered by save/load boundary tests only if the implementation makes it mechanical.

## Completed Session: Worker Debt Or Wage Clue

Gameplay outcome: the debugger scan of the nearby worker reveals wage-debt risk tied to the player's interference record, and the player can spoof that record with the G torch to clear the risk readout — linking the interference loop to the labor loop without adding any new AI or economy simulation.

Big Picture: hidden systems foreshadowing — the wage/debt pressure is not simulated, but its local trace becomes visible as a consequence of the suspicion record and manipulable via the inherited gadget.

Logical next step: Public Infrastructure Clue — foreshadow city infrastructure ownership without building it.

## Phase 83: Dynamic wage scan consequence.

- [x] When `localSuspicionRecordExists` is true for the target worker, `inheritedGadgetWorkerScan` appends `WAGE IMPACT: INCIDENT LOGGED; DOCK RISK: ACTIVE` to the static scan string.
- [x] When no record exists, scan is unchanged.
- [x] Acceptance: tests cover scan with and without a suspicion record.

## Phase 84: Wage record spoof verb.

- [x] Add `wage_record_spoofed: bool` (default false) to `FixedActorComponent`.
- [x] Add WORKER as a valid `inheritedGadgetCanSpoofTarget` only when a suspicion record exists on that worker.
- [x] G on worker with record toggles `wage_record_spoofed`; spoof result shows `SPOOFED WAGE RECORD: INCIDENT CLEARED`, restore shows `RESTORED WAGE RECORD: INCIDENT ACTIVE`.
- [x] When `wage_record_spoofed`, scan shows `WAGE IMPACT: RECORD ALTERED; DOCK RISK: CLEARED` instead of `DOCK RISK: ACTIVE`.
- [x] `wage_record_spoofed` persists through save/load (save version bumped to V9).
- [x] Acceptance: spoof gated on record existence; scan readout flips correctly; reversible; save/load round-trip tested.

## Phase 85: Add clinic access ledger scan and spoof target.

- [x] When a worker has a local suspicion/wage record, `SPACE` Debugger inspection on the clinic shows one compact municipal access line: `CLINIC LEDGER: WORK RECORD FLAGGED`.
- [x] `G` on the clinic toggles exactly one local spoof state, `CLINIC ACCESS: GHOST CLEARANCE`, with reversible spoof/restored last-result messages.
- [x] The spoof state is visible through clinic ordinary inspection, clinic Debugger inspection, and `neon_ai_playtest` target details.
- [x] Acceptance: the clinic remains non-enterable; no health, injury, medicine, doctor, appointment, public-works, or faction system is introduced; tests and an interactive terminal playtest cover flagged, spoofed, restored, and no-record cases.

## Phase 86: Connect clinic access to the worker record readout.

- [x] Worker Debugger scan reflects the clinic spoof as a local access mismatch, distinct from the wage-record spoof.
- [x] Clinic access spoofing does not clear local suspicion, wage/dock risk, route blockage, or workplace audit traces unless the player uses the existing matching verbs.
- [x] Acceptance: readouts make the difference between `WAGE RECORD`, `CLINIC ACCESS`, and `LOCAL SUSPICION` explicit; AI playtest snapshots expose all three states when present.

## Completed Session: Tense Streets — Cover, Crowds, And Hiding

Gameplay outcome: being witnessed becomes a real risk surface. The player picks *when* and *where* to act, has a place to retreat, and the existing suspicion mechanics gain teeth instead of being more readouts.

Big Picture: the local-risk loop has the bones (witness, suspicion, resolution) but no rhythm and no pressure. Adding a tiny day/night cycle, a second authored worker, and a "lay low" verb in housing turns the existing inspect-and-spoof verbs into stealth-flavored decisions without introducing simulation breadth (no schedules, no NPC AI, no pursuit).

Logical next step: with cover and crowds working, transit can carry the player to a second authored district that mirrors the loop and stress-tests per-district state.

## Phase 87: Add a deterministic day/night phase that modulates witness range.

- [x] Add a `WorldPhaseComponent` carrying a two-state phase (`DAY`, `NIGHT`) and an elapsed-time counter that flips on a configurable interval. No real-time clock, no NPC schedules.
- [x] At `NIGHT`, the witness range used by `workerWitnessing*` for output theft and route tampering halves.
- [x] HUD shows a single short `PHASE: DAY|NIGHT` line; AI playtest target detail and map header include the phase.
- [x] Tiny save/load preserves the current phase and elapsed offset (save version bumped to V10).
- [x] Acceptance: tests verify the same theft witnessed at `DAY` is not witnessed at `NIGHT` from the same distance; round-trip preserves the phase; one-phase baseline still works.

## Phase 88: Allow a second fixed worker and crowd camouflage.

- [x] Lift the `at most one fixed worker` runtime rule for `WorldConfig`; allow `fixed_worker_count = 2` to spawn a second worker on a distinct route.
- [x] When two or more workers are inside witness range of an interference event, halve the effective range further (stacks with `NIGHT`).
- [x] Each worker is independently inspectable; suspicion records remain per-worker; wage/dock state and `wage_record_spoofed` are independent.
- [x] Acceptance: tests cover the one-worker baseline, two-workers reducing effective witness range, save/load round-tripping both workers; AI playtest snapshots show both worker glyphs on the terminal map.

## Phase 89: Add a "lay low" verb in housing that decays an active suspicion at supply cost.

- [x] Inside housing, `T` consumes exactly one stored shelter `SUPPLY` and decays the active local suspicion to a new `LAID_LOW` resolution; the witness's HUD `LOCAL NOTICE` clears, but the suspicion record persists as inspectable history.
- [x] If no shelter supply is stored or no suspicion is active, the verb fails with a clear short readout (no resource consumed).
- [x] `LAID_LOW` is distinct from `RETURNED_OUTPUT`, `CORRECTED_ROUTE`, `HIDDEN_ITEM`: the player keeps the stolen part / spoofed signpost.
- [x] Acceptance: tests cover lay-low with and without supply, with and without an active record, and across both phases; `LAID_LOW` round-trips through save/load; AI playtest can choose the new prompt and report the result.
