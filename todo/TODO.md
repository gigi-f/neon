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

- The runtime has one player, one housing building, one workplace building, one supply building, derived lit pedestrian paths, and at most one fixed worker.
- The player can enter/exit housing and workplace with `E` and move in one-room interiors.
- The player can inspect nearby housing, workplace, supply, and path targets with `SPACE`.
- The fixed worker is config-driven and moves on the derived pedestrian path.

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
