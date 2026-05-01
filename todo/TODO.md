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

- Authored sandbox scope: one player, one or two authored districts, paired transit stations, and per-district housing/workplace/supply/market/clinic clusters with derived lit pedestrian paths/signposts and config-driven fixed workers.
- Core verbs: `E` enters/exits sites, boards/exits transit, applies carried items to housing/workplace state, returns suspected output, or hides a suspected item in housing; `T` talks to/acknowledges a nearby worker or lays low inside stocked housing; `F` picks up or drops the single carried object; `SPACE` runs Debugger inspection on nearby buildings, paths, signposts, station, and worker targets; `G` uses the Interference Torch to spoof/restore route signposts, disrupt/restore the workplace/supply dependency, spoof/restore one worker wage record, or spoof/restore clinic access when a local worker record exists; `F5`/`F9` save/load the tiny current scope.
- Production loop: player and worker both operate the same `SUPPLY` -> stocked bench -> output-ready bench -> carried `PART` -> improved housing chain, with item labels derived from the carried kind.
- Worker loop: fixed workers follow derived pedestrian paths through supply, workplace, and housing tasks inside their own district; route endpoints expose readable `TO ...`, `LABOR ROUTE`, `SUPPLY ROUTE`, district tags, and carried-flow labels.
- Hidden-system surface: Debugger inspection reveals labor pressure, route quota, site purpose, market access, clinic ration/access context, clinic ledger flags, dependency state, expected cargo, local suspicion witness detail, recoverable workplace log fragments, wage/dock risk, and access details, but scan memory remains volatile HUD-only state.
- Interference loop: spoofed signposts and disrupted dependencies are inspectable, district-local, pause affected worker/supply flow, show local `FLOW: BLOCKED` or `SUPPLY FLOW: DISRUPTED` readouts, and persist active blockage state through tiny save/load; restored routes show local `FLOW: CLEAR`, with recovery acknowledgement intentionally volatile.
- Local risk loop: a worker can witness expected `PART` theft or nearby `G` Interference Torch route tampering inside its district, producing a current-scope `LOCAL NOTICE` HUD line; day/night phase and nearby worker crowding shrink witness range, while active, returned, corrected, hidden, laid-low, and audit-traced suspicion states are inspectable and persist through tiny save/load without adding surveillance, wanted level, factions, or pursuit.
- Wage clue loop: debugger scan of the worker reveals `WAGE IMPACT: INCIDENT LOGGED; DOCK RISK: ACTIVE` when a suspicion record exists; `G` on the worker with a record toggles `wage_record_spoofed`, changing the scan to `DOCK RISK: CLEARED`; spoofed state persists through save/load (V10).
- Clinic access loop: clinic inspection and Debugger scan show the inert layout plus one `RECORDS STAFF ONLY` boundary; ordinary `E` at the clinic reports `CLINIC ACCESS DENIED`, while a local worker record allows `G` to toggle volatile `CLINIC ACCESS: GHOST CLEARANCE`, open that one boundary until restored or tiny reload, and produce a distinct worker mismatch readout without clearing other local risk state.
- AI playtest surface: `neon_ai_playtest` exposes adaptive terminal snapshots with commands, target details, action prompts, Debugger results, system state, a centered text map, and interactive transcripts. Gameplay feature completion must keep this surface current and include a live terminal playtest.

## Active Sessions:

## Session: Commerce — First Market Variety And Trade Hook

Gameplay outcome: markets start to differ by district or neighborhood pressure, and the player gets one small exchange action that makes commerce visible without introducing money or a full economy.

Epoch source: `Commerce, Rations, And Exchange Pressure` in `todo/EPOCHS.md`.

Scope guard: this session may add market category labels and one local exchange flag. It must not add wallets, prices, vendors, multi-slot inventory, broad supply chains, factories/refineries, truck logistics, or NPC shopping routines.

## Phase 99: Add authored market category labels.

- [ ] Give each authored market a compact category such as ration, local goods, or luxury preview, derived from district/site config.
- [ ] Show category, access pressure, and intended customer hint through existing inspection and Debugger readouts.
- [ ] Keep market category data deterministic and current-scope.
- [ ] Acceptance: one-district and two-district configs expose stable market category labels in tests and AI playtest target details.

## Phase 100: Add one market exchange verb.

- [ ] Let the player use one carried current-scope item at a market for a deterministic local result, such as exchanging a `PART` for one `SUPPLY` or marking a ration claim.
- [ ] The result must be visible on market inspection and not require a wallet, price table, vendor AI, or inventory expansion.
- [ ] Failed exchange attempts report exactly why they failed.
- [ ] Acceptance: tests cover valid exchange, invalid carried item, empty-handed attempt, and post-exchange readout.

## Phase 101: Add market playtest coverage and persistence boundary.

- [ ] Persist the exchange result only if it should survive save/load; otherwise keep it as volatile local feedback and state that clearly in the readout.
- [ ] Update the AI playtest harness so it can identify market type, attempt the exchange, and report the result.
- [ ] Acceptance: interactive terminal playtest demonstrates a market readout, one exchange, and the resulting changed local state.

## Session: Transit — First Scheduled Train Signal

Gameplay outcome: the existing paired stations gain a readable real-time train signal, so transit starts to feel like city motion without adding traffic, route planning, or independent vehicles.

Epoch source: `Complex transit` in `todo/EPOCHS.md`.

Scope guard: this session may add a station countdown/status and one wait/board consequence. It must not add road networks, traffic jams, autonomous cars, vehicle ownership, repair, freight logistics, or train networks.

## Phase 102: Add station arrival status.

- [ ] Give each station an inspectable train status such as arriving, boarding, doors closing, or departed using elapsed runtime rather than frame count.
- [ ] Show the status in station inspection, map/header details, and AI playtest target details.
- [ ] The first implementation may keep the existing direct ride path; the new value is readable timing.
- [ ] Acceptance: tests cover deterministic status advancement under controlled elapsed time.

## Phase 103: Make waiting at the station matter once.

- [ ] Add one player-facing consequence for waiting until the favorable station state, such as immediate open-door boarding or a clearer destination readout.
- [ ] Attempting to board at the wrong state produces a short readable result and does not strand the player.
- [ ] Acceptance: tests cover board-now, wait-then-board, and cross-district return behavior.

## Phase 104: Preserve transit readability in save/load and playtest.

- [ ] Save/load preserves any new train timing state that affects player choices.
- [ ] Update the AI playtest harness to report station status and choose a wait/board action.
- [ ] Acceptance: interactive terminal playtest demonstrates reading station status, waiting or boarding, and reaching the paired district.

## Session: Shelter — First Real Estate Listing Hook

Gameplay outcome: shelter starts pointing toward city-scale housing pressure through one visible listing/agent surface, while the player's current home remains the only actual base.

Epoch source: `Shelter` in `todo/EPOCHS.md`.

Scope guard: this session may add listing labels, one inspection surface, and one reversible mark/interest flag. It must not add money, moving services, multiple owned homes, storage transfer, rent simulation, furniture, or property markets.

## Phase 105: Add one real estate listing surface.

- [ ] Place or derive one local listing/agent sign tied to the current district's housing tier.
- [ ] Inspection shows a compact shelter type, neighborhood pressure, and a reason the listing is currently unreachable or risky.
- [ ] The listing should point toward future housing variety without changing the player's home base yet.
- [ ] Acceptance: tests and AI playtest target details expose the listing in the current district.

## Phase 106: Add one player mark-interest verb.

- [ ] Let the player use a current input on the listing to mark or clear interest in the shelter.
- [ ] The mark changes the listing readout and may add one Debugger line about records, clearance, or price pressure.
- [ ] The mark does not move the player, transfer belongings, charge money, or unlock a new home.
- [ ] Acceptance: tests cover mark, clear, and district-local readouts.

## Phase 107: Add shelter listing playtest coverage.

- [ ] Update the AI playtest harness to discover the listing, inspect it, toggle the mark, and report the changed readout.
- [ ] Keep the current shelter storage and lay-low mechanics unchanged.
- [ ] Acceptance: interactive terminal playtest demonstrates the listing loop without broad housing simulation.

## Session: Surveillance — One Local Watcher Surface

Gameplay outcome: surveillance becomes visible as one authored local watcher/audit surface the player can inspect and spoof, without turning suspicion into a global wanted level.

Epoch source: `Surveillance` in `todo/EPOCHS.md`.

Scope guard: this session may add one visible drone/robot/audit marker and one local record flag. It must not add pursuit, combat, jail, faction reputation, laws, patrol AI, camera networks, or citywide surveillance.

## Phase 108: Add one authored watcher marker.

- [ ] Place or derive one local watcher marker near an existing public or commercial site.
- [ ] Inspection identifies who or what the watcher observes using compact AGI/faction-flavored text without activating faction systems.
- [ ] The marker is inert until tied to an existing current-scope local record such as suspicion, route tampering, clinic access, or market exchange state.
- [ ] Acceptance: tests and AI playtest target details expose the watcher marker and its observed scope.

## Phase 109: Add one spoofable watcher record.

- [ ] Let `G` on the watcher mask or restore exactly one local observed record.
- [ ] Masking changes watcher/readout text but does not clear the underlying suspicion, wage, clinic, market, or route state unless the player uses the existing matching verb.
- [ ] The result must be readable on watcher inspection and Debugger scan.
- [ ] Acceptance: tests cover mask, restore, no-record attempt, and district-local isolation.

## Phase 110: Add watcher persistence and playtest coverage.

- [ ] Persist the watcher mask state if it affects future readouts after save/load.
- [ ] Update the AI playtest harness to find the watcher, inspect it, mask/restore the record, and report the changed text.
- [ ] Acceptance: interactive terminal playtest demonstrates a watcher readout, one mask, one restore, and no global enforcement response.
