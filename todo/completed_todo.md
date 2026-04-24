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
