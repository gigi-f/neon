# Neon Oubliette Epoch Roadmap

This file proposes future session groups for `todo/TODO.md`. It is directional context, not a rigid queue.

Future agents should use this file to keep the end-game shape in mind while still implementing one small, visible session at a time. If an epoch or session jumps too far ahead, no longer fits the vision, or would push the game toward player-management mechanics, push back and propose a smaller or different direction before writing it into `todo/TODO.md`.

## Session Design Directives

- **No decision-only phases.** Resolve design questions in the prior phase's acceptance criteria or a TODO note.
- **Keep AI playtesting current.** Any new verb, target, NPC type, symbol, map glyph, scenario, or persistence state must be reflected in the terminal playtest harness as part of the same slice.
- **Pace check: NPC first.** If the plan accumulates 3+ sessions of player verb work without a new NPC behavior, restructure before implementing.

## North Star Frame

Neon Oubliette is about a young girl with inherited illicit tools moving through a massive sci-fi city made of complex, autonomous systems: shelter, labor, supply, commerce, public works, scarcity, coercion, surveillance. The player should not become a manager, production planner, or commander.

The city should feel like a neon-lit oubliette: visible routes, clearance levels, institutional records, public surfaces with private control behind them, complex and independent NPC interactions, transit, vehicles, robots, drones, police guards, criminal gangs, construction crews, parks, coliseums, rich and poor. NPCs live their lives independently — workers clock in, guards patrol, traders negotiate, citizens eat and sleep and argue. The player learns by watching, eavesdropping, inspecting, hiding, carrying things, and choosing when to help, steal, expose, exploit, or disappear. NPC-to-NPC interactions are the primary entertainment; player verbs are tools for observation and limited intervention.

The city is controlled by 5 independent factions, each headed by an AGI with a different personality, morality, and goals. The AGIs may be friendly or antagonistic with each other, creating a complex relationship web. The player's actions will affect their reputation within each faction over time.

Implementation order should follow this ladder:
- Autonomous NPC behavior loop, visible to the player without any input.
- NPC-to-NPC interaction visible in real time (symbols first, speech text later).
- One player observation verb that enriches what the player sees.
- One player intervention that has a local, readable NPC consequence.
- Wider simulation after local hooks prove the need.

## Epoch: NPC Simulation Foundation

Gameplay outcome: the city has multiple NPC types with visible, autonomous behavior loops. NPC-to-NPC interactions happen in real time and are the primary thing the player watches.

Big Picture: Citizens, workers, guards, and traders each have a visible daily loop. A worker clocks in and out and shows distress when their supply is missing. A guard patrols and confronts a suspicious citizen — both showing emotion symbols. A trader stands at a market and approaches nearby citizens. All of this is visible without the player pressing anything. The player chooses when to step in, steal something, or observe from a distance. Emotion symbols are the first visual language; speech bubbles with short text come later.

Example sessions:

- Session: NPC Emotion Surface (symbols above glyphs — see active TODO)
- Session: Production Loop As City System (worker output → depot, citizen $ interest)
- Session: Second NPC Type — Guard (patrol, observes workers/citizens, real-time symbols)
- Session: Citizen needs loop — citizen shows hunger ($), walks to market, satisfaction symbol
- Session: NPC-to-NPC speech — guard stops a worker, both show symbols, one short text line above each

## Epoch: Surveillance — The City Watches NPCs

Gameplay outcome: surveillance systems observe NPC citizens, not just the player. The player watches surveillance watching others and can use carried items to interfere with what surveillance records.

Big Picture: Guards, drones, and audit robots each have visible observation fields. A guard sees a worker near the depot at an odd hour and flags them. A drone logs a citizen's repeated market visit. An audit terminal produces a flag readout visible to the player. The player can observe all of this and optionally interfere — blocking a drone's sightline by standing in it, slipping a data chip into an audit terminal, or slipping a note to a watched citizen. Surveillance is about the city's internal systems watching each other; the player is a third party who can subvert or exploit those observations.

Example sessions:

- Session: One guard with a visible observation field and patrol coverage map
- Session: Guard-to-NPC interaction: guard checks a worker, worker shows distress, guard shows aggression
- Session: Drone observation field visible to player; SPACE reveals drone's current log target
- Session: Player uses G (with carried item) to interfere with one surveillance record

## Epoch: Municipal Access And Public Control

Gameplay outcome: public infrastructure has staff, autonomous routines, and access layers. The player can observe institutional life from the outside and use tools to navigate one restricted boundary.

Big Picture: A clinic has doctors, nurses, janitors, and robots all moving autonomously. Injured citizens arrive by ambulance, are triaged, treated, and discharged. The clinic has restricted records and a loading dock with supply deliveries. A business org skims profit. The player watches this machine operate, learns its rhythms, and finds one weak point they can exploit with a carried item or observed credential. No health system for the player — only observation of the city's healthcare as an institution.

Example sessions:

- Session: Clinic NPC jobs and visible autonomous routines
- Session: Injured citizen → ambulance → emergency room → discharge loop
- Session: Player observes credential exchange and uses a carried item to pass one access layer

## Epoch: Commerce, Rations, And Exchange Pressure

Gameplay outcome: markets are at the core of NPC citizen lives. Citizens shop, consume, and show distress when supply is scarce. Markets must be supplied by trucks from outside the district.

Big Picture: Citizens visit markets autonomously based on need. Market stock depletes and restocks via supply deliveries. Markets in poor districts have lower quality and more distress. Citizens in wealthy districts show commerce symbols near luxury stalls. The player observes the flow of goods through the city and can interrupt it — stealing a supply truck's cargo, hiding stock, or redirecting a delivery — to see the downstream effect on NPC citizens.

Example sessions:

- Session: Citizen commerce loop (citizen shows $, walks to market, takes item, satisfaction)
- Session: Market supply delivery — truck arrives at loading dock, stock is moved inside
- Session: Market scarcity — empty market triggers citizen distress cascade

## Epoch: Shelter

Gameplay outcome: housing has visible social pressure. Citizens live in specific buildings; some are overcrowded, some are vacant. The player can observe evictions and find alternate shelter through observation, not a purchase menu.

Big Picture: Citizens have home addresses. Overcrowded units produce visible distress. Evictions happen — a citizen is removed, their glyph appears outside with a distress symbol. The player can squat in a temporarily vacant unit, observe a landlord agent making rounds, or watch a family shelter in a transit station. No real estate menu, no prices — just the city's housing pressure made visible.

Example sessions:

- Session: Citizens assigned to residential buildings, visible home address on inspection
- Session: Housing pressure — overcrowded units, distress symbols, eviction event
- Session: Player finds a vacant unit by observation (no purchase verb; occupancy from carrying a key item or watching a handoff)

## Epoch: Complex Transit

Gameplay outcome: the city moves continuously and visibly. Trains run on schedule. Citizens commute. Vehicles carry goods. The player can board and observe any of it.

Big Picture: Transit moves in real time and is subject to delays and obstructions independent of the player. Citizens use transit to get to work; a late train cascades into worker tardiness and a delayed depot. The player can observe timetables, miss a train, board a vehicle bound for an unexplored district, or block a freight delivery to see what starves downstream.

Example sessions:

- Session: Citizen uses transit to commute to work, shows confusion if train is late
- Session: Freight vehicle in motion with autonomous loading/unloading at a depot
- Session: Road signs, pedestrian crossings, visible traffic flows tied to NPC schedules

## Idea List (upcoming):

- Full building interiors: multiple rooms, doors, multi-floor pathfinding for NPCs.
- Faction reputation: player actions affect standing with each AGI-led faction.
- Laws per faction: different districts have different rules; guards enforce their faction's law.
- Wanted level, consequences, jail: local only at first, then faction-wide.
- Biology for NPCs: hunger, injury, sleep — drives their behavior, visible to the player.
- Relationships between NPCs: friends, enemies, family; affects who helps whom in distress.
- Sparse-set ECS replacement if profiling proves the current registry is blocking active work.
