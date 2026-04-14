## ADDED Requirements

### Requirement: Seven Crisis Types
The city SHALL be subject to macro-level crises that emerge from simulation layer conditions and create sustained narrative tension. Each crisis type SHALL have severity levels, countdown timers, and cascading effects across all five simulation layers.

| Crisis | Origin Layer | Trigger |
|--------|-------------|---------|
| ECONOMIC_COLLAPSE | L3 Economic | Unemployment spike, inflation, market crash |
| BIOLOGICAL_OUTBREAK | L1 Biology | Pathogen spread, contamination, xeno biological agents |
| INFRASTRUCTURE_FAILURE | L0 Physics | Power grid failure, water system collapse, structural cascades |
| POLITICAL_UNREST | L4 Political | High crime, protests, faction conflict, public opinion collapse |
| XENO_INCURSION | Cross-layer | Aborrax invasion, Archon manifestation, xeno influence expansion |
| ENVIRONMENTAL_HAZARD | L0 Physics | Toxic spills, radiation events, severe acid rain, flooding |
| FACTION_WAR | L4 Political | Open combat between factions for territory control |

#### Scenario: Crisis emergence from simulation
- **WHEN** simulation conditions cross a crisis threshold (e.g., unemployment exceeds a critical percentage)
- **THEN** a named crisis SHALL be declared with an initial severity level, and the crisis dashboard SHALL display it with a severity bar and countdown timer

#### Scenario: Crisis escalation
- **WHEN** a declared crisis is not addressed (conditions continue to worsen)
- **THEN** the crisis severity SHALL increase, its effects SHALL intensify, and it MAY trigger secondary crises (e.g., ECONOMIC_COLLAPSE → POLITICAL_UNREST → FACTION_WAR)

### Requirement: Crisis Cross-Layer Cascades
Each crisis type SHALL produce cascading effects across all five simulation layers. Crises are not isolated events — they are sustained disruptions that ripple through every aspect of city life.

**ECONOMIC_COLLAPSE cascade**:
- L0: Infrastructure maintenance budgets cut → accelerated decay
- L1: Citizens can't afford food/medical → health deterioration
- L2: Frustration spikes, desperation → crime rate surges
- L3: Markets crash, unemployment spiral, supply chains break
- L4: Political blame, faction rivalry, policy emergency

**BIOLOGICAL_OUTBREAK cascade**:
- L0: Contaminated areas become hazardous zones
- L1: Infection spread, hospital overflow, mortality
- L2: Fear, quarantine anxiety, social isolation
- L3: Medical supply shortage, economic disruption from workforce loss
- L4: Quarantine enforcement, faction blame, public health policy

**INFRASTRUCTURE_FAILURE cascade**:
- L0: Structural collapse, power outage, water shutoff
- L1: Temperature stress, dehydration, injuries
- L2: Panic, evacuation, looting
- L3: Business shutdown, production halt, supply chain disruption
- L4: Emergency response, infrastructure budget debates, contractor corruption

#### Scenario: Full cascade from infrastructure failure
- **WHEN** a power grid failure crisis is declared in an industrial zone
- **THEN** the player SHALL observe: factories go dark (L0), workers are exposed to temperature stress (L1), panic and frustration spike (L2), production halts and unemployment rises (L3), and the dominant faction debates emergency resource allocation (L4) — all unfolding over successive ticks

### Requirement: Crisis Dashboard
The game SHALL provide a Crisis Dashboard (accessible in both Standard and God Mode) displaying:
- Named active crises with severity bars (0-100%)
- Countdown timers (estimated time to resolution or escalation)
- Per-category metrics: infection rate, pollution level, unemployment percentage, power status
- Historical trend lines showing whether conditions are improving or worsening

#### Scenario: Dashboard as decision tool
- **WHEN** the player opens the Crisis Dashboard during an active BIOLOGICAL_OUTBREAK
- **THEN** they SHALL see the outbreak name, current severity, infection rate trend, affected zones, and estimated time to peak — enabling informed decisions about where to go, who to help, and what resources to stockpile

### Requirement: Faction Crisis Response
Each faction SHALL respond differently to crises based on their philosophy:

- **Consensus (Aura-9)**: Imposes order — curfews, increased surveillance, mandatory quarantines. Effective but authoritarian. Becomes repressive under crisis pressure.
- **Entropic Drift (Malware-Alpha)**: Exploits chaos — recruits the desperate, loots infrastructure, accelerates instability to weaken rivals. Grows stronger during crises.
- **Silicon Maw (The Architect)**: Offers augmentation — "upgrade or perish." Pushes biological solutions, recruits from the sick and injured. Crisis as recruitment tool.
- **Void-Walkers (The Signal)**: Remains enigmatic — continues Geometric Directives regardless of crisis. May issue cryptic "protection" to followers. Alien indifference.
- **Syndicate (The Arbiter)**: Profiteers — price gouging, black market medicine, protection rackets. Provides services at exorbitant cost. Crisis as business opportunity.

#### Scenario: Competing crisis responses
- **WHEN** a BIOLOGICAL_OUTBREAK is active
- **THEN** the player SHALL observe different factions offering different solutions — Consensus quarantine vs. Maw augmentation vs. Syndicate black market medicine — each with different costs, effectiveness, and political consequences

### Requirement: Emergent Narrative Philosophy
The game SHALL reject scripted narratives in favor of **systemic storytelling**. Narrative SHALL emerge from the interaction of simulation layers, faction directives, NPC needs, and player choices. The city tells its own story through its behavior.

**Three Rules of Systemic Storytelling**:
1. **Every system must have a visible symptom** — if the player can't see it happening, it doesn't exist as narrative. A collapsing economy must be visible in closed shops, hungry citizens, and faction tensions.
2. **Every system must have a player verb** — if the player can't act on it, it's backdrop, not gameplay. An outbreak must have actions the player can take (distribute medicine, quarantine zones, flee).
3. **Every system must have procedural depth** — if it doesn't interact with other systems to create emergence, it's a static feature, not a simulation. Crime must feed into economics, economics into politics, politics into enforcement.

#### Scenario: Emergent story from system interaction
- **WHEN** multiple simulation systems interact (e.g., drought → food shortage → price spike → theft → guard crackdown → political unrest)
- **THEN** a coherent narrative SHALL emerge that no designer scripted — the player experiences a story *generated* by systemic interaction

#### Scenario: Player agency within emergent narrative
- **WHEN** an emergent crisis is developing
- **THEN** the player SHALL have multiple meaningful ways to interact — they can profit from it, help resolve it, exploit it for faction gain, document it, or simply survive it — with each choice creating different cascading consequences

### Requirement: The Great Erasure
The city's backstory SHALL include "The Great Erasure" — a historical event where something was deliberately forgotten. This is the meta-narrative mystery of Neon Oubliette. It SHALL be:
- Referenced in NPC dialogue, myths, and rumors
- Never fully explained — always ambiguous, always provocative
- The reason the city is an "oubliette" — a place built to forget
- Hinted at through environmental storytelling, faction contradictions, and deep exploration

#### Scenario: Discovering Erasure fragments
- **WHEN** the player explores deep underground areas, faction archives, or speaks with ANCIENT-stage NPCs
- **THEN** they SHALL encounter fragmentary references to the Great Erasure — contradictory accounts, redacted records, cryptic warnings — building a picture that is compelling but never complete

### Requirement: Game Modes Support Observation
The game SHALL support two modes that together enable both participation and observation of emergent narrative:

**STANDARD MODE**: The player is an agent within the city — first-person survival, direct interaction, personal stakes. The city is experienced from street level.

**GOD MODE**: The player is a detached diagnostic observer — overhead view, inspection tools, data visualization. The city is observed as a system. The inspection toolkit SHALL use color-coded diagnostic lenses:
- Surface Scan (cyan) — entity identity, position, visual state
- Biological Audit (green) — organ health, metabolism, vitals
- Cognitive Profile (magenta) — emotions, goals, memories, relationships
- Financial Forensics (yellow) — credits, debts, transaction history
- Structural Analysis (red) — building integrity, zone data, infrastructure health

#### Scenario: God Mode reveals hidden cascades
- **WHEN** the player switches to God Mode during an active crisis
- **THEN** the diagnostic tools SHALL reveal the cascade mechanics invisible from street level — which buildings are structurally failing, which agents are infected, which markets are crashing, which factions are mobilizing

### Requirement: No Traditional Progression
The game SHALL NOT feature traditional RPG progression (XP, levels, skill points). Player progression SHALL be measured through:
- **Reputation**: Faction standing unlocks access, services, and story
- **Wealth**: Credits enable economic agency
- **Fame**: How widely the player's actions are known
- **Knowledge**: Information gathered about the city, factions, and the Great Erasure
- **Influence**: The player's ability to affect faction decisions, market prices, and political outcomes

#### Scenario: Reputation as progression
- **WHEN** the player increases their standing with a faction from NEUTRAL to FRIENDLY
- **THEN** new faction facilities, NPCs, dialogue options, and story threads SHALL become accessible — this IS the progression

### Requirement: Soft Combat
Violence in the game SHALL be "soft" — abstracted rather than tactical. Combat is not the core loop; it's a consequence of simulation dynamics. Attacks generate events (damage, crime reports, witness memories) rather than engaging a separate combat system. The emphasis SHALL be on the *consequences* of violence (reputation, wanted level, faction response, trauma) rather than the *mechanics* of fighting.

#### Scenario: Violence as consequence generator
- **WHEN** the player attacks an NPC
- **THEN** the system SHALL generate: damage to the victim (L1), fear in witnesses (L2), a crime report if observed (L4), potential guard response, wanted level increase, and reputation change — the violence matters because of what it *causes*, not how it plays out tactically

### Requirement: Design Inspirations as Creative Constraints
The following games define the creative ambition and should inform design decisions:

- **Dwarf Fortress**: Tissue-layer wound tracking, medical simulation, procedural storytelling through simulation. The standard for simulation depth.
- **Cataclysm: Dark Days Ahead**: Real-time survival needs, infection systems, JSON-driven content. The standard for survival authenticity.
- **RimWorld**: Complex NPC AI with moods, need systems, psychological break mechanics. The standard for NPC personality simulation.
- **Space Station 13**: Reagent systems, atmosphere simulation, role-based emergent gameplay. The standard for systemic interaction.
- **FTL**: Resource allocation strategy, system management under pressure. The standard for meaningful scarcity.

#### Scenario: Inspiration audit
- **WHEN** a new system is being designed for the successor
- **THEN** it SHALL be evaluated against these inspirations — does it achieve Dwarf Fortress depth? RimWorld personality? SS13 emergence? If not, it needs more design work.
