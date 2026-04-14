# Resource-Intensive Feature Flags

The following features in the Neon Oubliette specification are identified as high-risk for performance or technical complexity. They require careful optimization or may need to be simplified if the target hardware/engine cannot handle the load.

| Feature | Spec File | Risk Factor | Optimization Strategy |
|---------|-----------|-------------|-----------------------|
| **Real-Time 5-Layer Simulation** | `simulation-layers/spec.md` | **Critical**: Running 5 causally linked layers for thousands of agents simultaneously is computationally expensive. | Use staggered update rates (Physics/Biology = fast; Politics = very slow). Simulate only active regions; use statistical aggregation for "off-screen" districts. |
| **BDI Agent Intelligence** | `entities-and-creatures/spec.md` | **High**: Belief-Desire-Intention utility calculations for 1000+ NPCs can spike CPU usage. | Batch AI updates. Use a simplified "state machine" for background NPCs and full BDI only for NPCs near the player. |
| **Real-Time Continuous Pathfinding** | `entities-and-creatures/spec.md` | **High**: Moving thousands of agents through a continuous world space without grid-snapping. | Use Hierarchical Pathfinding (HPA*). Cache common paths. Use local steering (Flow fields/RVO) for crowd movement. |
| **Procedural Interior Generation** | `world-and-setting/spec.md` | **High**: Generating unique floor layouts for hundreds of 150-story skyscrapers. | Lazy generation: only generate an interior when a player enters. Use "Building Templates" with minor randomization rather than fully unique generation for every floor. |
| **Continuous AABB Collision** | `ambient-city-traffic/spec.md` | **Moderate**: Real-time collision for hundreds of ambient vehicles and pedestrians. | Use Spatial Partitioning (Quadtrees/Grids). Despawn ambient entities aggressively outside the camera radius. |
| **Organic Social Networks** | `dialogue-and-narrative/spec.md` | **Moderate**: Tracking relationships and rumor propagation across thousands of agents. | Limit social graph depth. Use "Social Chunks": only process interactions for agents currently in active simulation zones. |
| **Procedural Grammar Engine** | `dialogue-and-narrative/spec.md` | **Low-Moderate**: Dynamic text generation for every NPC interaction. | Cache generated strings. Use a lightweight templating system rather than a complex NLP library. |

---

# Terminology Migration (Originality Update)

To ensure creative originality and avoid direct overlap with *Book of the New Sun*, the following terms have been renamed across all specifications.

| Original Term | New Term | Rationale |
|---------------|----------|-----------|
| **Hierodule** | **Archon** | "Archon" suggests an ancient, high-status ruler/servant without the specific Wolfe-ian religious connotation. |
| **Cacogen** | **Aborrax** | Replaces the specific "Cacogen" term with a more visceral, descriptive title for alien threats. |
| **Megatherian** | **Behedicci** | Maintains the sense of scale and ancient power while using a more universal mythological term. |
| **Oubliette** (City Name) | **Oubliette** | *Retained per user preference. The name fits the "forgotten dungeon" theme perfectly.* |

---

# Simplified / Optimized Systems (Previous Cuts)

The following systems were identified as low-fun/high-resource and have been removed or simplified:
- **Organ-Level Biology**: Replaced with a unified Health/Stress meter and status effects.
- **Stock Market / Elections**: Replaced with Faction Bounties/Contracts and Faction Directives.
- **Articulated Vehicle Physics**: Large vehicles (Trains/Buses) are now rigid collision bodies.
- **Hunger/Thirst Needs**: Consolidated into a single "Metabolism" need.
- **Generic Item Provenance**: Ownership history now only applies to Unique/High-Value items.
