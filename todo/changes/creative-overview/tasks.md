# Neon Oubliette: Implementation Roadmap (Triaged Complexity)

## Phase 0: MVP / Proof of Concept (Foundational)
*Goal: Get a box moving in a boxy world.*
- [ ] 0.1 Create SDL2 project scaffold (C++20, CMake, EnTT, GLM).
- [ ] 0.2 Implement basic SDL_Renderer wrapper with "Neon Noir" (#0A0A0F) clear color.
- [ ] 0.3 Generate a 128x128 WU static **URBAN_CORE** zone (4x4 blocks, hard-coded grid, 6 WU roads).
- [ ] 0.4 Implement **PlayerTag** entity (Green Box) with WASD movement.
- [ ] 0.5 Implement **CameraSystem** (center follow) and AABB Collision with buildings.
- [ ] 0.6 Render static NPCs (Grey Boxes) and Buildings (Outlined Rectangles).

- [ ] 0.7 Implement basic time-of-day cycle (simple DAWN/DAY/NIGHT visual shift)
- [ ] 0.8 Add basic debug HUD (FPS, player position, tick rate)

## Phase 1: The Simulation Bus & Data (Low Complexity)
*Goal: Set up the plumbing for the 5-layer engine.*
- [ ] 1.1 Establish the **Simulation Coordinator**: Phased tick scheduling (L0 fast -> L4 slowest) and cross-layer event propagation bus.
- [ ] 1.2 Implement the **Data Pipeline**: JSON config loading for Religions, Species, Item Categories, and Drone/Robot archetypes.
- [ ] 1.3 Integrate **Open Source Icon Library** (Lucide/Heroicons): Thin-stroke diagnostic icons for all HUD elements.
- [ ] 1.4 **Neural Battery & Signal Heat**: Implement core resource meters and detection risk logic.
- [ ] 1.5 **Tier 1 Tools**: Initialize player with 5 Investigative Tools at Baseline (Legacy Firmware) level.

- [ ] 1.6 Add basic logging and error reporting system
- [ ] 1.7 Implement build presets for Debug/Release

## Phase 2: The Organic Loop (Medium Complexity)
*Goal: Physics, Biology, and the consequences of death.*
- [ ] 2.1 **L0 Physics Foundation**: Cell-based temperature, pressure, material states, and structural integrity (decay + acid rain acceleration).
- [ ] 2.2 **L1 Biology Foundation**: Unified Health/Stress model, injury statuses (e.g., "Internal Bleeding"), and metabolic decay (Hunger/Thirst).
- [ ] 2.3 **The Recovery**: Implement the **Assigned Pod (Tier 0)** spawn point and the **Engineer's Diary** lore item.
- [ ] 2.4 **The Erasure**: Implement the multi-layer death cascade (L0-L4 impact: item loss, credit strip, 25% info loss, ghost signature heat).
- [ ] 2.5 **Reclamation & Disposal**: Implement **ReclamationDrones** that retrieve bodies and transport them to the **CREMATORIUM** loop.
- [ ] 2.6 **Organic Items**: Implement FOOD, WATER, and MEDICAL categories with L1 status effects.
- [ ] 2.7 **Pathogen System**: Implement infection spread through proximity and shared spaces (upward propagation to L2 trauma).

- [ ] 2.8 Implement basic environmental hazards (e.g., placeholder acid rain, smog, or flooding event)
- [ ] 2.9 Add simple inventory UI for survival items

## Phase 3: The Social Layer (High Complexity - Narrative)
*Goal: NPCs that talk, remember, and can be overheard.*
- [ ] 3.1 **Audibility Model**: Implement the "Glitch-Text" HUD (Missing letters replaced by dashes resolved by proximity/augments).
- [ ] 3.2 **Eavesdropping**: NPCs engage in real-time dialogue based on needs/tasks; overhearing reveals free **INTELLIGENCE/LORE**.
- [ ] 3.3 **Hearing Augments**: Tier 1-3 audio implants (passive vs. neural drain) and the **Neural-Audio Recorder** (fragment capture).
- [ ] 3.4 **L2 Cognitive Foundation**: Pleasure/Arousal/Dominance (PAD) emotional axes and BDI (Belief-Desire-Intention) memory formation.
- [ ] 3.5 **Witness Memory & Social Graph**: Agents remember events (witness trauma), form social links, and share **RUMORS/GOSSIP** in conversation.
- [ ] 3.6 **Hybrid Dialogue**: Procedural grammar engine (6 speech profiles) + **Ink scripting** integration for Gold Path (Leaders/Fixers).
- [ ] 3.7 **Information System**: Typed knowledge economy (RUMOR, PROPAGANDA, INTELLIGENCE, PRICE_TIP) with organic propagation.

- [ ] 3.8 Implement basic NPC schedule state machine (SLEEPING/WORKING/LEISURE)
- [ ] 3.9 Add simple relationship/affinity tracking for NPCs

## Phase 4: The Systemic Layer (High Complexity - Simulation)
*Goal: Markets, Factions, and the Invisible Hand.*
- [ ] 4.1 **L3 Economic**: Local supply/demand markets, item **Provenance** (high-value only), and supply chain flows (Extraction -> Production -> Market).
- [ ] 4.2 **L4 Political**: Territorial ownership maps (Ownership Lines) and the **Faction Directive** system (zone-wide policy changes).
- [ ] 4.3 **Reputation & Wanted**: 0-5 Star escalating guard response and factionstanding (-100 to +100) with tier effects.
- [ ] 4.4 **The Routine**: Implement the **Box Sorting** mini-game and the L4 Political risk (Level 1 Sedition for ignoring directives).
- [ ] 4.5 **Directive Markets**: Implementation of economic foreknowledge (Scouting projected laws to short-sell/corner resources).
- [ ] 4.6 **Crisis & Emergence**: Implement 7 crisis types (Collapse, Outbreak, etc.) with dashboards and cross-layer cascades.
- [ ] 4.7 **Corruption & Crime**: Bribery, Favors, backroom deals, and clandestine drug labs with raid mechanics.

- [ ] 4.8 Implement basic zone/district system (AERODROME, TRANSIT, etc.) with height gradients
- [ ] 4.9 Add placeholder for vertical Z-axis transitions (stairs/elevators)

## Phase 5: World Scaling & The Long Game (Very High Complexity)
*Goal: A living mega-city and the five post-human gods.*
- [ ] 5.1 **Procedural City Generation**: Full 256x256 WU gen with all 14 zone types and Chicago-style height gradients.
- [ ] 5.2 **High-Fidelity Interiors**: Architectural adjacency rules and functional prop-sets (Desks, Racks, Containers).
- [ ] 5.3 **Futuristic Infrastructure**: Maglift Monorails, VTOL Hover Vessels (Aerodromes), and EMMV Auto-Pods.
- [ ] 5.4 **Advanced BDI AI**: Final NPC decision system with utility-based selection for 40+ task types.
- [ ] 5.5 **Xeno System**: Implement all 4 xeno types (Archon, Aborrax, Behedicci, Post-Human) with radius-based layer warping.
- [ ] 5.6 **Tool Masteries**: Implement Tier 2 (Enhanced Diagnostics) and Tier 3 (Direct Actions) tool upgrades and firmware modules.
- [ ] 5.7 **The Sanctuary**: Claimable player housing with 5-layer defenses (Jammers, DNA-locks) and Housing Brokers.
- [ ] 5.8 **AGI Cores**: Reaching the physical Throne Rooms of the 5 AGIs via the 5-Layer Security Gauntlet.
- [ ] 5.9 **Endgame**: Implementation of the 4 branching scenarios (Purge, Avatar, Synthesis, Reset).
