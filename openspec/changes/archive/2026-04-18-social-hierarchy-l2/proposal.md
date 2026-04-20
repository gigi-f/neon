## Why

L2 currently has PAD emotion, BDI goals, schedules, relationships, and economic data — but no concept of *where a citizen sits in the social order*. The `dominance` PAD axis exists but is never written to by any system; it defaults to 0.0 for everyone. Social hierarchy positioning closes that gap: it gives every citizen a rank derived from their economic state, uses that rank to drive the `dominance` axis, and exposes visible glyph differentiation so rank is legible at a glance.

## What Changes

- New `SocialRankComponent` on every citizen (rank enum + prestige float).
- New `SocialHierarchySystem` (L2, 2 Hz): rank assignment from economic state, dominance axis update, proximity prestige exchange, speed modulation.
- `spawnCitizens()` assigns `SocialRankComponent` and sets initial glyph char by rank after EconomicComponent is assigned.

## Capabilities

### New Capabilities
- `social-rank`: SocialRankComponent carrying a 5-tier rank enum and a continuous prestige float; drives dominance PAD axis and CitizenComponent.speed

### Modified Capabilities
- `citizen-spawn`: spawnCitizens() assigns SocialRankComponent + sets initial glyph char per rank

## Impact

- `src/components.h` — add `SocialRank` enum, `SocialRankComponent` struct under `// ── L2 Cognitive ──` banner
- `src/simulation_systems.h` — add `SocialHierarchySystem`; patch `spawnCitizens()` and `CitizenAISystem`
- `src/main.cpp` — call `SocialHierarchySystem::update()` at the L2 tick gate
- `todo/TODO.md` — mark "Social hierarchy positioning" checked
