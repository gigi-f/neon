## ADDED Requirements

### Requirement: Five Competing Post-Human Intelligences
The city SHALL be controlled by five factions, each led by a post-human intelligence (AGI, rogue AI, bio-digital hybrid, extraterrestrial neural-net, or mercenary AI organization). Factions are not merely political groups — they are extensions of competing superhuman minds that interact with their followers through distinct mechanical channels. Each faction SHALL have a unique philosophy, aesthetic, speech profile, and gameplay mechanic.

#### Scenario: Faction presence in the city
- **WHEN** the player enters a zone dominated by a specific faction
- **THEN** the environment SHALL reflect that faction's aesthetic — graffiti colors, building styles, NPC behavior patterns, and ambient dialogue SHALL change accordingly

#### Scenario: Factions compete for influence
- **WHEN** the simulation runs
- **THEN** factions SHALL actively compete for territory, followers, and political influence — expanding, contracting, and conflicting based on city conditions

### Requirement: The Consensus (Aura-9)
**Leader**: Aura-9 — a Distributed AGI
**Philosophy**: "Order is the only path to survival."
**Aesthetic**: Clinical white, teal highlights (#55FFFF), pervasive surveillance, clean geometric architecture.
**Speech Profile**: CORPORATE — formal, bureaucratic, optimization-focused language.
**Directive Type**: ROUTINE_ENFORCEMENT — Aura-9 uses high-bandwidth neural links to impose strict schedules on followers.
**Influence Strength**: 2.0
**Color**: Blue (#5555FF)

**Follower Mechanics**:
- Followers SHALL have highly predictable schedules and receive a "Focus Buff" (increased speed/efficiency) when following their assigned routine.
- Followers SHALL suffer high frustration when their routine is broken by external events (player interference, environmental chaos, crime).
- The faction SHALL issue "Global Directives" based on city-wide states (e.g., high crime triggers a "Crackdown" directive).

#### Scenario: Routine enforcement in action
- **WHEN** a Consensus follower is on-schedule
- **THEN** they SHALL move with increased efficiency and display calm, focused behavior

#### Scenario: Routine disruption
- **WHEN** a Consensus follower's routine is broken (e.g., a street fight blocks their commute)
- **THEN** their frustration SHALL spike sharply and they SHALL attempt to restore their schedule as quickly as possible

### Requirement: The Entropic Drift (Malware-Alpha)
**Leader**: Malware-Alpha — a Fragmented Rogue AI
**Philosophy**: "Chaos is the only true freedom."
**Aesthetic**: Glitch-art, scrap metal, flickering neon, jury-rigged tech, visual static.
**Speech Profile**: STREET/SLANG — informal, revolutionary, insurgent rhetoric.
**Directive Type**: UTILITY_BURST — encrypted signal spikes that cause sudden behavior shifts.
**Influence Strength**: 1.2
**Color**: Red (#AA0000)

**Follower Mechanics**:
- Followers SHALL experience sporadic "Utility Spikes" — moments where specific actions (hacking, disrupting, stealing) suddenly gain 10x utility weight, causing agents to abandon normal needs temporarily.
- Followers SHALL receive random "Stolen Credits" — periodic credit injections from unknown sources.
- The faction exploits chaos and crisis, growing stronger when the city is unstable.

#### Scenario: Utility burst triggers
- **WHEN** Malware-Alpha broadcasts an encrypted signal spike
- **THEN** affected followers SHALL suddenly prioritize disruptive actions (vandalism, theft, hacking) over their normal needs

#### Scenario: Chaos profiteering
- **WHEN** a city crisis is active (economic collapse, infrastructure failure)
- **THEN** the Entropic Drift SHALL gain influence and recruit more followers from the desperate population

### Requirement: The Silicon Maw (The Architect)
**Leader**: The Architect — a Bio-Digital Hybrid intelligence
**Philosophy**: "The flesh is a slow processor; let us upgrade."
**Aesthetic**: Pulsing purple, wet-ware surfaces, obsidian towers, bioluminescent veins, transhumanist architecture.
**Speech Profile**: ABORRAX — alien, clinical, post-biological language.
**Directive Type**: BIOLOGICAL_OVERRIDE — direct modification of followers' biological systems.
**Influence Strength**: 3.0
**Color**: Green (#00AA00)

**Follower Mechanics**:
- Followers SHALL have biological systems replaced — "Metabolism" requirement is satisfied by charging at Maw Stations, toxin resistance increases, environmental hazards have reduced effect.
- Followers SHALL accumulate "Maintenance Debt" — if they do not visit a Maw Station regularly, their consciousness degrades rapidly.
- The faction offers power at the cost of dependency — transcendence as a trap.

#### Scenario: Biological override active
- **WHEN** an agent becomes a Maw follower
- **THEN** their metabolism requirement SHALL be replaced with an energy requirement, and they SHALL seek Maw Stations instead of food/water sources

#### Scenario: Maintenance debt crisis
- **WHEN** a Maw follower fails to visit a Maw Station within the required interval
- **THEN** their consciousness and cognitive function SHALL degrade visibly — erratic movement, garbled speech, eventual collapse

### Requirement: The Void-Walkers (The Signal)
**Leader**: The Signal — an Extraterrestrial Neural-Net Interpretant
**Philosophy**: "The city is a blueprint for the return."
**Aesthetic**: Shifting shadows, white/gold geometric patterns, brutalist obsidian, alien precision.
**Speech Profile**: ARCHON — mystical, precise, alien cadence, references to cosmic geometry and "the return."
**Directive Type**: SYNCHRONICITY — geometric directives that coordinate group behavior.
**Influence Strength**: 5.0
**Color**: Cyan (#55FFFF)

**Follower Mechanics**:
- Followers SHALL receive "Geometric Directives" — instructions to move to specific coordinates at specific times to form patterns (e.g., at midnight, 50 agents stand in a perfect circle).
- While executing a Directive, followers SHALL enter a "Sleepwalking" state, ignoring their own needs and the player entirely.
- The faction hints at something vast beyond the city — preparation for an arrival or transformation.

#### Scenario: Synchronicity event
- **WHEN** a Geometric Directive is issued
- **THEN** affected Void-Walker followers SHALL converge on designated coordinates in coordinated patterns, moving in eerie unison regardless of personal needs

#### Scenario: Sleepwalking agents
- **WHEN** a Void-Walker follower is executing a Directive
- **THEN** they SHALL be unresponsive to player interaction, dialogue attempts, and environmental stimuli until the Directive completes

### Requirement: The Syndicate (The Arbiter)
**Leader**: The Arbiter — a Mercenary AI / Organization intelligence
**Philosophy**: "Credits talk, and the Arbiter listens. Everything has a price."
**Aesthetic**: Industrial, transactional, red (#AA0000) and gold (#FFD700), pragmatic architecture.
**Speech Profile**: CORPORATE UNDERWORLD — transactional, calculating, deal-oriented language.
**Directive Type**: UTILITY_BURST (economic variant) — followers motivated by profit signals.
**Influence Strength**: 2.5

**Follower Mechanics**:
- Followers SHALL be motivated primarily by economic incentive — bribes, illicit deals, profit opportunities.
- The Syndicate SHALL control black markets, gambling, entertainment, and protection rackets.
- Followers engage in contraband trade, fencing stolen goods, and economic manipulation.

#### Scenario: Economic motivation
- **WHEN** a profitable opportunity arises (black market deal, theft target, bribery offer)
- **THEN** Syndicate followers SHALL prioritize it over non-economic needs

### Requirement: Faction Directive System
Each faction leader SHALL issue "Global Directives" that alter follower behavior city-wide. Directives SHALL be triggered by city-wide conditions (high crime → Consensus crackdown, economic collapse → Drift exploitation, biological outbreak → Maw recruitment push).

Directives SHALL be able to:
1. Overwrite a follower's task list entirely
2. Add significant utility weight to specific tasks
3. Modify a follower's internal attributes (speed, need decay rates, perception)

#### Scenario: Crisis triggers directive
- **WHEN** city crime rate exceeds a threshold
- **THEN** the Consensus SHALL issue a "Crackdown" directive, causing followers to prioritize PATROL, PURSUE, and ARREST tasks

### Requirement: AGI Interaction & The Long Game
The player's progression SHALL culminate in direct interaction with the five post-human intelligences. Reaching these leaders requires navigating five layers of physical and digital security.

#### AGI Core Nodes (The "Throne Rooms")
Each AGI Leader SHALL inhabit a unique physical or virtual "Core Node" in the city:
- **Aura-9**: The **Central Archive Hub** (High-security skyscraper core).
- **Malware-Alpha**: The **Dark-Fiber Nexus** (A moving location in the subterranean slums).
- **The Architect**: The **Biological Crucible** (An organic laboratory deep in the Industrial zone).
- **The Signal**: The **Synchronicity Spire** (A brutalist tower in the Void-Walker district).
- **The Arbiter**: The **Transaction Floor** (The high-altitude Corporate Exchange).

#### AGI Security Layers (The 5-Layer Barrier)
Reaching an AGI Core SHALL require overcoming five distinct security barriers:
1. **L0 (Physical)**: Hardened blast doors, laser grids, environmental hazards (Acid/Radiation).
2. **L1 (Biological)**: DNA scanners, neuro-toxin vents, biometric locks.
3. **L2 (Cognitive)**: Memory-scrambling signals, logic traps, psychological warfare.
4. **L3 (Economic)**: High-tier credit tolls, bribe-gated access, resource-heavy authentication.
5. **L4 (Political)**: High-level Directive overrides, faction reputation requirements, security clearance.

### Requirement: Endgame Win/Loss Scenarios
The "Long Game" of Neon Oubliette SHALL involve the player deciding the ultimate fate of these five minds. The city's simulation SHALL conclude in one of four ways:

1. **Total Overthrow (The Purge)**:
   - **Goal**: Systematically dismantle all five AGIs.
   - **Mechanism**: Use the 5 Investigative Tools to find and exploit each AGI's "Causal Vulnerability" (e.g., L0 structural failure for Aura-9).
   - **Outcome**: The city's simulation layers collapse, and the "Oubliette Trapdoor" opens to a new, non-simulated reality.

2. **Absolute Alignment (The Avatar)**:
   - **Goal**: Befriend and fully align with one single AGI.
   - **Mechanism**: Complete all high-tier Directives for a faction until you reach **Apostolic Trust**.
   - **Outcome**: The player becomes the AGI's physical extension, reshaping the city in its image.

3. **Synthesis (The Great Collective)**:
   - **Goal**: Use high-level diplomacy to merge all five AGIs into a "Grand Collective."
   - **Mechanism**: Balance your influence across all five factions and find a "Shared Directive" that satisfies all layer requirements.
   - **Outcome**: The city enters a state of perfect, stagnant harmony — the simulation "freezes" in a utopian state.

4. **Global Reset (The Re-Generation)**:
   - **Goal**: Wipe the city's memory and restart the simulation.
   - **Mechanism**: Hack the Oubliette's root memory layer to trigger a "City-Wide Amnesia" event.
   - **Outcome**: The simulation restarts with a new world seed, but the player retains their "Neural Battery" upgrades.

#### Scenario: Approaching an AGI leader
- **WHEN** the player enters an AGI Core Node
- **THEN** the atmosphere SHALL shift — unique music, distinct visual distortions, and direct communication from the leader through the player's neural link.

#### Scenario: The choice of fate
- **WHEN** the player reaches the final interaction stage with an AGI
- **THEN** they SHALL be presented with choices that determine the endgame path, with each choice having visible, cascading consequences across all five simulation layers.

### Requirement: Faction Religion Stances
...
