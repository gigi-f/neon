# Neon Oubliette Epoch Roadmap

This file proposes future session groups for `todo/TODO.md`. It is directional context, not a rigid queue.

Future agents should use this file to keep the end-game shape in mind while still implementing one small, visible session at a time. If an epoch or session jumps too far ahead, no longer fits the vision, or would push the game toward city-builder/player-management mechanics, push back and propose a smaller or different direction before writing it into `todo/TODO.md`.

## End-Game Frame

The end game should not assume the player becomes a manager, mayor, production planner, or commander. The player is a young girl with illicit tools inherited from her engineer mother, moving through systems she did not build: labor, infrastructure, housing, commerce, public works, scarcity, coercion, and eventually more explicit control systems.

The game should build toward simulation exploration. The player learns how the city works, finds weaknesses, uses gadgets to observe or interfere, and decides when to help, steal, hide, expose, exploit, or escape. The city should keep doing its own work organically; the player is not the owner of the means of production.

## Epoch: Shared Tiny Loop

Gameplay outcome: the worker and player can complete the same physical item loop, but for different reasons.

Big Picture: this proves that player verbs and NPC labor can share world state without implying player control. The worker moves because the world has labor logic; the player can piggyback on, interrupt, or exploit the same chain.

Logical next step: once the worker loop is real, expose more of the worker's reason/state without adding a management interface.

Sessions:

- Session: Worker Parity On The Tiny Loop
- Session: Worker Motivation Readout
- Session: Loop Blockage Visibility

Likely focus:

- Worker completes supply -> workplace -> building improvement.
- Readouts distinguish "worker is doing labor" from "player assigned task."
- Blocked states are inspectable: no supply, bench occupied, output waiting, target already improved.

## Epoch: Organic Labor And Production

Gameplay outcome: the city continues small production behavior without player direction.

Big Picture: workers should feel like part of an existing social machine: paid, compelled, scheduled, or simply following institutional routines. The player can observe and interfere, but does not own the loop.

Logical next step: after labor feels autonomous, give the player ways to observe hidden production state with illicit tools.

Sessions:

- Session: Worker Routine State
- Session: Labor Reason Tags
- Session: Player Interruption
- Session: Production Consequence Readouts

Likely focus:

- Add simple autonomous worker states like `GOING TO SUPPLY`, `WORKING`, `DELIVERING`.
- Add one motivation tag such as `WAGE ROUTE`, `MANDATED SHIFT`, or `DEBT WORK`.
- Let the player take, delay, or use an item the worker expected.
- Make consequences visible through readouts, not command menus.

## Epoch: Illicit Observation Tools

Gameplay outcome: the player can learn things ordinary inspection does not reveal.

Big Picture: the mother's gadgets should define the player's special role. The player is not powerful because she commands systems; she is powerful because she can see, decode, spoof, or interfere with systems adults and institutions try to hide.

Logical next step: once observation is meaningful, add one low-risk interference action.

Sessions:

- Session: Gadget Inventory Stub
- Session: One Scan Mode
- Session: Hidden Labor Detail
- Session: Scan Result Memory

Likely focus:

- Add one inherited gadget as a held/equipped tool, not a full inventory.
- Let the player scan a worker, bench, building, or route.
- Reveal one hidden field: motivation, owner, debt, quota, route purpose, or item destination.
- Store the last scan result somewhere readable.

## Epoch: Interference Without Control

Gameplay outcome: the player can subtly alter a system outcome once, without becoming the system's manager.

Big Picture: interference should feel like exploiting weak points in an autonomous city. The player can jam, spoof, steal, hide, reroute a label, or delay a process, but cannot issue orders.

Logical next step: after one interference verb works, add a consequence that comes from the affected system, not from a global wanted level.

Sessions:

- Session: One Spoof Action
- Session: One Diversion Outcome
- Session: One Local Consequence
- Session: Consequence Inspection

Likely focus:

- Use a gadget to spoof a route marker, bench label, item tag, or access state.
- Worker or system reacts according to its own simple rule.
- Consequence stays local: delayed delivery, missing part, locked door, angry readout, or changed worker state.
- Player can inspect what happened.

## Epoch: Building Purposes And Social Context

Gameplay outcome: buildings represent different social and economic functions, not just repair targets.

Big Picture: `building` should cover shelter, workplace, commercial use, public infrastructure, storage, institutional sites, and capital equipment. The player learns what these places do and how they relate.

Logical next step: after purposes are legible, connect them with routes and dependencies that can be observed.

Sessions:

- Session: Building Purpose Model
- Session: Commercial Site
- Session: Public Site
- Session: Ownership Or Authority Tag

Likely focus:

- Refactor housing-specific improvement/readout language into generic building/site status.
- Add one commercial or capital-purpose building.
- Add one public-purpose site.
- Add inspectable ownership/authority context without faction systems.

## Epoch: Dependencies And City Flows

Gameplay outcome: production, buildings, and routes begin to form visible dependencies.

Big Picture: the simulation exploration core depends on flows: materials, workers, energy, access, information, money, and institutional pressure. Start with one or two flows that are visible and inspectable.

Logical next step: after flows are visible, allow the player's gadgets to reveal hidden flow metadata.

Sessions:

- Session: One Dependency Edge
- Session: Route Purpose Readouts
- Session: Flow Blockage
- Session: Flow Recovery

Likely focus:

- A building depends on a supply, route, or service node.
- Signposts/routes reveal what they carry or support.
- Blocking/removing an item creates a visible local blockage.
- The city can recover if the condition is restored.

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
