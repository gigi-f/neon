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
- Gameplay feature completion includes updating the AI playtest harness and running an adaptive terminal playtest for the new behavior.

## Session Rules

- Phases are grouped into cohesive sessions with one focused gameplay outcome.
- Session headings are ordered top-to-bottom; phase numbers remain the long-running implementation sequence.
- Each session must include `Gameplay outcome` notes.
- Prefer sessions of 2-4 small phases; a session may be shorter only if it delivers a complete player-facing outcome.
- If a proposed phase does not advance the current session outcome, defer it instead of circling on minutiae.
- If the big-picture connection is unclear, reframe or defer the phase before implementing it.
- When all phases in a session are complete, archive the completed phases and keep the next session outcome visible at the top of the active TODO.

## Current Runtime Shape

- Authored sandbox scope: one player, one housing building, one workplace building, one supply building, one market site, one clinic site, derived lit pedestrian paths/signposts, and up to two config-driven fixed workers.
- Core verbs: `E` enters/exits sites, applies carried items to housing/workplace state, returns suspected output, or hides a suspected item in housing; `T` talks to/acknowledges a nearby worker or lays low inside stocked housing; `F` picks up or drops the single carried object; `SPACE` runs Debugger inspection on nearby buildings, paths, signposts, and worker targets; `G` uses the Interference Torch to spoof/restore route signposts, disrupt/restore the workplace/supply dependency, spoof/restore one worker wage record, or spoof/restore clinic access when a local worker record exists; `F5`/`F9` save/load the tiny current scope.
- Production loop: player and worker both operate the same `SUPPLY` -> stocked bench -> output-ready bench -> carried `PART` -> improved housing chain, with item labels derived from the carried kind.
- Worker loop: fixed workers follow derived pedestrian paths through supply, workplace, and housing tasks; route endpoints expose readable `TO ...`, `LABOR ROUTE`, `SUPPLY ROUTE`, and carried-flow labels.
- Hidden-system surface: Debugger inspection reveals labor pressure, route quota, site purpose, market access, clinic ration/access context, clinic ledger flags, dependency state, expected cargo, local suspicion witness detail, recoverable workplace log fragments, wage/dock risk, and access details, but scan memory remains volatile HUD-only state.
- Interference loop: spoofed signposts and disrupted dependencies are inspectable, pause affected worker/supply flow, show local `FLOW: BLOCKED` or `SUPPLY FLOW: DISRUPTED` readouts, and persist active blockage state through tiny save/load; restored routes show local `FLOW: CLEAR`, with recovery acknowledgement intentionally volatile.
- Local risk loop: a worker can witness expected `PART` theft or nearby `G` Interference Torch route tampering, producing a current-scope `LOCAL NOTICE` HUD line; day/night phase and nearby worker crowding shrink witness range, while active, returned, corrected, hidden, laid-low, and audit-traced suspicion states are inspectable and persist through tiny save/load without adding surveillance, wanted level, factions, or pursuit.
- Wage clue loop: debugger scan of the worker reveals `WAGE IMPACT: INCIDENT LOGGED; DOCK RISK: ACTIVE` when a suspicion record exists; `G` on the worker with a record toggles `wage_record_spoofed`, changing the scan to `DOCK RISK: CLEARED`; spoofed state persists through save/load (V10).
- Clinic access loop: when a local worker record exists, clinic inspection and Debugger scan show `CLINIC LEDGER: WORK RECORD FLAGGED`; `G` on the clinic toggles volatile `CLINIC ACCESS: GHOST CLEARANCE`, and worker Debugger scan shows a distinct `CLINIC ACCESS: GHOST CLEARANCE MISMATCH`.
- AI playtest surface: `neon_ai_playtest` exposes adaptive terminal snapshots with commands, target details, action prompts, Debugger results, system state, a centered text map, and interactive transcripts. Gameplay feature completion must keep this surface current and include a live terminal playtest.

## Active Sessions:


