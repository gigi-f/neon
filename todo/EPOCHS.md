# Neon Oubliette Epoch Roadmap

This file proposes future session groups for `todo/TODO.md`. It is directional context, not a rigid queue.

Future agents should use this file to keep the end-game shape in mind while still implementing one small, visible session at a time. If an epoch or session jumps too far ahead, no longer fits the vision, or would push the game toward city-builder/player-management mechanics, push back and propose a smaller or different direction before writing it into `todo/TODO.md`.

## Session Design Directives

- **No decision-only phases.** "Decide whether X needs Y" is a design question, not an implementation phase. Resolve it in the prior phase's acceptance criteria or a TODO note.
- **Keep AI playtesting current.** Any new verb, target, prompt, map glyph, scenario, persistence state, or readout that an AI tester needs must be reflected in the terminal playtest harness as part of the same gameplay slice.
- **Pace check: focus on action.** If the plan accumulates 3+ sessions of simulation building without a new gameplay mechanic, restructure before implementing.

## North Star Frame

Neon Oubliette is about a young girl with inherited illicit tools moving through a massive sci-fi city made of complex systems: shelter, labor, supply, commerce, public works, scarcity, coercion, surveillance. The player should not become a manager, production planner, or commander.

The city should feel like a neon-lit oubliette: visible routes, clearance level access, institutional records, public surfaces with private control behind them, complex and independent NPC interactions, transit, vehicles, robots, drones, police guards, criminal gangs, construction crews, parks, colloseums, rich and poor. The player learns by exploring, eavesdropping, inspecting, hiding, spoofing, and choosing when to help, steal, expose, exploit, or disappear.

The city is controlled by 5 independent factions, with an AGI at the head of each. Each AGI has a different personality, morality, and goals. The AGIs may be friendly or antagonistic with each other, creating a complex relationship web. The player's actions and wealth will have an impact on their reputation within each faction.

Implementation order should follow this ladder:

- Visible local symptom.
- One player verb or interference opportunity.
- A readable consequence on the affected thing.
- A tight persistence/playtest boundary if the state matters.
- Wider simulation after local hooks prove the need.

## Epoch: Municipal Access And Public Control

Gameplay outcome: public infrastructure stops being only scenery and becomes a layered local control surface the player can inspect and spoof.

Big Picture Example: There is a clinic made up of doctors, nurses, robots, etc. The clinic has a large database of patient history. Supplies are regularly delivered to the hospital via a loading dock. Employees move the supplies to proper storage locations within the clinic. The clinic is organized like a modern day hospital: emergency room that holds injured citizens, vehicle bay for ambulances, surgery wing, labor delivery and nursery for births, etc. A janitor cleans up the rooms once vacated. A business org runs the hospital and attempts to extract profits. Injured citizens within the macro zone use the nearest hospital. The player must gain security clearance to access the back rooms. Entering without clearance raises suspicion and can lead to security measures against the player. Doctors clock in and clock out, returning home via public transit.

Example sessions:

- Session: Clinic Architectural Layout
- Session: Clinic NPC jobs and goals
- Session: Local NPC injury => ambulance => emergency room => treatment => discharge loop

## Epoch: Commerce, Rations, And Exchange Pressure

Gameplay outcome: the market is core to the lives of NPC citizens and a broader economy.

Big Picture: Citizens need to buy food, water, shelter, clothing, etc. in order to live their lives. A variety of markets cater to these needs. The markets must be supplied with wares to sell, often via truck or other large transport. Markets in poor neighborhoods will have lower quality wares. There are central "market districts" that sell luxury goods for a high price. We do not need to get as granular as raw material extraction. At least for now, it is ok for transport vehicles to enter from the player-inaccessible city limits and bring their goods to factories or refineries on the city outskirts.

Example sessions:

- Session: A variety of markets
- Session: Buying / selling / trading system
- Session: NPC commerce loop: buying clothes, buying food, taking home, using and consuming.

## Epoch: Tool UI system

Gameplay outcome: using the inherited debugger tool prints results to a Unix-terminal inspired windowed readout.

Big Picture: Instead of the most recent tool-use printing in a debug line, a new window can open up with a terminal style print-out of the debugger results. The window should mimic a linux GUI window, with a button to close, and a mouse-draggable top bar so the player can move the window where they see fit on the screen.

Example sessions:

- Session: Opening a GUI window over the gameplay window
- Session: Terminal UI design
- Session: GUI window mouse manipulation


## Epoch: Shelter

Gameplay outcome: Macro zones have real estate dealers that can offer housing based on market pricing. A slum unit is extremely cheap, whereas a penthouse downtown is extremely expensive.

Big Picture: The player has a home base that can be found on some sort of city map. The player can store goods in their home base. The player can buy a new shelter- part of the payment goes to movers who transfer player belongings (price based on quantity and value)

Example sessions:

- Session: Types of shelter
- Session: Real estate agent
- Session: Shelter transfer

## Epoch: Surveillance

Gameplay outcome: Depending on the AGI faction who controls a macro zone, there will be different types of surveillance. The player's standing within the faction determines the level and extent of surveillance and rule-breaking consequences.

Big Picture: Laws per AGI, guards per AGI/faction, consequences for rule breaking, player reputation

Example sessions:

- Session: Surveillance tools: drones and robots
- Session: Laws per AGI
- Session: Jail, repercussions, violence
- Session: Faction Reputation

## Epoch: Complex transit

Gameplay outcome: Regardless of player action, the city is in constant motion. NPC citizens going to work, guard robots monitoring, drones taking dead bodies to the crematorium. Trains run on a real-time schedule between macro zones. Trucks pick up and deliver goods to markets, factories, refineries. 

Big Picture: Transit moves in real time and is subject to traffic jams, delays, and obstructions. Independent vehicles are controlled by an NPC- for example, a hover craft being driven to work. Vehicles are not "set dressing"- they are simply tools used by citizens to accomplish their goals. Vehicles can break, be fixed, be purchased. Poor to mid-level citizens cannot purchase vehicles and must rely on transit.

Example sessions:

- Session: Train in motion
- Session: NPC owns a car, stores it at home, takes it to work
- Session: Road signs, traffic flows


## Idea List (upcoming):

- Full building doors, multi-room interiors, furniture components, and pathfinding.
- Multi-slot inventory, equipment, scanner tools, item provenance, market barter, and survival counters.
- Biology, injury, pathogens, cognitive state, relationships, schedules, conversations, rumors, and eavesdropping.
- Roads, traffic, vehicles, power grid simulation, conduits, and city-scale infrastructure.
- Factions, wanted level, directive markets, law enforcement, crises, xenos, AGI cores, death cascade, sanctuary systems, and broad narrative simulation.
- Sparse-set ECS replacement unless profiling proves the current registry is blocking active work.

