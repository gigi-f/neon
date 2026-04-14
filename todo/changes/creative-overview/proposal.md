## Why

Neon Oubliette is being restarted as an SDL2-based spiritual successor. The original Notcurses terminal implementation proved the creative vision and entity systems, but major technical systems need rethinking for windowed graphical rendering. Before rebuilding, the game's creative DNA must be captured in a single authoritative reference — a creative bible that preserves what makes Neon Oubliette *Neon Oubliette*, independent of any rendering backend or engine architecture. This document ensures that the soul of the game survives the migration.

## What Changes

- **Creative Foundation Document**: A comprehensive creative overview capturing the game's identity, tone, world-building, and design philosophy — written for humans building the successor, not as code documentation.
- **Entity System Preservation**: Faithful cataloging of all entity types, agent behaviors, item categories, and creature definitions that the developer has validated and wants carried forward.
- **Simulation Layer Philosophy**: Creative framing of the five-layer causal simulation (Physics → Biology → Cognitive → Economic → Political) as a narrative engine, not a technical specification.
- **Faction & Religion Canon**: Definitive reference for the five post-human faction leaders, six religions, their interrelationships, and mechanical expressions.
- **Narrative Systems Codex**: Documentation of the dialogue system philosophy, speech profiles, procedural grammar concepts, and Ink-based "Gold Path" approach.
- **Crisis & Emergence Design**: The emergent narrative philosophy, seven crisis types, and cross-layer cascade patterns that make the city feel alive.

## Capabilities

### New Capabilities
- `world-and-setting`: The mega-city as creative concept — zones, verticality, atmosphere, environmental hazards, and the "oubliette" metaphor.
- `factions-and-leaders`: Five post-human intelligence factions, their AI leaders, philosophies, aesthetic identities, directive mechanics, and religion stances.
- `religions-and-belief`: Six religions with dogma systems, holy days, faction affinities, worship mechanics, and suppression dynamics.
- `entities-and-creatures`: Complete entity catalog — species, xeno types, drones, NPC archetypes, life stages, agent task vocabulary, item categories, and building typology.
- `economy-and-crime`: Markets, supply chains, contraband economy, wanted/reputation system, political economy, corruption, and black market mechanics.
- `simulation-layers`: The five-layer causal cascade as a creative/narrative framework — how physics feeds biology feeds cognition feeds economics feeds politics.
- `dialogue-and-narrative`: Procedural grammar engine philosophy, Ink "Gold Paths", speech profiles, audibility model, information types, graffiti culture, and world myths.
- `crisis-and-emergence`: Seven crisis types, cascade examples, emergent narrative design philosophy, and the "every system has a visible symptom" rule.

### Modified Capabilities
_(None — this is a greenfield creative document for the successor project.)_

## Impact

This creative overview affects no existing code. It serves as the authoritative reference for all future implementation work on the SDL2 successor, informing:
- Entity component design and data schemas
- Content pipelines (items, creatures, factions, religions)
- Dialogue and narrative tooling choices
- World generation parameters and zone design
- UI/UX design language and color vocabulary
- AI behavior tree design for agents and faction directives
- Crisis and event system architecture
