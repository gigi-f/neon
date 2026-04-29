# Neon Oubliette Epoch Roadmap

This file proposes future session groups for `todo/TODO.md`. It is directional context, not a rigid queue.

Future agents should use this file to keep the end-game shape in mind while still implementing one small, visible session at a time. If an epoch or session jumps too far ahead, no longer fits the vision, or would push the game toward city-builder/player-management mechanics, push back and propose a smaller or different direction before writing it into `todo/TODO.md`.

## Session Design Directives

- **Vertical slices over horizontal layers.** A session that adds a site trace, its context, one consequence, and one player verb is better than separate sessions for tags, readouts, persistence, and tests.
- **Every session must include a player verb or interference opportunity.** Observation-only sessions that add inspectable labels with no new action are not allowed. If a proposed session has no verb, restructure it before writing it into `todo/TODO.md`.
- **Fold readouts and boundaries into mechanic phases.** Inspection text, scan metadata, AI playtest visibility, and persistence/test boundaries are acceptance criteria of the phase that introduces the mechanic.
- **No decision-only phases.** "Decide whether X needs Y" is a design question, not an implementation phase. Resolve it in the prior phase's acceptance criteria or a TODO note.
- **Keep AI playtesting current.** Any new verb, target, prompt, map glyph, scenario, persistence state, or readout that an AI tester needs must be reflected in the terminal playtest harness as part of the same gameplay slice.
- **Pace check: every 3 context-heavy sessions must be followed by 1 action-heavy session.** If the plan accumulates 3+ sessions of clues/readouts without a new interference verb, restructure before implementing.

## Current Completed Arc

Do not re-add these as future work. They are already archived in `todo/completed_todo.md`.

- The authored sandbox loop exists: housing, workplace, supply, market, clinic, derived paths, signposts, one fixed worker, one carried object, tiny save/load, and first-pass interiors for current enterable sites.
- The supply/workplace/housing loop exists for both player and worker, including supply pickup, stocked bench, output-ready part, housing improvement, and worker blocking/recovery readouts.
- The inherited tool exists as the Debugger/Interference Torch, with scan/readout surfaces, signpost spoofing, dependency disruption, flow blockage/recovery, and wage-record spoofing.
- Local risk exists without surveillance: witnessed output theft and route tampering create local suspicion; the player can return, correct, hide, persist, inspect, and recover one institutional log trace.
- The first hidden labor-control clue exists: worker wage/dock risk appears from local suspicion and can be spoofed locally without adding an economy or faction system.

## North Star Frame

Neon Oubliette is about a young girl with inherited illicit tools moving through systems she did not build: shelter, labor, supply, commerce, public works, scarcity, coercion, surveillance, and eventual escape pressure. The player should not become a mayor, manager, production planner, or commander.

The city should feel like a neon-lit oubliette: visible routes, rationed access, institutional records, public surfaces with private control behind them, and fragile shelter as the player's only stable place. The player learns by inspecting, carrying, hiding, spoofing, returning, correcting, and choosing when to help, steal, expose, exploit, or disappear.

Implementation order should follow this ladder:

- Visible local symptom.
- One player verb or interference opportunity.
- A readable consequence on the affected thing.
- A tight persistence/playtest boundary if the state matters.
- Wider simulation only after multiple local hooks prove the need.

## Epoch: Municipal Access And Public Control

Gameplay outcome: public infrastructure stops being only scenery and becomes one local control surface the player can inspect and spoof.

Big Picture: the clinic already represents rationed municipal service. The next step is not a health system; it is a tiny access record that shows public care is controlled by institutional state.

Good sessions:

- Session: Clinic Access Ledger
- Session: Municipal Route Trace
- Session: Public Service Denial Boundary

Likely focus:

- Tie one clinic readout to existing suspicion/wage-record state.
- Let `G` spoof or restore exactly one clinic access marker.
- Show the result on clinic inspection, worker Debugger scan, and the AI playtest terminal map/readout.
- Do not add health, medicine, injury recovery, appointments, doctors, faction ownership, or citywide public-works simulation.

## Epoch: Commerce, Rations, And Exchange Pressure

Gameplay outcome: the market becomes a readable pressure surface without becoming a shop.

Big Picture: commerce should reveal coercion before it offers buying and selling. Market access can explain why labor output, supply, and records matter.

Good sessions:

- Session: Market Ration Ledger
- Session: Cargo Value Trace
- Session: Spoofed Exchange Flag

Likely focus:

- Add one market ledger or ration readout tied to a carried supply/part or worker wage clue.
- Let `G` alter a single local market access flag or cargo label.
- Keep `E` unavailable at the market unless a concrete current-scope action exists.
- Do not add prices, wallets, barter, vendors, inventory slots, economy simulation, or ownership transfer.

## Epoch: Mother's Tool Origin

Gameplay outcome: using the inherited tool starts exposing why it exists and what it used to touch.

Big Picture: the mother should enter through tool behavior and local artifacts, not exposition dumps. The player can learn by calibrating, scanning, and cross-checking traces at shelter and institutional sites.

Good sessions:

- Session: Housing Calibration Note
- Session: Tool Origin Trace
- Session: One Cost Of Spoofing

Likely focus:

- Gate one shelter interior clue behind a recovered institutional trace.
- Let `G` or `SPACE` at the shelter produce a deterministic calibration result.
- Use the clue to improve AI playtest goals and target descriptions.
- Do not add cutscenes, quest logs, dialogue trees, family relationship systems, or a campaign branch.

## Epoch: Shelter, Scarcity, And Care

Gameplay outcome: the shelter becomes a place for a small irreversible-feeling choice without turning into base management.

Big Picture: the player needs reasons to return home that are not production optimization. Scarcity should be local and emotional before it becomes systemic.

Good sessions:

- Session: Shelter Buffer Choice
- Session: One Care Use For Stored Supply
- Session: Hidden Evidence Cost

Likely focus:

- Use the existing one-supply stock and one-item carry model.
- Add one shelter-local choice that consumes, hides, or marks a single resource.
- Show the consequence in housing inspection and terminal playtest snapshots.
- Do not add hunger, health meters, comfort scores, crafting recipes, furniture systems, or resource economy loops.

## Epoch: Soft Surveillance Seeds

Gameplay outcome: surveillance begins as local audit surfaces the player can see and interfere with, not as a global wanted level.

Big Picture: local suspicion and institutional logs have proven the boundary. Surveillance should grow from those records: audit nodes, access ledgers, service logs, and worker records.

Good sessions:

- Session: One Local Audit Node
- Session: Masked Record Boundary
- Session: Watcher Readout Without Pursuit

Likely focus:

- Attach one audit node to an existing site or signpost.
- Let `SPACE` reveal what it records and `G` mask/unmask exactly one local record.
- Keep local suspicion distinct from active surveillance and keep pursuit out of scope.
- Do not add cameras across the map, line-of-sight detection, wanted levels, law enforcement, alerts, chases, or faction response.

## Epoch: Escape Pressure And Control Systems

Gameplay outcome: the player eventually sees that the local systems are gates in a larger enclosure.

Big Picture: escape pressure should emerge after labor, shelter, commerce, public access, and audit records all have local hooks. The first escape-facing work should still be one gate, one clue, one verb.

Good sessions:

- Session: Boundary Gate Clue
- Session: Transit Permission Trace
- Session: One Exit Signal Spoof

Likely focus:

- Reuse existing path/signpost/access mechanics before adding transit.
- Make the first boundary inspectable and locally spoofable.
- Keep the goal ambiguous and grounded in the microcity.
- Do not add a world map, route planner, vehicles, train schedules, endgame sequence, or broad narrative state until the local gate is playable.

## Idea Pool

This is a sourcebook of high-impact, on-vision verbs and tonal hooks. **Do not implement from this pool directly.** When designing the next session, pull one or two ideas, shape them into a vertical slice (visible symptom → player verb → readable consequence), and write that slice into `todo/TODO.md`. If a candidate idea drifts toward management, optimization, or simulation breadth, drop it.

Ideas that name an existing epoch in parentheses are obvious feeders for that epoch, but anything here can be promoted into a new session or new epoch when its prerequisites are visible.

### Stealth, Cover, And Disappearance

- **Hide in housing during heat.** Add a "lay low" verb in housing that consumes time-of-day ticks and decays an active suspicion intensity without re-triggering the original LOCAL NOTICE. (feeds: Shelter, Scarcity, And Care; Soft Surveillance Seeds)
- **Crowd camouflage.** When more than one fixed actor is within witness range of the player, halve that range. Foreshadows pedestrian density without spawning crowds.
- **Worn worker uniform.** Pick up a discarded uniform near a workplace; while worn, the next single witness check treats the player as another worker. One charge per pickup, deterministic placement.
- **Steam vent cover.** Stand on a vent tile to obscure the player glyph for a short window — useful only during an active suspicion event.
- **Rooftop hop.** Contextual verb on a path tile abutting a building moves the player above-witness for one segment. No vertical movement system, just a one-tile change.
- **Maintenance vent.** Authored alternate entry to one building, hidden behind a glyph and gated by time-of-day. Demonstrates that public access is not the only access.

### New Interference Verbs

- **Forge route credential.** `G` on a signpost while carrying a stolen `PART` stamps a fake route claim — the worker follows it for one trip, then it lapses. Teaches that interference can be temporary and propagating.
- **Plant false evidence.** Drop a carried object inside a building it didn't come from; the next witness check shifts suspicion target onto that building's worker rather than the player.
- **Quiet return at night.** Returning a stolen part during the dark phase clears suspicion silently; doing it in daylight leaves a `RECONCILED` trace. Rewards rhythm, not speed.
- **Cut a streetlight.** `G` on a path-state node toggles `LIT → DARK`. While dark, that path's witness range halves and worker movement slows.
- **Timer-jam a signpost.** A spoof variant that auto-reverts after a short window. Gives the player a propagation experiment without long-term commitment.
- **Reroute a supply stash.** `G` on a stash redirects the next pickup endpoint so a delivery arrives at the wrong workplace. Foreshadows a logistics layer without simulating one.
- **Bribe-to-forget.** Drop a `SUPPLY` near a worker with active suspicion; if their dock-risk is high enough, suspicion clears and a `BRIBED` trace appears on later inspection. (feeds: Soft Surveillance Seeds)

### Observation Verbs (Debugger Extensions)

- **Eavesdrop on a worker.** Hold `SPACE` near a worker for N seconds to overhear one authored mutter ("they docked me again", "Sera's coughing won't stop"). Volatile HUD only, one per worker per cycle. (feeds: Mother's Tool Origin or a recurring-face epoch)
- **Read torn fragments.** Rare ground-spawn paper scraps near workplaces; `F` to pick up, `SPACE` reads one line. Same fragment surface as institutional logs but human-scale.
- **Trace residual heat.** Brief glyph footprint on the path a worker just walked, decays in seconds. Lets the player infer "where did they go" without minimap markers.
- **Inspect dropped object provenance.** `SPACE` on a carryable shows last-handler glyph color (not identity). Foreshadows surveillance without simulating it.
- **Cross-check site against record.** `SPACE` while carrying an institutional log fragment reveals an extra inspection line on related sites. Pairs scan with carry.

### Recurring Faces And Personal Stakes

- **A named recurring worker.** One worker has a name, persists across sessions, and accumulates per-name state (debt clock, stress). The player's choices feel attached to a person, not to a role. (feeds: Mother's Tool Origin or new epoch)
- **A sibling at home.** Housing has a younger sibling glyph; supply level affects sibling state readout (`STEADY`, `HUNGRY`, `SICK`). No HP, no death — just visible moral pressure. (feeds: Shelter, Scarcity, And Care)
- **Mother's letters.** Authored letters in housing or institutional sites. Each one expands a single Debugger label or unlocks one verb mode. Lore and verb in the same slice. (feeds: Mother's Tool Origin)
- **Workshop fragment.** An abandoned engineer's workshop tile. Inspecting it reveals one truth about a gadget mode and unlocks a tool upgrade pickup. (feeds: Mother's Tool Origin)

### Risk Escalation (Still Local)

- **Worker stress accumulates.** Repeated incidents on the same worker shift their readout (`STRESSED`, `DESPERATE`). Stressed workers refuse certain interactions; desperate workers accept the bribe verb.
- **Workplace lockdown.** After N unresolved incidents at a workplace, the door won't open until time-of-day rotates. Forces alternates (vents, rooftop, waiting).
- **Cooling cycle.** Dawn resets some suspicion states; gives the player a breathable rhythm and makes time-of-day matter.
- **Audit officer cameo.** When audit-trace count crosses a threshold, a one-off NPC walks through the affected workplace, reads a clipboard, leaves. No system, just visible foreshadowing. (feeds: Soft Surveillance Seeds)

### Hidden Systems Foreshadowing

- **Power grid hint.** One path-state node flickers in a pattern; inspecting reveals `GRID PHASE: 2/3`. The third phase never appears mechanically — just an ambient promise.
- **Camera glance.** A static camera glyph above a workplace blinks red for a single frame when the player's audit-trace count crosses a threshold. Never tracks. Never alerts. (feeds: Soft Surveillance Seeds)
- **Black market vendor.** At night, one workplace exterior shows a vendor glyph; inspect for a price label only — no trade, no wallet. (feeds: Commerce, Rations, And Exchange Pressure)
- **Clinic admit log.** Recoverable log line: `INTAKE: NO QUESTIONS`. Foreshadows injury handling without injury. (feeds: Municipal Access And Public Control)
- **Letter from a distant district.** One-off pickup mentioning a place we'll never visit ("the Spire"). Anchors the larger world without expanding it. (feeds: Escape Pressure And Control Systems)
- **Curfew bell.** A scheduled sound/text event. Workers visibly hurry indoors. No mechanic — just rhythm and dread.

### Carried Object Ecology

- **Output quality variance.** A bench occasionally produces a `PART` with a hidden flag; taking the flagged variant raises suspicion faster. Inspecting the bench before pickup reveals the flag.
- **Spoilage timer on supply.** Holding a `SUPPLY` past a tick threshold downgrades it to a useless variant. Pressures commitment to a destination.
- **Ration mark.** A `SUPPLY` carries a mark indicating an intended worker; delivering to the wrong worker triggers a different reaction line and possibly a witness event.
- **Heavy item.** Certain `PART`s slow the player while carried. Foreshadows weight without inventory.

### Time, Rhythm, And Atmosphere

- **Time-of-day darkness as cover.** Night halves witness range city-wide. Workers chattier at night (pairs with eavesdrop).
- **Rain burst.** A scheduled atmospheric event temporarily reduces witness range and damps the streetlight verb effect.
- **Power flicker.** Random `DARK` pulses on lit paths; a torch use timed to a flicker doubles its effect window.
- **Listen to the city.** Hold `SPACE` away from any target → one ambient line ("the city breathes in shifts"). Pure mood, but proves the gadget is contextual.
- **Shift change windows.** Worker positions are predictable on a cycle; the player can plan around predictable absence/presence rather than reacting.

### Tool Mode And Upgrade Surface

- **One new gadget mode per pickup.** Tool upgrades are found, never bought. Each pickup adds exactly one verb or one inspection line. Keeps progression diegetic and tied to exploration.
- **Mode swap is itself a verb.** A modifier key cycles between Debugger / Torch / a third found mode (e.g., `Recorder`, `Splicer`). Surfacing the mode in the HUD makes the inherited tool feel mechanical.
- **One cost per spoof.** Spoofing burns a small consumable charge that recharges at shelter. Forces the player to come home, reinforcing the shelter loop.

## Pool Discipline

- An idea graduates to `todo/TODO.md` only when it has (a) a clear player verb, (b) a visible symptom, and (c) a session-sized scope (2–4 phases).
- Two ideas can be combined in one session if they form a single vertical slice; do not bundle three.
- If an idea would require an unbuilt system to be at all interesting, defer it until a foreshadowing session has prepped the visible hook.
- Discard freely. This pool exists to widen the option space, not to be exhaustively implemented.
