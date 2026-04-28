# Neon Oubliette Epoch Roadmap

This file proposes future session groups for `todo/TODO.md`. It is directional context, not a rigid queue.

Future agents should use this file to keep the end-game shape in mind while still implementing one small, visible session at a time. If an epoch or session jumps too far ahead, no longer fits the vision, or would push the game toward city-builder/player-management mechanics, push back and propose a smaller or different direction before writing it into `todo/TODO.md`.

## Session Design Directives

- **Vertical slices over horizontal layers.** A session that adds a site, its context, its dependency, and one interaction is better than separate sessions for sites, tags, edges, and labels. Go deep on one concept rather than wide across many.
- **Every session must include a player verb or interference opportunity.** Observation-only sessions that add inspectable labels with no new action are not allowed. If a proposed session has no verb, restructure it to include one before writing it into TODO.md.
- **Fold readouts and boundaries into mechanic phases.** Inspection text, scan metadata, and boundary tests are acceptance criteria of the phase that introduces the mechanic, not separate phases. A phase that only adds readouts or only proves boundaries is a sign the prior phase was too narrow.
- **No decision-only phases.** "Decide whether X needs Y" is a design question, not an implementation phase. Resolve it in the prior phase's acceptance criteria or a TODO note.
- **Pace check: every 3 observation sessions must be followed by 1 action session.** If the plan accumulates 3+ sessions of context/inspection/label work without a new verb, restructure before implementing.

## End-Game Frame

The end game should not assume the player becomes a manager, mayor, production planner, or commander. The player is a young girl with illicit tools inherited from her engineer mother, moving through systems she did not build: labor, infrastructure, housing, commerce, public works, scarcity, coercion, and eventually more explicit control systems.

The game should build toward simulation exploration. The player learns how the city works, finds weaknesses, uses gadgets to observe or interfere, and decides when to help, steal, hide, expose, exploit, or escape. The city should keep doing its own work organically; the player is not the owner of the means of production.

## Epoch: Local Risk, Not Surveillance Yet

Gameplay outcome: player interference has local risk before any surveillance system exists.

Big Picture: risk should begin with nearby people and systems noticing anomalies, not with a premature global surveillance model. This gives future surveillance something to generalize from later.

Logical next step: once local suspicion exists, decide whether surveillance should emerge from cameras, informants, logs, institutional audits, or some other observed system.

Sessions:

- Session: Witnessed Interruption
- Session: Local Suspicion Readout
- Session: Hiding Or De-escalation
- Session: Suspicion Persistence

Likely focus:

- Worker notices if the player takes expected output nearby.
- Building or worker readout shows suspicion or concern.
- Player can leave, return item, hide item, or use a gadget to reduce local suspicion.
- Save/load preserves the small suspicion state.

## Epoch: Hidden Systems Foreshadowing

Gameplay outcome: the player begins seeing traces of larger systems without those systems being fully implemented.

Big Picture: this keeps the end game in mind without overbuilding. Small clues can point toward surveillance, capital, coercion, public infrastructure, and institutional control before those systems exist mechanically.

Logical next step: choose one foreshadowed system to make mechanically real only after it has repeated visible hooks.

Sessions:

- Session: Institutional Log Fragment
- Session: Worker Debt Or Wage Clue
- Session: Public Infrastructure Clue
- Session: Gadget Origin Clue

Likely focus:

- Add inspectable/loggable fragments tied to existing buildings and workers.
- Keep clues local and deterministic.
- Do not add faction AI, surveillance networks, economy simulation, or narrative campaign structure.
- Use clues to guide which system deserves the next real mechanic.
