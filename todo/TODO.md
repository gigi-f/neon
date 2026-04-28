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

- The runtime has one player, one housing building, one workplace building, one supply building, one market site, derived lit pedestrian paths, and at most one fixed worker.
- The player can enter/exit housing, workplace, and supply with `E` and move in one-room interiors.
- The player can inspect nearby housing, workplace, supply, market, and path targets with `SPACE`.
- The player can pick up or drop the single carried object with `F`, including taking it from supply while empty-handed.
- The player can store carried supply inside housing with `E`, producing `BUILDING SUPPLY: 1/1`.
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
- The player has `MOTHER'S DEBUGGER` as an always-available inherited gadget, uses it with `G`, and sees a volatile last gadget result in the HUD.
- The debugger reveals hidden worker labor pressure (`DEBT WORK`, `PAY DOCKED IF STALLED`, and `ROUTE QUOTA: 1`) without changing worker behavior.
- The debugger reveals static/derived site metadata such as building purpose, market access, and route-carried material.
- Gadget scan memory is intentionally deferred; the HUD only keeps the current volatile last result.
- The player can use `Shift+G` on a route signpost to spoof or restore that signpost signal.
- A spoofed signpost is inspectable and produces `LOOP: SPOOFED; CONSEQUENCE: ROUTE SIGNAL CONFUSED` without changing worker AI yet.
- A worker on a path with a spoofed signpost shows `BLOCKED: ROUTE SIGNAL CONFUSED` and does not advance until the signpost is restored.
- Tiny save/load persists spoofed signpost state as current-scope mechanical state.
- The player can use `Shift+G` on the workplace/supply dependency to disrupt or restore supply flow.
- A disrupted dependency is inspectable from workplace and supply, appears in debugger scans, and pauses worker supply flow until restored.
- Pedestrian paths and signposts derive readable route purpose labels such as `LABOR ROUTE` and `SUPPLY ROUTE`.
- The inherited debugger reveals route expected cargo and access details on path/signpost scans.
- Spoofed signposts corrupt their carried-flow label, and spoofing the workplace/supply route shows `SUPPLY FLOW: DISRUPTED` on the workplace.
- Spoofed route signposts now derive an explicit local `FLOW: BLOCKED` label on the affected path/signpost, and restoring the signpost clears it.
- Restored route signposts now show local `FLOW: CLEAR` recovery on the signpost, path, and affected workplace supply-flow readout; tiny save/load persists active spoofed blockages but treats recovery acknowledgement as volatile.

## Next Session Candidate: Local Risk, Not Surveillance Yet

Gameplay outcome: player interference has local risk before any surveillance system exists.

Big Picture: risk should begin with nearby people and systems noticing anomalies, not with a premature global surveillance model. This gives future surveillance something to generalize from later.

Logical next step: split the next session into 2-4 tiny phases that start with one witnessed interruption and one local suspicion readout, then add one current-scope way to hide, return, or de-escalate before considering persistence.

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
