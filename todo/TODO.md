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
- Fold readouts and boundaries into mechanic phases. Readability, inspection text, and boundary tests are acceptance criteria of the phase that introduces the mechanic, not separate phases.
- No observation-only sessions. Every session must end with at least one new player verb or interference opportunity, not just inspectable labels.
- No decision-only phases. Design decisions (e.g. "decide whether X needs Y") belong in the prior phase's acceptance criteria or a TODO note, not a standalone phase.
- Gameplay feature completion includes updating the AI playtest harness and running an adaptive terminal playtest for the new behavior.

## Session Rules

- Phases are grouped into cohesive sessions with one focused gameplay outcome.
- Session headings are ordered top-to-bottom; phase numbers remain the long-running implementation sequence.
- Each session must include `Gameplay outcome`, `Big Picture`, and `Logical next step` notes.
- Each session must name the logical next step that will become the focus of the following session.
- Prefer sessions of 2-4 small phases; a session may be shorter only if it delivers a complete player-facing outcome.
- If a proposed phase does not advance the current session outcome or prepare the named logical next step, defer it instead of circling on minutiae.
- If the big-picture connection is unclear, reframe or defer the phase before implementing it.
- When all phases in a session are complete, archive the completed phases and keep the next session outcome visible at the top of the active TODO.
- Prefer vertical slices over horizontal layers. A session that adds a site, its context, its dependency, and one interaction is better than separate sessions for sites, tags, edges, and labels.
- Every 3 sessions of observation/context work must be followed by 1 session that adds a player verb or NPC behavior. If the current plan violates this, restructure before implementing.

## Current Runtime Shape

- Authored sandbox scope: one player, one housing building, one workplace building, one supply building, one market site, one clinic site, derived lit pedestrian paths/signposts, and at most one config-driven fixed worker.
- Core verbs: `E` enters/exits sites, applies carried items to housing/workplace state, returns suspected output, or hides a suspected item in housing; `T` talks to/acknowledges the nearby worker; `F` picks up or drops the single carried object; `SPACE` runs Debugger inspection on nearby buildings, paths, signposts, and worker targets; `G` uses the Interference Torch to spoof/restore route signposts, disrupt/restore the workplace/supply dependency, spoof/restore one worker wage record, or spoof/restore clinic access when a local worker record exists; `F5`/`F9` save/load the tiny current scope.
- Production loop: player and worker both operate the same `SUPPLY` -> stocked bench -> output-ready bench -> carried `PART` -> improved housing chain, with item labels derived from the carried kind.
- Worker loop: the fixed worker follows derived pedestrian paths through supply, workplace, and housing tasks; route endpoints expose readable `TO ...`, `LABOR ROUTE`, `SUPPLY ROUTE`, and carried-flow labels.
- Hidden-system surface: Debugger inspection reveals labor pressure, route quota, site purpose, market access, clinic ration/access context, clinic ledger flags, dependency state, expected cargo, local suspicion witness detail, recoverable workplace log fragments, wage/dock risk, and access details, but scan memory remains volatile HUD-only state.
- Interference loop: spoofed signposts and disrupted dependencies are inspectable, pause affected worker/supply flow, show local `FLOW: BLOCKED` or `SUPPLY FLOW: DISRUPTED` readouts, and persist active blockage state through tiny save/load; restored routes show local `FLOW: CLEAR`, with recovery acknowledgement intentionally volatile.
- Local risk loop: a worker can witness expected `PART` theft or nearby `G` Interference Torch route tampering, producing a current-scope `LOCAL NOTICE` HUD line; active, returned, corrected, hidden, and audit-traced suspicion states are inspectable and persist through tiny save/load without adding surveillance, wanted level, factions, or pursuit.
- Wage clue loop: debugger scan of the worker reveals `WAGE IMPACT: INCIDENT LOGGED; DOCK RISK: ACTIVE` when a suspicion record exists; `G` on the worker with a record toggles `wage_record_spoofed`, changing the scan to `DOCK RISK: CLEARED`; spoofed state persists through save/load (V9).
- Clinic access loop: when a local worker record exists, clinic inspection and Debugger scan show `CLINIC LEDGER: WORK RECORD FLAGGED`; `G` on the clinic toggles volatile `CLINIC ACCESS: GHOST CLEARANCE` without making the clinic enterable or adding health/public-works systems.
- AI playtest surface: `neon_ai_playtest` exposes adaptive terminal snapshots with commands, target details, action prompts, Debugger results, system state, a centered text map, and interactive transcripts. Gameplay feature completion must keep this surface current and include a live terminal playtest.

## Active Sessions

## Session: Clinic Access Ledger

Gameplay outcome: the clinic becomes one local municipal access surface the player can inspect and spoof without adding a health system.

Big Picture: public care should appear as rationed institutional access before it becomes medicine, injury, or public-works simulation. The player sees how local suspicion and wage records can affect service access, then uses the inherited tool to alter one tiny record.

Logical next step: after clinic access can be spoofed locally, make commerce read the same pressure through a market ledger.

## Phase 86: Connect clinic access to the worker record readout.

- [ ] Worker Debugger scan reflects the clinic spoof as a local access mismatch, distinct from the wage-record spoof.
- [ ] Clinic access spoofing does not clear local suspicion, wage/dock risk, route blockage, or workplace audit traces unless the player uses the existing matching verbs.
- [ ] Acceptance: readouts make the difference between `WAGE RECORD`, `CLINIC ACCESS`, and `LOCAL SUSPICION` explicit; AI playtest snapshots expose all three states when present.

## Session: Tense Streets — Cover, Crowds, And Hiding

Gameplay outcome: being witnessed becomes a real risk surface. The player picks *when* and *where* to act, has a place to retreat, and the existing suspicion mechanics gain teeth instead of being more readouts.

Big Picture: the local-risk loop has the bones (witness, suspicion, resolution) but no rhythm and no pressure. Adding a tiny day/night cycle, a second authored worker, and a "lay low" verb in housing turns the existing inspect-and-spoof verbs into stealth-flavored decisions without introducing simulation breadth (no schedules, no NPC AI, no pursuit). This is the action-heavy session called for by the binding pace rule before more context layers are added on top.

Logical next step: with cover and crowds working, transit can carry the player to a second authored district that mirrors the loop and stress-tests per-district state.

## Phase 87: Add a deterministic day/night phase that modulates witness range.

- [ ] Add a `WorldPhaseComponent` (or extend `WorldConfigComponent`) carrying a two-state phase (`DAY`, `NIGHT`) and a tick counter that flips on a configurable interval. No real-time clock, no NPC schedules.
- [ ] At `NIGHT`, the witness range used by `workerWitnessing*` for output theft and route tampering halves.
- [ ] HUD shows a single short `PHASE: DAY|NIGHT` line; AI playtest target detail and map header include the phase.
- [ ] Tiny save/load preserves the current phase and tick offset (save version bumped if the field changes the format).
- [ ] Acceptance: tests verify the same theft witnessed at `DAY` is *not* witnessed at `NIGHT` from the same distance; round-trip preserves the phase; one-phase baseline still works.

## Phase 88: Allow a second fixed worker and crowd camouflage.

- [ ] Lift the `at most one fixed worker` runtime rule for `WorldConfig`; allow `fixed_worker_count = 2` to spawn a second worker on a distinct route.
- [ ] When two or more workers are inside witness range of an interference event, halve the effective range further (stacks with `NIGHT`).
- [ ] Each worker is independently inspectable; suspicion records remain per-worker; wage/dock state and `wage_record_spoofed` are independent.
- [ ] Acceptance: tests cover the one-worker baseline, two-workers reducing effective witness range, save/load round-tripping both workers; AI playtest snapshots show both worker glyphs on the terminal map.

## Phase 89: Add a "lay low" verb in housing that decays an active suspicion at supply cost.

- [ ] Inside housing, `T` consumes exactly one stored shelter `SUPPLY` and decays the active local suspicion to a new `LAID_LOW` resolution; the witness's HUD `LOCAL NOTICE` clears, but the suspicion record persists as inspectable history.
- [ ] If no shelter supply is stored or no suspicion is active, the verb fails with a clear short readout (no resource consumed).
- [ ] `LAID_LOW` is distinct from `RETURNED_OUTPUT`, `CORRECTED_ROUTE`, `HIDDEN_ITEM`: the player keeps the stolen part / spoofed signpost.
- [ ] Acceptance: tests cover lay-low with and without supply, with and without an active record, and across both phases; `LAID_LOW` round-trips through save/load; AI playtest can choose the new prompt and report the result.

## Session: Transit — Cracking Open The World

Gameplay outcome: the world is no longer one site cluster. A transit station carries the player between two parallel districts, each with its own worker, sites, and per-district suspicion state.

Big Picture: the no-transit limitation has served its purpose (kept the prototype small while the verb vocabulary stabilized). Lifting it now mirrors the existing loop in a second authored district without adding routes, schedules, vehicles, or city-scale infrastructure. Transit is one stop, one direction, manual ride. Per-district state proves the underlying mechanics survive scale.

Logical next step: with two districts and transit, the next small step is one piece of state that *travels* between districts (an institutional record that follows the player, or a carried object whose provenance changes meaning across districts).

## Phase 90: Add a station, a second authored district, and a player ride verb.

- [ ] Allow `WorldConfig` to authorize a second `MicroZone` cluster with its own housing, workplace, supply, market, clinic, and signposts. Each cluster is self-contained.
- [ ] Add a `StationComponent` and one station glyph on the boundary of each district, paired by a `transit_link_id`.
- [ ] `E` at a station moves the player instantly to the connected station in the other district with a short `RIDING TRANSIT...` HUD message; carried object follows the player.
- [ ] Tiny save/load preserves the player's current district and station state; save version bumped to `V10`.
- [ ] Acceptance: 2-district and 1-district configs both build cleanly; ride from A→B and B→A round-trip with and without a carried object; AI playtest map renders both clusters and reports the player's current district.

## Phase 91: Per-district worker spawn, routes, and inspection labels.

- [ ] Each district spawns its own configured workers on its own derived paths; workers do not migrate or perceive across districts.
- [ ] Inspection labels (`PURPOSE`, `ROUTE`, `WAGE IMPACT`, `LOCAL NOTICE`, `LOCAL WITNESS`) include a short district tag (e.g., `A:WORKER`, `B:WORKER`) so AI playtest readouts and HUD remain unambiguous.
- [ ] Active local suspicion in district A does not affect worker readouts, wage spoofs, or audit state in district B.
- [ ] Acceptance: tests verify per-district isolation of suspicion, wage spoofing, and dependency disruption; save/load round-trips per-district worker state for both districts.

## Phase 92: Per-district interference state and persistence boundary.

- [ ] Spoofed signposts, dependency disruptions, and `LAID_LOW` traces are scoped to their district and inspected with district tags.
- [ ] Returning to a district restores all of its current-scope flow, witness, and wage state from save without leaking to the other district.
- [ ] Acceptance: tests cover (a) spoofing in A while B is clean, (b) save/load while in B with active spoof in A, (c) ride to A restores the original spoof state; AI playtest transcripts demonstrate cross-district navigation with state intact.

## Still Deferred

These remain intentionally out of scope until a smaller phase above creates a concrete need:

- Full building doors, multi-room interiors, furniture components, and pathfinding.
- Multi-slot inventory, equipment, scanner tools, item provenance, market barter, and survival counters.
- Biology, injury, pathogens, cognitive state, relationships, schedules, conversations, rumors, and eavesdropping.
- Roads, traffic, vehicles, power grid simulation, conduits, and city-scale infrastructure. (Transit and stations are now in scope as one-stop manual ride links between authored districts only — not routes, schedules, vehicles, or networks.)
- Factions, wanted level, directive markets, law enforcement, crises, xenos, AGI cores, death cascade, sanctuary systems, and broad narrative simulation.
- Sparse-set ECS replacement unless profiling proves the current registry is blocking active work.

## Creative North Star

Keep the identity of Neon Oubliette, but do not let it drive premature implementation breadth:

- The city is a neon-lit oubliette: surveillance, abandonment, and artificial hope.
- The player is a ghost in the machine, rebuilding agency from minimal tools and fragile shelter.
- The five-layer simulation remains a long-term narrative framework, not a reason to rebuild 76 systems at once.
- Visible symptom first, player verb second, systemic depth third.
