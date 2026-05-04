# Neon Oubliette Implementation Plan

This file is the singular source of truth for active implementation order. Completed slices are archived in `todo/completed_todo.md`.

For longer-range direction, read `todo/EPOCHS.md` before adding new sessions. `todo/EPOCHS.md` is a proposal and north-star guide, not an implementation contract. If a proposed epoch or session jumps too far ahead, no longer fits the simulation-exploration vision, or would turn the game into player-management work, push back and suggest a smaller or different direction before editing this TODO.

## Binding Rules

- Build from observed NPC behavior outward. Add player controls only when observation alone is insufficient.
- Every system must have a visible symptom in the game world: NPC state, city motion, environment change, or real-time symbol.
- NPC behavior must be autonomous. NPCs act from their own rules; the player does not assign, command, or manage them.
- Player verbs are limited to: `SPACE` (inspect static objects), `T` (talk to one nearby NPC), `F` (pick up / drop one object), `E` (enter/exit sites, board transit), `G` (use carried item on a target). No new management, delivery, or apply-to-building verbs.
- Every new component/system must be current-scope, tested, and wired into rendering or readouts before expanding scale.
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

- Authored sandbox scope: two player-visitable districts connected by transit, each with housing, workplace, supply, market, clinic, signposts, and config-driven fixed workers following derived pedestrian paths.
- Core player verbs: `SPACE` inspects static objects (buildings, signposts, paths, sites, workers) and opens the Debugger terminal overlay; `T` talks to one nearby NPC (stub); `F` picks up or drops the single carried object; `E` enters/exits sites and boards/exits transit; `G` uses the carried item on a nearby target (no current targets — stub only); `F5`/`F9` save/load.
- Worker loop: one or two fixed workers per district follow derived pedestrian paths autonomously — moving supply to workplace bench, producing output, and returning. Worker inspection shows current state and district tag.
- Transit: paired district stations advance through elapsed-time `ARRIVING`/`BOARDING`/`DOORS CLOSING`/`DEPARTED` states; timing persists through save/load.
- Day/night phase: two-state `DAY`/`NIGHT` cycle on a configurable elapsed-time interval; `NIGHT` halves witness range for output theft; phase persists through save/load.
- Witnessed theft: if the player takes the ready workplace output while a worker is close enough, a local suspicion record is created; inspectable via `SPACE` on the worker and workplace; persists through save/load.
- AI playtest surface: `neon_ai_playtest` exposes adaptive terminal snapshots with commands, target details, action prompts, Debugger results, system state, a centered text map, and interactive transcripts.

## Active Sessions

## Session: Verb Cleanup — Cut Management Actions

Gameplay outcome: player verbs are reduced to observation and movement only; all management-oriented verbs, disruption mechanics, apply-to-building actions, and their orphaned state are removed. Build remains clean and all remaining verbs still work.

Scope guard: this session removes mechanics only. No new player verbs or NPC features until the NPC Emotion Surface session.

## Phase 108: Remove T lay-low and all G disruptions.

- [ ] Remove `T` lay-low: shelter supply consumption, `LAID_LOW` suspicion resolution state. `T` becomes a stub showing a short talk prompt near an NPC and nothing otherwise.
- [ ] Remove all `G` disruption targets and their derived state: signpost spoof/restore (and all `FLOW: BLOCKED` / `FLOW: CLEAR` / `SUPPLY FLOW: DISRUPTED` readouts), wage record spoof/restore (`DOCK RISK: CLEARED`, `WAGE RECORD: SPOOFED`), clinic ghost clearance spoof/restore (`CLINIC ACCESS: GHOST CLEARANCE`), dependency disruption/restore.
- [ ] Stub `G` as "use carried item on target": when nothing nearby accepts an item, show a short no-op message (e.g. `NO TARGET`). Keep the verb key and the `inheritedGadgetCanSpoofTarget` call site as a hook for future item-use targets.
- [ ] Remove suspicion resolution states `CORRECTED_ROUTE` and `RETURNED_OUTPUT` (both required now-removed verbs).
- [ ] Acceptance: clean build; `SPACE` inspection and witnessed-theft suspicion still work; tests pass.

## Phase 109: Remove E apply-to-buildings and orphaned state.

- [ ] Remove `E` apply-items-to-buildings actions: bench stocking, PART delivery to housing, return stolen PART to workplace, hide item in housing, shelter listing mark-interest, market exchange verb (PART → SUPPLY / ration claim).
- [ ] Remove orphaned state and readouts: housing improvement flags (`BENCH STOCKED`, `OUTPUT READY`, `BUILDING IMPROVED`), shelter stock counter, hidden item state, shelter listing entity and sign (placement, inspection, interest-mark readouts).
- [ ] Worker output stays at the workplace as an inspectable depot state after production — no delivery to housing required. The player can observe it with `SPACE`; the worker loop still runs autonomously.
- [ ] Acceptance: worker autonomous loop still runs and is fully inspectable; no dead code referencing removed state; all tests pass.

## Phase 110: Update AI playtest harness and run interactive playtest.

- [ ] Remove all playtest harness references to cut verbs and readouts: lay-low, G interference, signpost spoof, wage spoof, clinic clearance, market exchange, shelter listing, apply-to-building prompts, `FLOW: BLOCKED` / `CORRECTED_ROUTE` / `RETURNED_OUTPUT` strings.
- [ ] Update target-detail and scenario text to reflect the current verb set: SPACE, T (talk), F (carry), E (move/transit), G (no-op stub).
- [ ] Acceptance: `cmake --build build`, `rtk test ctest --test-dir build --output-on-failure`, and an interactive terminal playtest all pass showing SPACE/E/F/T/G each working correctly.

## Session: NPC Emotion Surface

Gameplay outcome: NPCs display real-time emotion symbols above their glyphs without player input, making NPC state and reactions visible at a glance as a foundation for all future NPC-to-NPC interaction.

Epoch source: `NPC Simulation Foundation` in `todo/EPOCHS.md`.

Scope guard: symbols only — no speech text, no dialogue system, no conversation parsing. Symbols must fade automatically with no player input required.

## Phase 111: Add NPCEmotionComponent and symbol rendering.

- [ ] Add `NPCEmotionComponent` with a symbol enum (NEUTRAL, STARTLED, AGGRESSIVE, COMMERCE, CONFUSED, DISTRESSED) and a `float fade_timer` (symbols auto-fade after ~2s; NEUTRAL shows nothing).
- [ ] Render the active symbol as a single character one tile above the NPC glyph in the existing glyph pass, only when non-neutral and within the viewport.
- [ ] Symbol characters: `!` startled, `@` aggressive, `$` commerce, `?` confused, `~` distressed.
- [ ] Acceptance: a test entity with a non-neutral emotion shows the symbol above its glyph; neutral shows nothing; fade timer clears the symbol after the configured duration; tests cover set, render, and fade.

## Phase 112: Wire first emotion triggers.

- [ ] `!` (startled): trigger on the worker when a witnessed-theft suspicion record is created.
- [ ] `?` (confused): trigger on the worker when they cannot progress in the autonomous loop (no supply available, bench blocked, etc.) — reuse or derive from existing blocked-worker readout conditions.
- [ ] Acceptance: theft witness event sets `!` on the affected worker; blocked loop condition sets `?`; both fade after ~2s; no symbol appears without a qualifying trigger; tests cover both.

## Phase 113: Update AI playtest harness for emotion symbols.

- [ ] Expose active NPC emotion symbols in AI playtest target details and terminal map snapshots.
- [ ] Acceptance: interactive terminal playtest demonstrates a witnessed-theft event followed by visible `!` on the worker fading after 2s; AI harness reports the symbol in target details.

## Session: Production Loop As City System

Gameplay outcome: the worker production loop serves the city rather than the player's housing. Output accumulates at an inspectable workplace depot, and citizens can express interest in it through commerce symbols.

Epoch source: `NPC Simulation Foundation` in `todo/EPOCHS.md`.

Scope guard: no full economy, no buying/selling mechanics, no NPC inventory. Output is a depot state that future sessions can build on. No new player verbs.

## Phase 114: Worker output stays at workplace as inspectable depot.

- [ ] After producing output at the bench, the worker keeps it at the workplace as a depot state (not delivered to housing). `SPACE` on the workplace shows what output is available.
- [ ] Confirm the housing-improvement chain is fully removed (from Phase 109). Worker loop is: supply → bench → output at workplace depot → worker returns to supply.
- [ ] Acceptance: worker loop runs autonomously and is inspectable end-to-end; depot state visible via SPACE; no housing improvement state remains; tests pass.

## Phase 115: Citizens express interest in depot output.

- [ ] When output is available at the workplace depot and a citizen-type NPC is within proximity range, trigger `$` (commerce) on the citizen.
- [ ] Keep interest as a symbol only — no transfer, no inventory, no transaction in this phase.
- [ ] Acceptance: `$` appears on a citizen within range of a stocked depot; clears when depot is empty or citizen moves away; tests cover stocked and empty cases.

## Phase 116: Worker emotion symbols during production loop and playtest.

- [ ] Worker shows `?` (confused) when waiting at supply with nothing to pick up (supply unavailable).
- [ ] Worker shows neutral while on route and working bench.
- [ ] Update AI playtest harness to cover depot inspection and citizen `$` trigger.
- [ ] Acceptance: interactive terminal playtest demonstrates visible worker symbol states across the production loop.

## Session: Second NPC Type — Guard

Gameplay outcome: a guard NPC patrols and observes other NPCs; guard-to-NPC interactions are visible in real time through emotion symbols, giving the player their first view of the city watching its own citizens.

Epoch source: `Surveillance — The City Watches NPCs` in `todo/EPOCHS.md`.

Scope guard: one guard type, one district, symbols only. No laws, no pursuit, no jail, no faction system, no camera networks.

## Phase 117: Add guard NPC type with patrol route.

- [ ] Add a guard NPC spawned by config alongside workers. Guard follows a fixed patrol route within the district.
- [ ] Guard inspection via `SPACE` shows role, route, and currently observed target (if any).
- [ ] Acceptance: guard spawns, patrols, and is inspectable; guard count is config-driven; save/load preserves guard position; tests cover spawn, route, and inspection.

## Phase 118: Guard observes workers and citizens; shows emotion symbols.

- [ ] Guard maintains a current observation target: the nearest anomalous NPC (worker with active suspicion record, or citizen with `~` distress symbol).
- [ ] Guard shows `!` (startled) when it first detects an anomaly; `@` (aggressive) when it moves toward the anomaly.
- [ ] NPC being observed shows `~` (distressed) when the guard is within close range.
- [ ] Acceptance: guard-to-NPC interaction is visible through symbols in real time without player input; tests cover detection, approach, and distress trigger.

## Phase 119: SPACE on guard reveals observation target; AI playtest.

- [ ] `SPACE` on the guard shows its current observation target (role, district tag, reason flagged).
- [ ] Update AI playtest harness so the terminal map shows guard glyph and harness can inspect the guard and report its target.
- [ ] Acceptance: interactive terminal playtest demonstrates guard patrol, anomaly detection, visible symbol exchange, and guard inspection via SPACE.
