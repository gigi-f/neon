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
