## ADDED Requirements

### Requirement: The Mega-City as Oubliette
The game world SHALL be a single procedurally-generated mega-city. The city is not a level or a map — it is the game. The name "Oubliette" (a dungeon with only a trapdoor above) SHALL inform the city's identity: a neon-lit prison that society has chosen to forget. The city runs whether the player acts or not. It decays, evolves, and breathes on its own.

#### Scenario: City as living entity
- **WHEN** the player takes no action for an extended period
- **THEN** the city SHALL continue to simulate — agents move, markets shift, buildings decay, crises develop — demonstrating that the city is not waiting for the player

#### Scenario: Oubliette metaphor in world design
- **WHEN** the player explores the city
- **THEN** the city SHALL feel inescapable — towering structures, underground warrens, no visible horizon or exit — reinforcing the "forgotten dungeon" theme

### Requirement: Procedural City Generation
The city SHALL be procedurally generated from a world seed. The default world size SHALL be 256x256 World Units (WU) (configurable). The city SHALL be subdivided into districts, zones, blocks, and lots, with building density and height varying by zone type and distance from the urban core.

#### Scenario: Seed-based reproducibility
- **WHEN** the same world seed is used
- **THEN** the city layout, zone assignments, and building placements SHALL be identical

#### Scenario: Chicago-style height gradient
- **WHEN** approaching the urban core
- **THEN** building heights SHALL increase dramatically — from 1-4 floor slum shanties at the periphery to 150+ floor glass towers at the center — creating a visible skyline gradient

### Requirement: Vertical Z-Axis
The city SHALL have vertical depth via a Z-axis. Players SHALL be able to move between ground-level streets, building interiors at various floors, rooftops, and subterranean areas (sewers, underground chapels, clandestine labs). Each Z-level SHALL have distinct atmospheric conditions.

#### Scenario: Descending underground
- **WHEN** the player enters a sewer or underground area
- **THEN** the atmosphere SHALL shift — reduced visibility, different hazards, different NPC populations (criminals, cultists, alien activity)

#### Scenario: Ascending a skyscraper
- **WHEN** the player moves upward through building floors
- **THEN** the environment SHALL transition from street-level grime through office spaces to executive suites, reflecting the city's vertical class stratification

### Requirement: Zone Types
The city SHALL contain the following 14 zone types, each with distinct visual identity, building typology, NPC density, and economic character:

| Zone | Character |
|------|-----------|
| URBAN_CORE | Dense skyscraper forest, corporate HQs, maximum height |
| CORPORATE | Glass towers, clean streets, high surveillance |
| COMMERCIAL | Shops, markets, moderate density |
| MIXED_COMMERCIAL | Combined retail/residential, street-level commerce |
| RESIDENTIAL | Apartment blocks, row houses, domestic life |
| SLUM | Shanties, overcrowding, decay, squatters |
| INDUSTRIAL | Factories, warehouses, pollution, production |
| CIVIC | Government buildings, courts, administrative |
| RELIGIOUS | Temples, shrines, broadcast towers, worship sites |
| PARK | Green space, nature effects (reduced frustration, cooling) |
| TRANSIT | Maglift stations, pneumatic tube hubs, transit platforms |
| AERODROME | High-altitude hover vessel docks and VTOL pads |
| INDUSTRIAL | Factories, warehouses, pollution, **CREMATORIUMS** |
| COLOSSEUM | Arena zones with arena floor/seating terrain |

### Requirement: Futuristic Transit Infrastructure
The city's movement SHALL be powered by three distinct tiers of futuristic transportation:
- **Maglift Monorails**: Frictionless high-speed rail that hums with blue static electricity, connecting major districts via elevated tracks.
- **VTOL Aerodynes & Hover Vessels**: Vertical-thrust vehicles that navigate the "canyons" between skyscrapers. These replace traditional airplanes for inter-city and high-altitude travel.
- **EMMVs (Electric Micro-Mobility Vehicles)**: Self-driving, single-occupancy "Auto-Pods" and high-torque courier "Strider-Bikes" that navigate the dense Chicago-style street grid.

#### Scenario: Visual transit identity
- **WHEN** the player observes a TRANSIT zone
- **THEN** they SHALL see Maglift tracks and Pneumatic Tube stations rather than traditional diesel/electric trains, reinforcing the high-tech/low-life aesthetic.

### Requirement: Terrain Vocabulary
- **WHEN** the player moves between zone types
- **THEN** the visual environment, NPC density, ambient activity, and economic indicators SHALL change noticeably

### Requirement: Terrain Vocabulary
The game SHALL support the following terrain types: VOID, STREET, SIDEWALK, GRASS, DIRT, CONCRETE_FLOOR, WOOD_FLOOR, WALL, WINDOW, OFFICE_CARPET, FLOWER_BED, WATER_FEATURE, ARENA_FLOOR, ARENA_SEATING, RAIL, SEWER_FLOOR, SEWER_WATER.

#### Scenario: Terrain affects traversal and atmosphere
- **WHEN** an agent or player moves across different terrain types
- **THEN** the terrain SHALL affect movement properties and convey the local environment (sewers feel different from office carpet)

### Requirement: Interior Spaces
Buildings SHALL contain interior spaces with the following room types: LOBBY, OFFICE, SERVER_ROOM, EXECUTIVE_SUITE, BEDROOM, KITCHEN, BATHROOM, LIVING_ROOM, FACTORY_FLOOR, STORAGE, SUPERVISOR_OFFICE, HALLWAY, NAVE, ALTAR, SEATING_AREA, FENCE, CLANDESTINE_LAB, SECURITY_HUB, HOLDING_CELL, **CREMATORIUM_CHAMBER**.

Interior spaces SHALL include doors, stairs, elevators, and navigation grids. Building interiors SHALL maintain their own coordinate space accessible via portals from the exterior.
#### Scenario: Entering a building
- **WHEN** the player enters a building from the street
- **THEN** the view SHALL transition to the building's interior, revealing rooms, NPCs, items, and interior-specific activities (working, sleeping, cooking, crafting).

### Requirement: High-Fidelity Interior Simulation
Interior spaces SHALL NOT be empty boxes. They MUST be logical, densely packed, and purpose-driven to support the city's causal simulation.

#### 1. Logical Layouts (Adjacency Rules)
Buildings SHALL generate interior rooms based on architectural adjacency rules:
- **RESIDENTIAL**: KITCHEN/LIVING_ROOM adjacency; BEDROOM/BATHROOM privacy.
- **CORPORATE**: LOBBY → HALLWAY → OFFICE → EXECUTIVE_SUITE hierarchy.
- **INDUSTRIAL**: STORAGE → FACTORY_FLOOR → SUPERVISOR_OFFICE flow.
- **SCENARIO**: A player exploring a RESIDENTIAL unit SHALL find the BATHROOM adjacent to the BEDROOM, never opening directly into a LOBBY.

#### 2. Densely Packed Prop-Sets
Rooms SHALL be populated with functional props that provide visual density and serve as item containers (L3 Economic):
- **OFFICE**: 4-8 **DESKS** per room, **TERMINALS**, **FILING_CABINETS**, **COFFEE_STATIONS**.
- **STORE**: **CLOTHING_RACKS**, **FOOD_SHELVES**, **REFRIGERATION_UNITS**, **CHECKOUT_COUNTERS**.
- **FACTORY**: **CONVEYOR_BELTS**, **CRATES**, **HEAVY_MACHINERY**, **TOOL_RACKS**.
- **RESIDENTIAL**: **BEDS**, **DINING_TABLES**, **STORAGE_UNITS**, **DOMESTIC_APPLIANCES**.
- **SCENARIO**: When the player enters a COMMERCIAL "Grocer" node, they SHALL see rows of **FOOD_SHELVES** packed with individual FOOD/WATER items, not just a flat floor.

#### 3. Purpose-Driven Room Tasks
Every room type SHALL be mapped to specific NPC **Agent Tasks**:
- **KITCHEN/LIVING_ROOM**: Enables **SEEK_SUSTENANCE** and **LEISURE** tasks.
- **OFFICE/FACTORY**: Enables **GO_TO_WORK** and **PRODUCE_GOODS** tasks.
- **STORAGE/WAREHOUSE**: Enables **OPEN_CONTAINER** and **TAKE_ITEM** (Scavenging/Theft).
- **SCENARIO**: An NPC in the **WORKING** state SHALL be found at a **DESK** or **MACHINERY** prop, performing their assigned task, never idling in a BATHROOM.

### Requirement: Environmental Hazards
...
The city SHALL feature persistent environmental hazards that affect all simulation layers:
- **Acid Rain**: Periodic weather event that damages exposed entities and structures
- **Industrial Smog**: Localized pollution reducing air quality and health
- **Heat Islands**: Temperature hotspots from dense infrastructure
- **Toxic Gas**: Hazardous areas affecting consciousness and causing pain
- **Radiation**: Localized zones with biological effects
- **Flooding**: Water accumulation in low-lying areas

#### Scenario: Acid rain event
- **WHEN** acid rain begins
- **THEN** exposed NPCs SHALL seek shelter, structural integrity of exposed buildings SHALL degrade, and dialogue SHALL reference the weather ("Acid rain. It eats through the concrete. And us.")
### Requirement: Player Housing & The Sanctuary
The player SHALL be able to establish a "Home-Base" (Sanctuary) by claiming, renting, or squatting in an interior space. A Sanctuary provides a persistent location for metabolic restoration, item storage, and tool maintenance.

#### Housing Acquisition via Broker
To establish a Sanctuary, the player SHALL first interact with a local "Housing Broker" NPC at a designated office location within the zone. The broker's identity, office type, and dialogue SHALL reflect the zone's character:

| Housing Tier | Broker NPC Type | Office Location | Transaction Style |
|--------------|-----------------|-----------------|-------------------|
| **Tier 0: The Pod** | **NONE (Assigned)** | SLUM / RESIDENTIAL | **AUTOMATED**: Assigned by the Consensus at spawn. |
| **Tier 1: The Squat** | Slum Lord / Gang Boss | Repurposed STORAGE or CLANDESTINE_LAB | Transactional / Threatening (STREET speech) |
| **Tier 2: The Rental** | Real Estate Agent | COMMERCIAL Office | Bureaucratic / Formal (STANDARD speech) |
| **Tier 3: The Penthouse** | Faction Liaison / Executive | CORPORATE EXECUTIVE_SUITE | High-Status / Reputation-Gated (CORPORATE speech) |

#### Scenario: Starting in the Assigned Pod (Tier 0)
- **WHEN** the player begins a new simulation (Spawn)
- **THEN** they SHALL materialize in their **Assigned Pod** — a 2x2 WU "living box" within a massive **Pod Apartment** filled with dozens of identical units.
- **The Engineer's Diary**: The Pod SHALL contain a unique, non-tradable item: **The Engineer's Diary**.
- **Lore Delivery**: Interacting with the Diary SHALL provide optional, fragmentary lore entries explaining the "Great Erasure," the player's mother's disappearance, and the illegal nature of the 5 investigative tools.
- **The Neon Work Board**: Upon exiting their pod into the apartment hall, the player SHALL encounter a flickering **NEON_WORK_BOARD**.
- **Initial Directive**: The board SHALL display the player's current **Consensus Directive**: "CITIZEN [ID] IDENTIFIED. PROCEED TO BOX SORTING AT 0700 HOURS. FAILURE TO COMPLY IS A LEVEL 1 SEDITION EVENT."

#### Scenario: The Box Sorting Center (L0 Industrial/Transit)
- **Location**: A specialized **INDUSTRIAL** building located within 10 World Units (WU) of the starting Pod Apartment.
- **Components**: Features a massive **CONVEYOR_BELT** system and a player-accessible **SORTING_TERMINAL**.
- **The Mini-Game**: Interacting with the terminal SHALL trigger the **BOX_SORTING** task — a repetitive, low-skill mini-game (moving items between conveyor streams) that pays a baseline rate of 1 credit per game-hour.

#### Scenario: Ignoring the Directive (L4 Political Risk)
- **WHEN** the player ignores their assigned work directive
- **THEN** they SHALL NOT be physically forced to comply, but their **Wanted Level** with the **Consensus (Aura-9)** SHALL increase slightly (Level 1 Sedition). This creates a "soft" pressure to escape the initial district or earn credits elsewhere.

#### Scenario: Acquiring a Slum Squat
...
- **THEN** they SHALL engage in dialogue with the **Slum Lord** (STREET profile), who offers a room for a low credit fee or a "Favor" (L3/L4 task). Upon completion, the room's L0-L4 ownership state SHALL update to "Player Sanctuary."

#### Scenario: Renting a Corporate Penthouse
- **WHEN** the player reaches **ALLY** reputation with the dominant faction and enters a CORPORATE zone's **Real Estate Office**
- **THEN** they SHALL meet with an **Executive Broker** (CORPORATE profile) to sign a high-credit lease. The Penthouse SHALL then provide premium utilities and VTOL/GhostDrone launch pads.

#### Sanctuary Components (Internal)
...
The player SHALL be able to install and interact with the following components:
- **Neural-Link Bed**: Accelerates **Neural Charge** recovery and provides a safe interface for **GhostDrone** piloting.
- **Storage Lockers**: Persistent inventory for items (maintaining their **Provenance** history).
- **Crafting Stations**: Forge, Laboratory, and Workbench for Tier 2/3 tool maintenance and item production.
- **Metabolic Auto-Chef**: Consumes Raw Materials to produce high-tier FOOD and WATER items.
- **Trophy Pedestal**: Displays high-value LUXURY items, providing a permanent reduction in **Frustration**.

#### Sanctuary Protection (5-Layer Defense)
To prevent raids from rival factions or thieves, the player SHALL be able to install specialized defenses:
- **L0 (Physical)**: **Reinforced Blast Doors** and **Auto-Turrets** (Sentinel-derived).
- **L1 (Biological)**: **Biometric DNA-Locks** that only allow the player (or trusted NPCs) to enter.
- **L2 (Cognitive)**: **Signal Jammers** that reduce the Sanctuary's "Social/Cognitive Visibility," preventing NPCs from remembering the location.
- **L3 (Economic)**: **Bribery Accounts** that automatically pay off local gangs or corrupt guards to ignore the Sanctuary.
- **L4 (Political)**: **Diplomatic Immunity Shields** (Requires high Faction Reputation) that mark the room as a "Non-Intervention Zone."

#### Scenario: Defending against a raid
- **WHEN** the player's **Signal Heat** or **Wanted Level** exceeds a critical threshold
- **THEN** a Faction Raid or Scavenger Break-in SHALL occur. The installed defenses (L0-L4) SHALL attempt to repel the intruders automatically based on their respective layer strengths.

### Requirement: Aesthetic Identity
The visual language SHALL be "medical diagnostic interface meets cyberpunk noir":
- **Color Palette**: Deep blacks (#0A0A0F, #12121A) as base, with neon accents — cyan (#00FFFF) for information, green (#00FF88) for biology/health, red (#FF0044) for danger/alerts, gold (#FFD700) for wealth/religion, magenta (#FF00FF) for cognition/mystery.
- **Diagnostic Metaphor**: The player interface SHALL feel like a diagnostic system jacked into a failing city — clinical observation of urban decay.
- **Atmosphere**: Dystopian noir — constant electronic surveillance, flickering lights, towering corporate spires above crumbling slums.

#### Scenario: Color as information layer
- **WHEN** the player inspects different simulation aspects
- **THEN** each layer SHALL use its assigned diagnostic color — biology in green, cognition in magenta, economics in gold, politics in red — creating a consistent visual language

### Requirement: Time of Day Cycle
The city SHALL operate on a cyclic day/night system with four phases:
- **DAWN** (transition period)
- **DAY** (peak activity, work shifts)
- **DUSK** (transition, commuting)
- **NIGHT** (reduced activity, different NPC populations, increased crime)

#### Scenario: Night changes city behavior
- **WHEN** the time shifts to NIGHT
- **THEN** most citizens SHALL be sleeping or in leisure activities, crime rates SHALL increase, different NPC archetypes (criminals, cultists, nightlife) SHALL become more active, and the visual atmosphere SHALL darken
