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
- The player has `MOTHER'S DEBUGGER` as an always-available inherited gadget, uses it with `G`, and sees a volatile last gadget result in the HUD.
- The debugger reveals hidden worker labor pressure (`DEBT WORK`, `PAY DOCKED IF STALLED`, and `ROUTE QUOTA: 1`) without changing worker behavior.
- The debugger reveals static/derived site metadata such as building purpose and route-carried material.
- Gadget scan memory is intentionally deferred; the HUD only keeps the current volatile last result.
- The player can use `Shift+G` on a route signpost to spoof or restore that signpost signal.
- A spoofed signpost is inspectable and produces `LOOP: SPOOFED; CONSEQUENCE: ROUTE SIGNAL CONFUSED` without changing worker AI yet.
- A worker on a path with a spoofed signpost shows `BLOCKED: ROUTE SIGNAL CONFUSED` and does not advance until the signpost is restored.
- Tiny save/load persists spoofed signpost state as current-scope mechanical state.

## Session: Building Purpose Model

Gameplay outcome: buildings have generic purposes that can describe housing, workplace, supply, and future sites without shelter-specific assumptions.

Big Picture: `building` should cover shelter, production, commercial use, public infrastructure, storage, institutions, and capital equipment. Purpose should become data the player can inspect, not a city-builder control surface.

Logical next step: after purposes are generic and inspectable, add one commercial or capital-purpose site.

## Phase 57: Generic Building Purpose Field

Goal: add a small purpose model that can describe current building roles and future site types.

- [ ] Add or extend a building-purpose helper/data shape for current roles: housing, workplace, supply.
- [ ] Keep existing `MicroZoneRole` behavior intact unless a narrow refactor is required.
- [ ] Avoid adding ownership, zoning policy, rents, taxes, or economic simulation.
- [ ] Add tests that current roles resolve to stable purpose labels.

Acceptance:

- [ ] Building purpose becomes explicit data or a clearly named derived helper.
- [ ] Existing interactions continue to work.
- [ ] Future building types can be added without duplicating shelter-only language.

## Phase 58: Purpose-Aware Building Readouts

Goal: make basic building inspection describe what the site is for.

- [ ] Update housing, workplace, and supply inspection/readouts to include purpose language.
- [ ] Keep action prompts unchanged.
- [ ] Ensure the debugger scan can reuse or enrich the same purpose information.
- [ ] Add tests for purpose-aware readouts on all current building roles.

Acceptance:

- [ ] Buildings are more legible as social/economic sites.
- [ ] The player does not gain management controls.
- [ ] Readouts prepare for commercial and public sites.

## Phase 59: Generic Building Status Language

Goal: reduce shelter-specific wording where the current mechanic is really building improvement or site condition.

- [ ] Rename or wrap player-facing readout helpers so they say `BUILDING`, `SITE`, or another generic term where appropriate.
- [ ] Keep housing-specific text only where the meaning is truly domestic.
- [ ] Do not perform a broad file-wide rename unless needed for the player-facing behavior.
- [ ] Add regression tests for current housing improvement and workplace bench readouts.

Acceptance:

- [ ] The code and UI no longer imply all production outcomes are shelter-only.
- [ ] Existing tiny-loop behavior is unchanged.
- [ ] Future building-purpose sessions have clearer entry points.

## Session: Commercial Or Capital Site

Gameplay outcome: the map contains one non-domestic, non-workplace site that represents commercial or capital purpose.

Big Picture: the city should include places that exist for exchange, storage, finance, machinery, or institutional capital, not only survival shelter and production benches.

Logical next step: after one commercial/capital site exists, add one public-purpose site for contrast.

## Phase 60: One Commercial Site Role

Goal: add one current-scope commercial or capital-purpose site type.

- [ ] Pick a narrow role such as `MARKET`, `DEPOT`, `MACHINE SHOP`, or `CAPITAL NODE`.
- [ ] Place one site in the authored sandbox without overlapping existing buildings or paths.
- [ ] Give it a glyph, color, and basic inspection label.
- [ ] Add placement/non-overlap tests.

Acceptance:

- [ ] The map has a new kind of building purpose.
- [ ] No market prices, buying/selling, ownership, rent, or economy simulation is introduced.
- [ ] The site is visible and inspectable.

## Phase 61: Commercial Site Scan Metadata

Goal: let the inherited debugger reveal one hidden commercial/capital metadata field.

- [ ] Add one scan result such as `PURPOSE: EXCHANGE`, `CAPITAL EQUIPMENT`, or `ACCESS: RESTRICTED`.
- [ ] Show the result through the existing gadget result HUD.
- [ ] Keep metadata static or derived.
- [ ] Add tests for valid scan result and no-target behavior.

Acceptance:

- [ ] The player learns something beyond ordinary inspection.
- [ ] No financial system, faction owner, shop UI, or transaction verb is introduced.
- [ ] The site points toward later economic/social context.

## Phase 62: Commercial Site Boundary

Goal: make clear what the commercial site can and cannot do in this pass.

- [ ] Ensure `E`, `F`, worker delivery, and building improvement do not accidentally treat the site as housing/workplace/supply.
- [ ] Add explicit no-op or unavailable readouts where needed.
- [ ] Add tests for unchanged player and worker loop behavior.
- [ ] Keep the site as observation context only for now.

Acceptance:

- [ ] The new site does not break the tiny production loop.
- [ ] The player can observe it but not manage it.
- [ ] Future sessions can choose one real interaction later.

## Session: Public Site

Gameplay outcome: the map contains one public-purpose site that contrasts with private/commercial building purposes.

Big Picture: public infrastructure should enter as a visible place and purpose first, before transit, utilities, bureaucracy, or surveillance systems become mechanics.

Logical next step: after public and commercial purposes exist, add a simple authority/ownership tag that differentiates site context.

## Phase 63: One Public Site Role

Goal: add one current-scope public-purpose site type.

- [ ] Pick a narrow role such as `CLINIC`, `PUMP`, `PUBLIC TERMINAL`, or `MUNICIPAL NODE`.
- [ ] Place one site in the authored sandbox with visible glyph/color/readout.
- [ ] Keep it non-interactive except for inspection and scanning.
- [ ] Add placement and inspection tests.

Acceptance:

- [ ] Public-purpose space exists in the sandbox.
- [ ] No health system, utility grid, transit network, public works simulation, or bureaucracy is introduced.
- [ ] The site is visible and inspectable.

## Phase 64: Public Site Scan Metadata

Goal: let the debugger reveal one hidden public-purpose field.

- [ ] Add one scan field such as `AUTHORITY: MUNICIPAL`, `SERVICE: PUBLIC`, or `ACCESS: RATIONED`.
- [ ] Reuse the existing gadget result surface.
- [ ] Keep metadata deterministic.
- [ ] Add tests for the scan result and ordinary inspection remaining simpler.

Acceptance:

- [ ] Public context becomes legible without building a public-works system.
- [ ] The player remains an observer/interferer.
- [ ] The metadata prepares later authority tags.

## Phase 65: Public Site Routing Stub

Goal: decide whether the public site needs an inspectable route edge now.

- [ ] If a route is useful, add one pedestrian route/signpost to the public site with no worker behavior attached.
- [ ] If a route is premature, document why the site remains standalone for now.
- [ ] Keep path/signpost logic generic if a route is added.
- [ ] Add tests for the chosen boundary.

Acceptance:

- [ ] The site either has a visible connection or a clear deferral.
- [ ] No transit, service delivery, or NPC schedule is introduced.
- [ ] The next session can add authority/ownership context without route ambiguity.

## Session: Ownership Or Authority Tag

Gameplay outcome: sites can expose a small social context tag such as private, municipal, corporate, illicit, or household.

Big Picture: authority and ownership should begin as inspectable context, not as faction AI or property simulation. This helps the player read the city as a social system.

Logical next step: after site context is visible, connect sites with one dependency edge.

## Phase 66: One Site Context Tag

Goal: add one generic tag field or derived helper for building/site context.

- [ ] Support a small set of labels such as `HOUSEHOLD`, `WORKPLACE`, `PRIVATE`, `MUNICIPAL`, or `COMMERCIAL`.
- [ ] Assign tags to current sites deterministically.
- [ ] Keep tags readout-only.
- [ ] Add tests for all current site tags.

Acceptance:

- [ ] Site context is visible without faction mechanics.
- [ ] Tags are generic enough for future building roles.
- [ ] Existing interactions are unchanged.

## Phase 67: Context-Aware Inspection

Goal: show the site context tag in ordinary inspection or building readout.

- [ ] Add compact readout text such as `AUTHORITY: MUNICIPAL` or `CONTEXT: COMMERCIAL`.
- [ ] Keep debugger scan able to reveal a richer version if useful.
- [ ] Avoid new UI panels.
- [ ] Add tests for ordinary and scan readouts.

Acceptance:

- [ ] The player can distinguish site context at a glance.
- [ ] No ownership transfer, law, rent, faction, or economy system is introduced.
- [ ] Context tags prepare dependency and risk work.

## Phase 68: Context Boundary Tests

Goal: protect against site tags becoming accidental mechanics too early.

- [ ] Add tests proving context tags do not alter collision, entry, carryables, worker route behavior, or save/load unless explicitly supported.
- [ ] Document any intentionally unsupported tag behavior in TODO notes if needed.
- [ ] Keep all tag behavior deterministic.
- [ ] Avoid speculative hooks for future factions.

Acceptance:

- [ ] Tags remain player-facing context only.
- [ ] Future mechanics can opt in deliberately.
- [ ] The code remains small and current-scope.

## Session: One Dependency Edge

Gameplay outcome: one site visibly depends on another site or route, and the player can inspect that dependency.

Big Picture: city flow should start with visible edges, not hidden graphs. Materials, access, energy, information, money, and authority can become flows later if the first edge is legible.

Logical next step: after one dependency edge exists, make route purpose readouts explain what flows across it.

## Phase 69: Dependency Edge Data

Goal: represent one dependency between existing sites without building a graph engine.

- [ ] Choose one edge such as `WORKPLACE DEPENDS ON SUPPLY` or `BUILDING DEPENDS ON PART`.
- [ ] Store or derive the edge in a small helper/data shape.
- [ ] Keep it readout-only in this phase.
- [ ] Add tests for dependency resolution and missing-target behavior.

Acceptance:

- [ ] One dependency is explicit and inspectable.
- [ ] No city-scale dependency graph, planner, or optimization UI is introduced.
- [ ] Existing loop mechanics remain unchanged.

## Phase 70: Dependency Inspection Readout

Goal: show the dependency on the involved site or route.

- [ ] Add text such as `DEPENDS ON: SUPPLY` or `SUPPORTS: WORKPLACE`.
- [ ] Ensure the readout is available through ordinary inspection.
- [ ] Keep debugger scan optional/enriching rather than required.
- [ ] Add tests for source and target readouts.

Acceptance:

- [ ] The player can understand a simple dependency in-world.
- [ ] No management view or dependency editor is introduced.
- [ ] The edge prepares route-purpose work.

## Phase 71: Dependency Boundary

Goal: define what the dependency edge does not do yet.

- [ ] Add tests proving the edge does not change worker routing, item pickup, or building improvement unless existing mechanics already do.
- [ ] Keep any blockage/readiness behavior derived from current loop state.
- [ ] Document deferred graph behavior in TODO notes if useful.
- [ ] Avoid speculative dependency components for every future flow type.

Acceptance:

- [ ] The dependency is readable, not managerial.
- [ ] Current gameplay remains deterministic.
- [ ] The next session can focus on route purpose text.

## Session: Route Purpose Readouts

Gameplay outcome: paths and signposts explain what they connect and what currently moves or could move along them.

Big Picture: route purpose turns movement infrastructure into readable city flow. This supports future material, labor, access, and information flows without adding a planner.

Logical next step: after route purpose is visible, add one flow blockage that follows from an existing player action.

## Phase 72: Path Purpose Helper

Goal: derive a compact purpose label for each current pedestrian path.

- [ ] Use connected building/site roles to derive labels such as `LABOR ROUTE`, `SUPPLY ROUTE`, or `PUBLIC ACCESS`.
- [ ] Keep labels deterministic and readout-only.
- [ ] Add tests for housing-workplace, workplace-supply, and any new site route.
- [ ] Do not add pathfinding or route assignment behavior.

Acceptance:

- [ ] Paths have readable purpose.
- [ ] The purpose follows existing map topology.
- [ ] No route planner is introduced.

## Phase 73: Signpost Purpose Text

Goal: make signposts show route purpose in addition to destination.

- [ ] Extend signpost readout with `ROUTE: ...` or `CARRIES: ...`.
- [ ] Preserve existing direction glyph behavior.
- [ ] Ensure spoofed signpost text still reads coherently.
- [ ] Add tests for normal and spoofed signpost readouts.

Acceptance:

- [ ] The player can understand both destination and function.
- [ ] Existing spoof behavior remains intact.
- [ ] No new controls are introduced.

## Phase 74: Route Scan Enrichment

Goal: let the debugger reveal one hidden route-purpose detail.

- [ ] Add one scan result such as `EXPECTED CARGO: SUPPLY/PART` or `ACCESS: WORKER ONLY`.
- [ ] Reuse the current gadget result HUD.
- [ ] Keep scan results volatile unless a later save need appears.
- [ ] Add tests for path and signpost scan results.

Acceptance:

- [ ] The debugger adds hidden context rather than duplicating ordinary inspection.
- [ ] No route map, minimap, or traffic system is introduced.
- [ ] Flow blockage work has clear route vocabulary.

## Session: Flow Blockage

Gameplay outcome: one existing player action creates a visible blockage in a site or route flow.

Big Picture: flow systems should emerge from concrete interruptions the player can perform, not from abstract simulation breadth.

Logical next step: after one blockage is visible, add a recovery path and decide whether it persists.

## Phase 75: One Flow Blockage Trigger

Goal: choose one current verb that creates a readable flow blockage.

- [ ] Use an existing action such as carrying expected supply, holding finished part, or spoofing a route signpost.
- [ ] Derive one blockage label from that state.
- [ ] Keep the trigger local and reversible.
- [ ] Add tests for blockage appearing from the chosen action.

Acceptance:

- [ ] The player can create a visible blockage using existing verbs.
- [ ] No new job system, economy, or crisis state is introduced.
- [ ] The blockage follows existing world state.

## Phase 76: Site And Route Blockage Readouts

Goal: show the blockage where the player would look for it.

- [ ] Add readout text to the affected site, path, signpost, or worker.
- [ ] Ensure text names the missing/blocked flow.
- [ ] Keep the HUD compact.
- [ ] Add tests for each affected readout.

Acceptance:

- [ ] The blockage is inspectable from the relevant object.
- [ ] No alert feed or objective marker is introduced.
- [ ] The player can reason from visible symptoms.

## Phase 77: Blockage Does Not Spread

Goal: make the first flow blockage explicitly local.

- [ ] Add tests proving unrelated sites and routes remain normal.
- [ ] Avoid adding cascading city-wide effects.
- [ ] Keep future cascade work deferred until multiple local flows exist.
- [ ] Document the boundary if needed.

Acceptance:

- [ ] The flow blockage has a small blast radius.
- [ ] The game avoids premature systemic breadth.
- [ ] Recovery work can target one concrete condition.

## Session: Flow Recovery

Gameplay outcome: the player can restore the blocked flow through a small, current-scope action.

Big Picture: recovery should make systems feel organic and reversible. The city continues when conditions are restored, rather than waiting for player commands.

Logical next step: after local flow interruption and recovery work, consider local risk only if the player-facing loop needs stakes.

## Phase 78: One Recovery Condition

Goal: define exactly how the selected blockage clears.

- [ ] Use a current action such as dropping/returning supply, restoring a signpost, delivering a part, or clearing a held output.
- [ ] Ensure the worker or loop resumes from existing state.
- [ ] Add tests for blocked and recovered behavior.
- [ ] Keep recovery deterministic.

Acceptance:

- [ ] The player can restore the flow without a reset.
- [ ] The city resumes organically.
- [ ] No repair minigame or management command is introduced.

## Phase 79: Recovery Readout

Goal: make recovery visible through the same objects that showed the blockage.

- [ ] Add compact text such as `FLOW: CLEAR`, `RECOVERED`, or restored normal labels.
- [ ] Ensure stale blockage text clears.
- [ ] Add tests for readout transitions.
- [ ] Keep the HUD/readout surfaces small.

Acceptance:

- [ ] The player can tell the flow recovered.
- [ ] No separate log or journal is required.
- [ ] Future risk work can reference resolved/unresolved local states.

## Phase 80: Recovery Persistence Boundary

Goal: decide whether the blockage/recovery state belongs in tiny save now.

- [ ] If the state is derived from already-saved data, add tests proving save/load restores it.
- [ ] If new persistence is needed, add only the smallest required state.
- [ ] If persistence is premature, document the deferral.
- [ ] Do not add a generalized event or history system.

Acceptance:

- [ ] Save/load behavior is explicit and deterministic.
- [ ] Flow recovery stays current-scope.
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
