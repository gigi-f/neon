# Tasks: Social Hierarchy Positioning (L2)

## Task 1 — Add SocialRank enum and SocialRankComponent to components.h
- Insert under `// ── L2 Cognitive ──` banner (before CognitiveComponent)
- `enum class SocialRank { VAGRANT=0, SLUM_DWELLER=1, WORKING_POOR=2, MIDDLE_CLASS=3, CORPORATE_ELITE=4 };`
- `struct SocialRankComponent { SocialRank rank = SocialRank::WORKING_POOR; float prestige = 0.0f; };`

## Task 2 — Add SocialHierarchySystem to simulation_systems.h
- Add static helpers `rankFromEco()` and `speedForRank()` inside the class
- Pass 1: rank refresh + dominance nudge
- Pass 2: prestige decay + proximity prestige exchange

## Task 3 — Patch spawnCitizens() in simulation_systems.h
- After EconomicComponent assign: compute rank, assign SocialRankComponent, update CitizenComponent.speed and GlyphComponent.chars

## Task 4 — Wire SocialHierarchySystem in main.cpp
- Declare `SocialHierarchySystem socialHierarchySystem;` with other systems
- Call `socialHierarchySystem.update(registry);` at L2 tick gate

## Task 5 — Mark TODO.md
- Check off "Social hierarchy positioning" under Phase 7 L2
