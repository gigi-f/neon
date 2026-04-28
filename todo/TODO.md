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

## Session: Public Site With Context

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

## Session: One Dependency Chain

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

- [ ] Add a debugger verb (reuse `Shift+G` spoof pattern or extend it) that can disrupt the dependency, such as spoofing the dependency target, marking it severed, or confusing the flow label.
- [ ] Show the disruption through inspection/readout on the affected site and dependency edge.
- [ ] Derive one consequence from the disruption, such as worker unable to resolve next destination, or site showing `DEPENDENCY: DISRUPTED`.
- [ ] Ensure the disruption is reversible through the same verb or a restore action.
- [ ] Add tests for disruption trigger, consequence readout, worker impact (if any), reversal, and boundary (unrelated sites/routes unchanged).

Acceptance:

- [ ] The player has a new interference opportunity tied to the dependency, not just a label to read.
- [ ] The disruption is local and reversible.
- [ ] No global risk, wanted level, or surveillance reaction is introduced.

## Session: Route Purpose And Flow

Gameplay outcome: paths and signposts explain what they connect and carry, and spoofing a signpost now disrupts flow labels rather than only blocking movement.

Big Picture: route purpose turns movement infrastructure into readable city flow. Enriching the existing spoof verb to affect flow labels gives the player a deeper interference tool without adding new UI.

Logical next step: after route purpose and enriched spoofing exist, add one explicit flow blockage with visible consequence and recovery.

## Phase 67: Path And Signpost Purpose Labels

Goal: derive and display purpose labels for paths and signposts.

- [ ] Use connected building/site roles to derive path labels such as `LABOR ROUTE`, `SUPPLY ROUTE`, or `PUBLIC ACCESS`.
- [ ] Extend signpost readout with route purpose in addition to destination, such as `TO WORKPLACE; CARRIES: LABOR`.
- [ ] Add one debugger scan result for routes, such as `EXPECTED CARGO: SUPPLY/PART` or `ACCESS: WORKER ONLY`.
- [ ] Ensure spoofed signpost text still reads coherently with purpose labels.
- [ ] Add tests for purpose derivation, signpost readouts (normal and spoofed), and scan enrichment.

Acceptance:

- [ ] Paths and signposts have readable purpose derived from map topology.
- [ ] The debugger reveals hidden route detail beyond ordinary inspection.
- [ ] No route planner, minimap, or traffic system is introduced.

## Phase 68: Spoof Enrichment — Flow Label Disruption

Goal: make signpost spoofing disrupt the flow label, not just block movement.

- [ ] When a signpost is spoofed, change or corrupt the route purpose label visible in inspection, such as `ROUTE: CONFUSED` or `CARRIES: ???`.
- [ ] Derive a flow-level consequence: worker or site readout shows flow disruption, such as `SUPPLY FLOW: DISRUPTED` on the dependent site.
- [ ] Keep the existing movement-blocking behavior and add the flow-label disruption on top.
- [ ] Ensure restoring the signpost clears the flow disruption readout.
- [ ] Add tests for spoofed flow labels, site-level flow consequence, restoration, and boundary (unrelated flows unchanged).

Acceptance:

- [ ] Spoofing now has a richer effect: movement block plus flow-label corruption.
- [ ] The player can see flow disruption propagate to a dependent site.
- [ ] No new controls are introduced; the existing `Shift+G` verb gains depth.

## Session: Flow Interruption And Recovery

Gameplay outcome: the player can create a visible flow blockage through an existing action, see the consequence on affected sites, and restore the flow through a current-scope recovery action.

Big Picture: flow systems should emerge from concrete interruptions the player can perform. Recovery should make systems feel organic and reversible.

Logical next step: after local flow interruption and recovery work, consider local risk only if the player-facing loop needs stakes.

## Phase 69: One Flow Blockage With Visible Consequence

Goal: create one explicit flow blockage from an existing player verb, with readable consequences on affected objects.

- [ ] Use an existing action such as carrying expected supply, holding finished part, or spoofing a route signpost.
- [ ] Derive one blockage label from that state, such as `FLOW: BLOCKED` or `SUPPLY FLOW: INTERRUPTED`.
- [ ] Show the blockage on the affected site, path, signpost, or worker inspection/readout.
- [ ] Add tests proving the blockage is local (unrelated sites and routes remain normal), reversible, and deterministic.

Acceptance:

- [ ] The player can create a visible blockage using existing verbs.
- [ ] The blockage is inspectable from the relevant objects.
- [ ] No new job system, economy, crisis state, alert feed, or objective marker is introduced.
- [ ] The blockage has a small blast radius; no cascading city-wide effects.

## Phase 70: Flow Recovery And Persistence Boundary

Goal: define recovery, make it visible, and settle the persistence question.

- [ ] Use a current action to clear the blockage: dropping/returning supply, restoring a signpost, delivering a part, or clearing a held output.
- [ ] Show recovery through the same objects that showed the blockage, such as `FLOW: CLEAR` or restored normal labels.
- [ ] Ensure stale blockage text clears when the condition is resolved.
- [ ] If blockage/recovery state is derived from already-saved data, add tests proving save/load restores it correctly. If new persistence is needed, add only the smallest required state. If persistence is premature, document the deferral in a TODO note.
- [ ] Add tests for blocked → recovered transitions, readout clearing, and the chosen persistence boundary.

Acceptance:

- [ ] The player can restore the flow without resetting the world.
- [ ] Recovery is visible through inspection without a separate log or journal.
- [ ] Save/load behavior is explicit and deterministic for current scope.
- [ ] No repair minigame, management command, or generalized event/history system is introduced.
- [ ] The roadmap can move toward local risk or hidden-system clues without ambiguity.

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
