## ADDED Requirements

### Requirement: Five-Layer Causal Simulation
The game SHALL simulate the city through five causally-linked layers. Each layer represents a different scale of consequence, from the physical to the political. Events in lower layers SHALL propagate upward, creating emergent narrative cascades. This is the core creative innovation of Neon Oubliette — the engine that transforms a static city into a living story.

#### Scenario: Upward causation
- **WHEN** an event occurs at a lower layer (e.g., a building collapses — L0 Physics)
- **THEN** it SHALL cascade upward — injuries (L1 Biology), fear and trauma (L2 Cognitive), insurance claims and business losses (L3 Economic), political response and policy change (L4 Political)

#### Scenario: The player observes cascade in real-time
- **WHEN** a cascade is in progress
- **THEN** the player SHALL be able to observe its effects at each layer using the diagnostic inspection tools — seeing physics damage in red, biological effects in green, cognitive effects in magenta, economic effects in gold

### Requirement: Layer 0 — Physics and Topology (The Foundation)
**Creative Identity**: The material world — concrete, steel, heat, pressure, decay. This is the city's skeleton.

**What It Simulates**: Temperature per location, atmospheric pressure, material phase states (solid, liquid, gas), structural integrity of buildings and infrastructure. The city as a physical object that obeys thermodynamics.

**Key Insight**: The city is *decaying*. Structural integrity drops over time. Acid rain accelerates it. Maintenance slows it. When physics fails, everything built on top fails too.

**Narrative Role**: Physics creates the *setting* of consequences. A heatwave isn't just weather — it's a physics event that stresses biology, frays cognition, disrupts economics, and tests politics.

#### Scenario: Structural failure cascade
- **WHEN** a building's structural integrity reaches zero
- **THEN** the building SHALL collapse or become condemned — displacing occupants (L2), destroying stored goods (L3), triggering emergency response (L4), and potentially injuring nearby agents (L1)

#### Scenario: Environmental physics shape daily life
- **WHEN** temperature extremes occur (heat island, cold snap)
- **THEN** exposed agents SHALL suffer biological stress (L1), seek shelter (L2), and energy costs SHALL spike (L3)

### Requirement: Layer 1 — Biology (The Organic)
**Creative Identity**: Flesh, blood, health, disease. This is the city's vulnerable meat.

**What It Simulates**: Individual biological health via a **Unified Health/Stress Meter**. Instead of organ tracking, the system tracks specific **Injury Statuses** (e.g., "Gunshot Wound", "Toxin Exposure", "Neural Glitch"). Metabolic processes, neural states, pathogen vectors, consciousness levels, pain.

**Key Insight**: A dead body is not stateless — it becomes an ecosystem for decomposition. Biology doesn't stop at death; it transforms.

**Narrative Role**: Biology makes consequences *personal*. A mugging isn't just a crime statistic — someone is bleeding, someone is suffering from toxin exposure, someone needs medical care they can't afford.

**Biological Indicators**: Health (0-100), Stress (0-100), Consciousness (0-100), Pain (0-100), and active Injury/Augmentation status effects.

#### Scenario: Wound cascade
- **WHEN** an agent is injured in a violent encounter
- **THEN** they SHALL lose Health, gain an injury status (e.g., "Laceration"), consciousness may decrease, and the agent SHALL seek MEDICAL items or hospital services — generating medical costs (L3) and potentially a crime report (L4)

#### Scenario: Pathogen spread
- **WHEN** a biological pathogen is introduced (via xeno incursion, contaminated water, etc.)
- **THEN** it SHALL spread through proximity and shared spaces, infected agents SHALL show symptoms (reduced Health/capability, pain), and if unchecked, it SHALL escalate to a BIOLOGICAL_OUTBREAK crisis

### Requirement: Layer 2 — Cognitive and Social (The Mind)
**Creative Identity**: Thoughts, memories, emotions, relationships, fear, desire. This is the city's psychology.

**What It Simulates**: Individual agents with BDI (Belief-Desire-Intention) architecture. Social graphs, memory formation, emotional states (pleasure, arousal, dominance), motivation engines, faction allegiance, religious belief.

**Key Insight**: Agents *remember*. A witness to violence forms a memory that affects their future behavior, relationships, and faction loyalty. The city accumulates psychological scars.

**Narrative Role**: Cognition is where simulation becomes *story*. Physics creates settings, biology creates stakes, but cognition creates *characters* — beings with desires, fears, grudges, and loyalties that the player can understand and influence.

**Emotional Dimensions**: Pleasure (satisfaction), Arousal (activation/energy), Dominance (sense of control). These three axes define an agent's emotional state and influence decision-making.

#### Scenario: Witness trauma
- **WHEN** an NPC witnesses a violent crime
- **THEN** they SHALL form a fear memory associated with the location and perpetrator, their arousal SHALL spike, their dominance SHALL drop, and they SHALL avoid the area in future — potentially changing their commute route and daily routine

#### Scenario: Social network propagation
- **WHEN** an NPC learns important information (rumor, price tip, faction intelligence)
- **THEN** they SHALL share it through their social network during conversations, spreading information organically through the city

### Requirement: Layer 3 — Economic (The Flow)
**Creative Identity**: Credits, goods, debts, supply, demand, greed. This is the city's bloodstream.

**What It Simulates**: Individual transactions up to macro-economic indicators. Inventory with provenance tracking for high-value items, market nodes with supply/demand curves, currency flows, resource networks, supply chains, production systems.

**Key Insight**: Every significant item has a *history*. Stolen goods carry provenance. The economy is not abstract numbers — it's a web of traceable transactions.

**Narrative Role**: Economics makes consequences *collective*. A factory closure isn't just one person's problem — it's unemployment, mortgage defaults, store closings, crime spikes, and political unrest. Economics is how individual suffering becomes systemic crisis.

#### Scenario: Item provenance creates story
- **WHEN** the player finds a luxury item in a slum pawnshop
- **THEN** the item's provenance SHALL reveal its journey — perhaps stolen from a corporate executive, passed through three owners — telling a story of the city's economic flows

#### Scenario: Economic ripple from single event
- **WHEN** a major employer (factory, corporation) shuts down
- **THEN** local unemployment SHALL spike, displaced workers SHALL seek new employment or turn to crime, consumer spending SHALL drop, connected businesses SHALL suffer, and the zone's economic health SHALL visibly decline

### Requirement: Layer 4 — Political and Power (The Structure)
**Creative Identity**: Law, power, factions, policy, control, rebellion. This is the city's nervous system — the command and control layer.

**What It Simulates**: Organizations and their power structures, policies and faction directives, territory influence maps, infrastructure maintenance decisions, crisis response, and the perpetual struggle between order and chaos.

**Key Insight**: Politics is the layer that *decides* how all other layers are managed. Who gets medical care (L1), who gets food (L3), which buildings get maintained (L0), whose frustration matters (L2) — all of these are political decisions.

**Narrative Role**: Politics makes consequences *systemic*. A mugging is an L2 event. A mugging *epidemic* is an L4 event — it triggers policy responses (Directives), faction competition, patrol allocation, and public opinion shifts that reshape the entire city.

#### Scenario: Political response to crisis
- **WHEN** a crisis (biological outbreak, economic collapse) exceeds a severity threshold
- **THEN** the dominant faction SHALL issue response Directives — allocating resources, deploying guards, declaring emergency zones — and rival factions SHALL propose alternative responses, competing for public support

#### Scenario: Directive changes affect daily life
- **WHEN** a faction issues a new Directive (increased surveillance, curfew, tax change)
- **THEN** the Directive SHALL take effect across all affected zones — guard behavior SHALL change, NPC routines SHALL adapt, economic conditions SHALL shift, and public opinion SHALL react

### Requirement: Five-Layer Investigative Tools (Tiered Evolution)
The player SHALL start the game with all five specialized tools at **Tier 1 (Legacy Firmware)**. These tools provide baseline functionality sufficient for early-game investigation and credit-earning, but require significant investment to unlock their full diagnostic and disruptive potential.

| Tool | Tier 1: Baseline (Start) | Tier 2: Enhanced | Tier 3: Master (Direct Action) |
|------|-------------------------|------------------|--------------------------------|
| **L0 Physics Probe** | Local wireframe; structural fracture detection. | Through-wall thermal/heat signatures; extended range. | **Action**: Resonance Break (Destroy structures). |
| **L1 Biology Spectrometer** | Simple Health % and Heart Rate. | **Biometric T-Pose Modal**; Body-part segmentation. | **Action**: Adrenaline Spike (Stabilize/Panic). |
| **L2 Cognitive Resonator** | Dominant Emotion detection. | Social Graph links; Lie Detection (HUD vibration). | **Action**: Neural Over-Resonate (Force leak). |
| **L3 Economic Lens** | Item Value and Market Category. | **Provenance Bloom** (Full transaction history). | **Action**: Market Injection (Manipulate prices). |
| **L4 Political Scouter** | Current Zone Faction Ownership. | **Directive List** (Active laws); NPC Faction Rank. | **Action**: Directive Spoof (Temporary bypass). |

#### Tool Upgrade Mechanism
- **Acquisition**: Upgrades SHALL be purchased as "Firmware Modules" from specialized vendors (e.g., Silicon Maw for Biology, Syndicate for Economic).
- **Cost**: Tier 2 upgrades SHALL have moderate credit costs; Tier 3 upgrades SHALL require high credit costs AND specific Faction Reputation thresholds.

### Requirement: Non-Human Diagnostic Profiles (L1)
The **Biometric Pulse Spectrometer (Tier 2+)** SHALL adjust its visual output when scanning non-human species to reflect their unique biology/composition:

- **Synthetics**: The T-Pose SHALL replace "Health" with **"Chassis Integrity"** and "Stress" with **"Processor Load."** It SHALL display **"Coolant Levels"** and **"Battery State"** instead of heart rate.
- **Archons**: The T-Pose SHALL be replaced by a shimmering **"Voxel Cloud"** (Magenta/Cyan). It SHALL display **"Reality Flux"** (%) instead of vitals and list status effects like `[COSMIC_SYNCHRONICITY]`.
- **Aborrax**: The scan SHALL highlight **"Parasitic Clusters"** and **"Radiation Leakage"** in high-contrast Green. It SHALL warn the player of local L0/L1 environmental hazards emanating from the entity.

### Requirement: Tool Resource & Balance (The "Cognitive Cost")
To prevent tool spamming and ensure tactical gameplay, all tools SHALL share a common resource and risk profile:

1. **Neural Battery (Charge)**: Each active use of a tool consumes "Neural Charge." This charge SHALL replenish slowly over time, or instantly via **NEURAL_STIM** items.
2. **Signal Signature (Heat)**: Active tools (especially L0 and L4) emit a detectable electronic signature. High signature levels SHALL draw the attention of nearby guards, drones, and faction security systems.
3. **Cognitive Load (Toggle Cooldown)**: Switching between lenses rapidly SHALL cause "Neural Glitching" — temporary visual artifacts and a delay before the next lens can be activated.
4. **Acquisition Time (Lock-On)**: Higher-layer tools (L2, L3, L4) SHALL require a "Lock-On" period (1-3 seconds) where the player must maintain line-of-sight, leaving them vulnerable.

#### Scenario: L0 Acoustic Stress Probe (Physics)
- **WHEN** the player equips the Probe and triggers a **PULSE**
- **THEN** it SHALL consume 15% Neural Charge and emit a "High" Signal Signature, potentially alerting nearby NPCs to the player's presence.
- **Visuals**: Transitions the world to a wireframe Cyan view — highlighting "Fracture Points" in buildings, hidden shafts, and thermal hotspots.

#### Scenario: L1 Biometric Pulse Spectrometer (Biology)
- **WHEN** the player (possessing Tier 2+) targets an NPC with the Spectrometer and triggers a **SCAN**
- **THEN** it SHALL consume 5% Neural Charge and open a **BIOMETRIC_MODAL**.
- **BIOMETRIC_MODAL Visuals**: The modal SHALL display the target NPC in a **Green T-Pose**, segmented into 6 zones: **Head, Torso, L-Arm, R-Arm, L-Leg, R-Leg**.
- **Data per Segment**: Each zone SHALL show its "Structural Integrity" (0-100%) and active "Injury Statuses" (e.g., "Shattered Bone" on L-Leg, "Internal Bleeding" in Torso).
- **Global Vitals**: The modal SHALL also show the NPC's Stress, Consciousness, and Heart Rate.

#### Scenario: L2 Neural-Linguistic Resonator (Cognitive)
- **WHEN** the player (possessing Tier 2+) maintains "Lock-On" for 2 seconds with the Resonator
- **THEN** it SHALL consume 10% Neural Charge and reveal the target's **Mind Map** in Magenta.
- **Data**: Shows Dominant Emotion (Pleasure/Arousal/Dominance) and Social Links. During dialogue, the resonator SHALL "vibrate" visually if the NPC's stress level suggests deception.

#### Scenario: L3 Crypto-Forensic Lens (Economic)
- **WHEN** the player (possessing Tier 2+) triggers a **BLOOM** on an item or shop node
- **THEN** it SHALL consume 10% Neural Charge and show a Gold "Provenance Bloom" — a history of previous owners and transaction dates. 
- **Constraint**: This tool SHALL require the player to be within 5 World Units (WU) of the target.

#### Scenario: L4 Authority Scouter (Political)
- **WHEN** the player (possessing Tier 2+) activates the Scouter **MAP**
- **THEN** it SHALL consume a continuous 2% Neural Charge per second while active and emit a "Critical" Signal Signature.
- **Visuals**: Overlays Red "Ownership Lines" and lists active **Directives** for the current zone. Using this in high-security zones is a "Hostile Act" if detected.

### Requirement: Tool-Driven "Direct Action" Verbs (Tier 3 Only)
Each investigative tool SHALL possess a "Direct Action" capability—a high-stakes, high-cost intervention that physically or digitally alters the simulation.

| Tool | Action Verb | Neural Cost | Signal Heat | Effect |
|------|-------------|-------------|-------------|--------|
| **L0 Physics Probe** | **Resonance Break** | 60% | Critical | Destroys a "Fracture Point" wall or disables a machine. |
| **L1 Biology Spectrometer** | **Adrenaline Spike** | 50% | High | Instantly stabilizes an NPC's health OR forces them into a Panic state. |
| **L2 Cognitive Resonator** | **Neural Over-Resonate** | 50% | High | Forces an NPC to leak a high-tier **Intelligence** or **Rumor**. |
| **L3 Economic Lens** | **Market Injection** | 45% | Moderate | Temporarily lowers/raises prices at a single shop node by 25%. |
| **L4 Political Scouter** | **Directive Spoof** | 70% | Critical | Temporarily ignore one **Directive** (e.g., Curfew) for 60 seconds. |

#### Scenario: Direct Action draws immediate response
- **WHEN** the player (possessing Tier 3) uses a **Resonance Break** (L0) to enter a restricted area
- **THEN** it SHALL deplete 60% of their Neural Charge and emit a "Critical" Signal Signature, instantly alerting all nearby Faction Enforcers and Sentinel Drones.

### Requirement: Tool-Lure Mechanics (Risk of Detection)
To encourage discrete usage and reward stealth, each tool SHALL have a "Lure Profile" that attracts specific hostile entities when activated. High-signature usage SHALL not only alert guards but actively "summon" specialized predators.

| Layer Tool | Lure Type | Entity Attracted | Rationale |
|------------|-----------|------------------|-----------|
| **L0 Physics Probe** | **Electronic** | **Sentinel Drones** | High-energy pulses trigger automated security sweeps. |
| **L1 Biology Spectrometer** | **Biometric** | **Feral Xeno-Predators** | The scan "pings" the biological frequency that xenos use for hunting. |
| **L2 Cognitive Resonator** | **Neural** | **Neural Phantoms** | High-resonance cognitive states attract entities that feed on focus. |
| **L3 Economic Lens** | **Visual/Digital** | **Scavenger Thieves** | The gold "Provenance Bloom" is visible to NPCs with high Greed needs. |
| **L4 Political Scouter** | **Administrative** | **Faction Enforcers** | Unauthorized territorial mapping is flagged as a Level 4 Sedition event. |
| **GhostDrone** | **Electronic/Visual** | **Sentinel Drones** | Unsanctioned drone flight in restricted Z-space triggers an intercept. |

#### Scenario: Tool use draws non-human threats
- **WHEN** the player deploys an illicit **GhostDrone** in a CORPORATE or URBAN_CORE zone
- **THEN** it SHALL emit a signal signature (based on Tier) that can alert **Sentinel Drones**, leading to a high-speed chase through the "skyscraper canyons."

### Requirement: Cross-Layer Event Propagation
Events SHALL propagate across layers through a causal chain. The game SHALL support both upward propagation (physics → biology → cognition → economics → politics) and downward propagation (political decisions → economic policy → cognitive stress → biological consequences → physical infrastructure).

**Canonical Example — The Mugging Cascade**:
1. **L2 Cognitive**: A desperate citizen mugs a pedestrian
2. **L1 Biology**: The victim sustains injuries — health loss, injury status
3. **L2 Cognitive**: Witnesses form fear memories, the victim is traumatized
4. **L3 Economic**: Medical costs for the victim, the stolen credits change hands, local business confidence drops
5. **L4 Political**: The crime is reported, crime statistics update, the area's safety rating drops, the dominant faction considers deploying more patrols (Directives), public opinion shifts

#### Scenario: Complete cascade visibility
- **WHEN** a significant event triggers a multi-layer cascade
- **THEN** the player SHALL be able to trace the cascade through diagnostic tools — seeing the originating event and its consequences propagating upward through each layer over subsequent ticks

### Requirement: Layer Update Cadence (Creative Framing)
Layers SHALL update at different conceptual rates reflecting their temporal nature:
- **L0 Physics and L1 Biology**: Fast — physical and biological processes are immediate and continuous
- **L2 Cognitive**: Moderate — thoughts and emotions form over slightly longer timescales
- **L3 Economic**: Slow — markets aggregate over economic cycles
- **L4 Political**: Slowest — political change is deliberate and institutional

The specific tick rates and scheduling are a technical concern for the successor's architecture, but the *creative intent* is that the player SHALL perceive physics as immediate, economics as gradual, and politics as slow-moving but powerful.

#### Scenario: Temporal perception of layers
- **WHEN** the player observes the city over time
- **THEN** they SHALL notice that physical effects happen immediately (building collapse), biological effects develop quickly (health loss/bleeding), cognitive effects ripple over hours (fear spreading), economic effects accumulate over days (price changes), and political effects evolve over weeks (Directive shifts)
