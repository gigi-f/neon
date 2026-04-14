## ADDED Requirements

### Requirement: Species Taxonomy
The game SHALL support the following species, each with distinct lifespans, biological properties, and social roles:

| Species | Lifespan | Role |
|---------|----------|------|
| Human | 120 years | Baseline citizen — the majority population |
| Synthetic | 200 years | AI/robotic entities, material: steel, mechanical needs |
| Aborrax | Ageless | Alien scavengers/invaders, biological threats, unintelligible speech |
| Archon | Ageless | Ancient alien servants, reality-warping, extremely high status (95+) |
| Rat | Short | Urban fauna, disease vector |
| Canine | Medium | Urban fauna, companion potential |
| Feline | Medium | Urban fauna, companion potential |

#### Scenario: Species affects social interaction
- **WHEN** the player encounters an Archon
- **THEN** the entity SHALL have extremely high social status, and other NPCs SHALL display deference behavior

#### Scenario: Alien speech barrier
- **WHEN** the player attempts dialogue with an Aborrax
- **THEN** dialogue SHALL be rendered as "[UNINTELLIGIBLE CHITTERING / FEEDBACK]" — alien, hostile, incomprehensible

### Requirement: Four Xeno Types
The game SHALL feature four categories of xenobiological entities that disrupt the simulation across all layers:

**Archon**: Post-human or ancient alien servants. Reality-warping presence. Highest status entities in the city. Connected to the Void-Walkers faction and the Cult of the Archons religion. Their presence hints at something vast and alien behind the city's existence.

**Aborrax**: Scavenger-species alien invaders. Biological threats that attack, infect, and destabilize neighborhoods. Communicate in unintelligible noise. Hostile, unpredictable, feared by citizens.

**Behedicci**: Massive, slow-moving or entirely static entities. Their presence warps local conditions — temperature shifts, gravity anomalies, resource scarcity. They are environmental hazards as much as creatures.

**Post-Human**: Highly evolved human variants that have transcended baseline humanity. Extremely high status, often connected to faction leadership. Rare, powerful, unsettling.

#### Scenario: Xeno cross-layer influence
- **WHEN** a xeno entity is present in a zone
- **THEN** it SHALL exert influence across simulation layers — temperature offset (L0 Physics), frustration delta (L2 Cognitive), scarcity modifier (L3 Economic), and potentially local gravity distortion (L0 Physics) — within its influence radius

#### Scenario: Aborrax incursion
- **WHEN** an Aborrax incursion event occurs
- **THEN** Aborrax entities SHALL appear in vulnerable zones, attacking citizens, spreading biological contamination, and triggering guard response — escalating into a city-wide crisis if unchecked

### Requirement: NPC Core Needs System
Every NPC agent SHALL track three core needs on a 0-100 scale:
- **Metabolism** (100 = full, 0 = exhausted): Consolidates Hunger and Thirst. Decays over time, satisfied by consuming food or water items.
- **Frustration** (0 = content, 100 = enraged): Increases from unmet needs, crime, conflict, environmental stress; decreases from socialization, entertainment, worship.
- **Socialization** (100 = fulfilled, 0 = lonely): Satisfied by conversations, relationships, faction activities, worship.

Needs SHALL decay over time and drive NPC behavior — exhausted agents seek sustenance, lonely agents seek conversation, frustrated agents become volatile.

#### Scenario: Low Metabolism changes behavior
- **WHEN** an NPC's metabolism drops below a critical threshold
- **THEN** they SHALL abandon their current task to SEEK_SUSTENANCE, and their dialogue SHALL reference their desperation ("My energy is low. I need to find something to eat or drink soon.")

#### Scenario: High frustration triggers volatility
- **WHEN** an NPC's frustration reaches a high threshold
- **THEN** their crime risk (boldness) SHALL increase, they SHALL become more likely to SHOVE, MUG, or STEAL, and their dialogue SHALL turn hostile

### Requirement: Seven Life Stages (Humans and Synthetics Only)
`Human` and `Synthetic` agents SHALL progress through life stages that affect their visual representation, capabilities, and social role. Fauna (Rats, Canines, Felines) do NOT use life stages.

| Stage | Character |
|-------|-----------|
| INFANT | Small, fragile, dependent — the most vulnerable |
| CHILD | Maturing, learning, limited agency |
| YOUNG_ADULT | Peak vibrancy, entering the workforce |
| ADULT | Standard city dweller, full agency |
| ELDER | Fading, experienced, may have accumulated status/wealth |
| ANCIENT | Mythic, rare — those who have survived extraordinarily long |
| AGELESS | Aborraxi, Archons — beings outside the normal lifecycle |

Agents SHALL have biological wear (0.0-1.0) that increases with age and hardship, affecting health and appearance.

#### Scenario: Aging affects capability
- **WHEN** an agent reaches ELDER stage
- **THEN** their visual appearance SHALL desaturate/dim, movement speed SHALL decrease, but their social status and accumulated wealth may be higher than younger agents

### Requirement: Social Hierarchy
Every NPC SHALL have a social hierarchy position defined by:
- **Status**: Numeric value reflecting social standing
- **Class Title**: Descriptive label (worker, professional, executive, etc.)
- **Deference/Yielding**: Behavioral modifiers — lower-status agents yield to higher-status ones in movement and social interaction

#### Scenario: Status affects interaction
- **WHEN** a low-status agent encounters a high-status agent in a narrow space
- **THEN** the low-status agent SHALL yield movement priority and display deferential body language

### Requirement: Relationship System
NPCs SHALL maintain relationships with other agents on a five-tier scale:
- **STRANGER** (no prior interaction)
- **ACQUAINTANCE** (limited interaction history)
- **FRIEND** (positive affinity, mutual benefit)
- **INTIMATE** (deep bond, shared home potential)
- **ENEMY** (negative affinity, active hostility)

Each relationship SHALL track: affinity (-100 to +100), last interaction tick, shared home status, and a rolling history of the last 3 interactions.

#### Scenario: Relationship evolves through interaction
- **WHEN** two NPCs have repeated positive interactions (conversation, trade, worship together)
- **THEN** their relationship tier SHALL gradually progress from STRANGER toward FRIEND or INTIMATE

#### Scenario: Betrayal creates enemies
- **WHEN** an NPC is mugged, stolen from, or otherwise harmed by a known agent
- **THEN** the relationship SHALL shift sharply toward ENEMY, affecting future encounters and dialogue

### Requirement: Agent Task Vocabulary
Agents SHALL be able to perform the following task types, organized by category:

**Basic Survival**: IDLE, WANDER, SEEK_SUSTENANCE, SEEK_SHELTER, GO_TO_WORK

**Movement**: MOVE_TO_TARGET, MOVE_ALONG_PATH, AWAITING_PATH, WAIT_FOR_TRANSIT, RIDE_TRANSIT, HURRY_TO_TRANSIT

**Item Interaction**: PICK_UP_ITEM, CONSUME_ITEM, OPEN_CONTAINER, TAKE_ITEM_FROM_CONTAINER, CRAFT_ITEM, USE_ITEM

**Resource & Production**: SEEK_HARVESTABLE, EXTRACT_RESOURCE, PRODUCE_GOODS, REPAIR

**Social & Religious**: WORSHIP, VISIT_SHRINE, FOLLOW_LEADER, PROCESSION

**Crime**: STEAL_FROM_AGENT, MULE_GOODS, SELL_CONTRABAND, FENCE_ITEM, SEEK_FENCE, MUG_AGENT, AWAIT_MUGGING_COMPLIANCE, REACT_TO_MUGGING, FLEE, SHOVE

**Law Enforcement**: PURSUE, INVESTIGATE, ARREST, PATROL

**Housing**: SQUAT

Tasks SHALL be selected by a utility-based AI system (BDI — Belief-Desire-Intention) that weighs current needs, faction directives, personality, and environmental conditions.

#### Scenario: Need-driven task selection
- **WHEN** an NPC's metabolism drops to a low value
- **THEN** the utility calculation SHALL increasingly weight SEEK_SUSTENANCE until it becomes the highest-priority task

#### Scenario: Faction directive overrides needs
- **WHEN** a faction issues a directive (e.g., Void-Walker Synchronicity event)
- **THEN** the directive SHALL override normal utility calculations, causing followers to execute the directive task regardless of personal needs

### Requirement: Schedule State Machine
NPCs SHALL follow a daily routine governed by four schedule states:
- **SLEEPING** (night)
- **WORKING** (day — at workplace, producing, earning credits)
- **LEISURE** (dawn/dusk — socializing, shopping, worship, entertainment)
- **COMMUTING** (transitions between home, work, and leisure locations)

#### Scenario: Commuting behavior
- **WHEN** the time transitions from SLEEPING to WORKING
- **THEN** NPCs SHALL enter COMMUTING state, traveling from home to workplace via streets or transit in real-time.

### Requirement: Item System
The game SHALL feature items categorized into 9 market categories:

| Category | Purpose |
|----------|---------|
| FOOD | Restores metabolism |
| WATER | Restores metabolism |
| MEDICAL | Healing, treatment (L1 Injury Statuses) |
| TECHNOLOGY | Advanced tools, electronics |
| LUXURY | Status symbols, wealth display |
| CLOTHING | Appearance, identity, protection |
| WEAPONRY | Combat capability |
| TOOLS | Crafting, harvesting, repair |
| CONTRABAND | Illegal goods — higher value, criminal risk |

Items SHALL have: trade value in credits, durability (current/max with decay over time), and category-specific properties.

#### Scenario: Contraband risk/reward
- **WHEN** the player carries or trades CONTRABAND items
- **THEN** the items SHALL have higher value than legal equivalents, but detection by guards SHALL increase wanted level and trigger law enforcement response

### Requirement: Raw Materials
The crafting and production economy SHALL use five raw material types:
- **METAL**: Salvaged scrap, industrial ore — used in weapons, tools, construction
- **CHEMICAL**: Lab reagents, fuel, acid — used in drugs, medical supplies
- **BIOMASS**: Food base, biological tissue — used in food production, biological research
- **ELECTRONIC**: Components, chips — used in technology, drones, surveillance
- **ENERGY**: Fuel cells, isotopes — used in power generation, Maw Station charging

#### Scenario: Material scarcity drives conflict
- **WHEN** a raw material becomes scarce in a zone
- **THEN** item prices SHALL increase, production SHALL slow, and competition for resources SHALL intensify — potentially triggering economic crisis

### Requirement: Building Properties
Buildings SHALL have: height (floors), zone type, occupant count, structural integrity (decays over time via urban decay), and squatter count. Buildings with low structural integrity SHALL show visible decay. Abandoned buildings SHALL attract squatters.

Each building SHALL maintain interior state: rooms with types, utilities (power, water), and a local economic profile (GDP, unemployment, wage index, crime rate, material scarcity).

#### Scenario: Urban decay cascade
- **WHEN** a building's structural integrity drops below a threshold
- **THEN** it SHALL become visually decrepit, occupants SHALL flee if possible or become squatters, local property values SHALL drop, and the zone's crime rate SHALL increase

### Requirement: Drone Archetypes
The city SHALL feature three drone archetypes operating as autonomous entities:

**SurveillanceDrone**: Optical sensor (range 10 WU, detection 70%), aerial patrol behavior. Faction-aligned — primarily Consensus. Represents the city's panopticon surveillance network.

**DeliveryDrone**: Cargo capacity (50 units), aerial heavy pathfinding, delivery route behavior. Commercial infrastructure — moves goods through supply chains.

**MaintenanceDrone**: Repair capability (power 15), material storage (10 units), maintenance protocol behavior. Infrastructure maintenance — patches the city's decaying systems.

**ReclamationDrone**: Automated body removal system. Equipped with biological containment clamps. Scans for "L1 Biology: Health = 0" entities, attaches to them, and transports them to the nearest **CREMATORIUM**. Faction-aligned with **Consensus** or **Industrial** infrastructure.

**PropagandaDrone (Herald)**: Flying holoscreen and loudspeaker array. Broadcasts faction-aligned "Gold Path" messages, religious sermons, and city-wide news. They increase the "Social/Cognitive Visibility" of their faction's **Directives** in a zone.

**CleaningDrone (Sweeper)**: Low-level "vacuum" drones. They maintain high cleanliness ratings in **CORPORATE** and **URBAN_CORE** zones but are conspicuously absent from **SLUMS**, reinforcing the city's vertical class stratification.

**GhostDrone (Player-Owned)**: Illicit, high-mobility reconnaissance drone. These are "Ghost" variants of industrial models, stripped of faction identification and safety limiters. They allow the player to scout between vertical layers (Z-axis) remotely.

| Tier | Name | Max Range (WU) | Max Height (Floors) | Signature |
|------|------|----------------|---------------------|-----------|
| 1 | **Screamer** | 20 | 2 | High |
| 2 | **Specter** | 50 | 10 | Moderate |
| 3 | **Void-Wraith** | 150 | 50+ | Low |

### Requirement: Robot Archetypes
Unlike the aerial, high-mobility drones, robots are ground-based, heavy-duty entities designed for specialized industrial, medical, and security tasks.

**Enforcer Walker (Sentry)**: Heavy, multi-legged L4 Political walker. Primarily used by the **Consensus** during "Crackdowns." Equipped with non-lethal (sound-cannons) and lethal (laser-arrays) options. High structural integrity and immune to L2 Cognitive manipulation.

**Medical-Unit (Medi-Bot)**: Specialized L1 Biology robot. They can stabilize NPCs or the player for a high credit fee (L3). They prioritize high-status agents and ignore those with high **Criminal Records** or **Maw Maintenance Debt**.

**Hauler-Unit (Mule)**: Heavy-duty L3 Economic transport robots. They move massive quantities of raw materials (METAL, CHEMICAL, BIOMASS) between Industrial and Commercial zones. They follow set routes and yield only to high-status faction members.

All drones and robots SHALL have: speed, power systems (capacity, consumption rate, charge rate), and AI behavior trees governing their routines.

#### Scenario: GhostDrone vertical scouting
- **WHEN** the player deploys a **GhostDrone** from a rooftop or street level
- **THEN** they SHALL transition to the drone's camera view, allowing them to fly upward through the "skyscraper canyons" and scan interior windows or rooftops within the drone's Tiered range.

#### Scenario: Reclamation drone activity
- **WHEN** an NPC or player dies in the street
- **THEN** a ReclamationDrone SHALL be dispatched from the nearest industrial hub, retrieve the body, and transport it to a crematorium for processing.

#### Scenario: Surveillance drone detection
- **WHEN** a player commits a crime within a SurveillanceDrone's sensor range
- **THEN** the drone SHALL detect the crime with its detection probability and trigger a CrimeReport, increasing wanted level

### Requirement: Crime Risk Profile
NPCs SHALL have a crime risk profile defined by:
- **Boldness** (0-100): Innate propensity toward criminal behavior
- **Active Criminal Flag**: Whether currently engaged in criminal activity
- **Last Crime Tick**: When they last committed a crime

High boldness combined with high frustration and low metabolism SHALL dramatically increase the likelihood of criminal behavior.

#### Scenario: Desperation breeds crime
- **WHEN** an NPC has high frustration, low metabolism, and moderate-to-high boldness
- **THEN** their utility calculation SHALL increasingly weight criminal tasks (STEAL, MUG, SELL_CONTRABAND) as viable survival strategies
