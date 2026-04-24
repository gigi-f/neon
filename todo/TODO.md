# Neon Oubliette Implementation Plan

This file is the singular source of truth for active implementation order. Completed slices are archived in `todo/completed_todo.md`.

Supersedes all previous TODO files, copied OpenSpec artifacts, archived specs, and broad simulation plans that were moved under `todo/`. The external planning reference is `/Users/gm1/.claude/plans/we-went-too-big-linear-map.md`; this file is the repo-local executable plan.

## Binding Rules

- Build from the sandbox outward. Do not reintroduce broad simulation systems until the smaller layer below is stable, visible, and tested.
- Every system must have a visible symptom in the game window or HUD.
- Every system must create or support a player verb.
- Every new component/system must be current-scope, tested, and wired into the UI or renderer before expanding scale.
- No ambient runtime spawning. Actor/entity counts come from explicit configuration or authored placement.
- Keep the build small. Do not keep compatibility stubs for deleted systems.
- If a feature needs an OpenSpec change, write only the current focused change. Do not restore archived/spec-dump folders under `todo/`.

## Current Runtime Shape

- The runtime has one player, one housing building, one workplace building, one derived pedestrian path, and at most one fixed worker.
- The player can enter/exit housing and workplace with `E`.
- The player can inspect nearby housing, workplace, and path targets with `SPACE`.
- The fixed worker is config-driven and moves on the derived pedestrian path.

## Phase 10: One Carried Object

Goal: introduce inventory pressure with one object, not an inventory system.

- [ ] Add exactly one authored carryable object in the world, placed by config.
- [ ] Add `E PICK UP` and `E DROP` for that object.
- [ ] Show carried/not-carried state in the HUD.
- [ ] Add tests for pickup range, single-object ownership, and drop placement outside solids.

Acceptance:

- [ ] The player can carry at most one object.
- [ ] No bag slots, equipment slots, item flags, market barter, survival counters, or scanner tools are introduced.
- [ ] The carried object is visible when dropped and visible in HUD when carried.

## Phase 11: One Shelter Need

Goal: add one survival-adjacent pressure only after carrying exists.

- [ ] Add one named shelter need, such as `REST`, as a HUD state.
- [ ] Let entering housing satisfy or reset that single need.
- [ ] Keep the need deterministic and non-lethal.
- [ ] Add tests for need change inside housing and no change outside housing.

Acceptance:

- [ ] The need creates a reason to return home.
- [ ] No health, hunger, thirst, injury, pathogen, death, or survival inventory system is introduced.
- [ ] The need is visible without debug logs.

## Phase 12: One Explicit Path Upgrade

Goal: improve infrastructure only when a current verb needs it.

- [ ] Add one path state, such as `ROUGH` or `LIT`, to the existing pedestrian path.
- [ ] Show the path state in inspection readouts.
- [ ] Add one visible render difference for the path state.
- [ ] Add tests that the state exists only on derived pedestrian paths.

Acceptance:

- [ ] The infrastructure upgrade is visible and inspectable.
- [ ] No traffic, vehicles, transit, road hierarchy, power simulation, conduits, or stop lights are introduced.
- [ ] The path remains non-solid and derived from housing-workplace need.

## Phase 13: One More Building Purpose

Goal: add a third building role only after the player loop uses the first two.

- [ ] Choose one role with a direct player-loop purpose, such as `SUPPLY` or `CLINIC`.
- [ ] Add explicit config counts for that role.
- [ ] Add placement, validation, glyph rendering, and one inspection readout.
- [ ] Derive only the one needed pedestrian connection from existing buildings.
- [ ] Add tests for role count, non-overlap, and conditional path creation.

Acceptance:

- [ ] The new role is visible, inspectable, and connected only when configured.
- [ ] No zone distribution tables, procedural city generation, market system, hospital system, or ambient population logic is introduced.
- [ ] The default sandbox can still be reduced back to housing-only by config.

## Phase 14: Save And Reload Tiny State

Goal: persist the small sandbox only after the loop has state worth preserving.

- [ ] Save player position, current building state, carried object state, and worker route progress.
- [ ] Reload the same small state deterministically.
- [ ] Add a clear failure path if the save file is missing or invalid.
- [ ] Add tests for serialize/deserialize round trip of the tiny state model.

Acceptance:

- [ ] Save/load covers only current-scope state.
- [ ] No global simulation snapshot, ECS reflection, content database, or migration layer is introduced.
- [ ] Runtime still starts cleanly without a save file.

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
