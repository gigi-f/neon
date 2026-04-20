# Design: Social Hierarchy Positioning (L2)

## Component Layout

```cpp
// components.h — under // ── L2 Cognitive ──
enum class SocialRank { VAGRANT = 0, SLUM_DWELLER = 1, WORKING_POOR = 2, MIDDLE_CLASS = 3, CORPORATE_ELITE = 4 };

struct SocialRankComponent {
    SocialRank rank     = SocialRank::WORKING_POOR;
    float      prestige = 0.0f; // [-1, +1]
};
```

## Rank Mapping Helper (static, local to SocialHierarchySystem)

```cpp
static SocialRank rankFromEco(const EconomicComponent& eco) {
    if (eco.employer == MAX_ENTITIES && eco.credits < 30.0f) return SocialRank::VAGRANT;
    if (eco.employer == MAX_ENTITIES || eco.daily_wage < 8.0f)  return SocialRank::SLUM_DWELLER;
    if (eco.daily_wage < 15.0f) return SocialRank::WORKING_POOR;
    if (eco.daily_wage < 25.0f) return SocialRank::MIDDLE_CLASS;
    return SocialRank::CORPORATE_ELITE;
}

static float speedForRank(SocialRank r) {
    switch(r) {
        case SocialRank::VAGRANT:         return 18.0f;
        case SocialRank::SLUM_DWELLER:    return 24.0f;
        case SocialRank::WORKING_POOR:    return 30.0f;
        case SocialRank::MIDDLE_CLASS:    return 36.0f;
        case SocialRank::CORPORATE_ELITE: return 44.0f;
    }
    return 30.0f;
}
```

## SocialHierarchySystem::update() Pseudocode

```
Pass 1: view<SocialRankComponent, EconomicComponent, CognitiveComponent, CitizenComponent>
  for each e:
    newRank = rankFromEco(eco)
    if newRank != sr.rank:
        sr.rank = newRank
        citizen.speed = speedForRank(newRank)
    target = (int(rank) / 4.0) * 2.0 - 1.0
    cog.dominance += (target - cog.dominance) * 0.05

Pass 2: collect list of view<SocialRankComponent, TransformComponent>
  prestige decay: sr.prestige *= 0.98
  N² proximity pairs within 50 WU:
    delta_rank = int(rA) - int(rB)
    if delta_rank > 0: srA.prestige = clamp(srA.prestige+0.01, -1,1); srB.prestige = clamp(srB.prestige-0.005,-1,1)
    if delta_rank < 0: symmetric
```

## spawnCitizens() patch

After `registry.assign<EconomicComponent>(citizen, eco)`:
```cpp
SocialRank sr = rankFromEco(eco);
registry.assign<SocialRankComponent>(citizen, sr, 0.0f);
// Update speed
auto& c = registry.get<CitizenComponent>(citizen);
c.speed = speedForRank(sr);
// Update glyph char
auto& g = registry.get<GlyphComponent>(citizen);
g.chars = (sr >= SocialRank::MIDDLE_CLASS) ? "I" : (sr == SocialRank::VAGRANT ? "." : "i");
```

## main.cpp L2 gate

```cpp
// Existing:
scheduleSystem.update(registry, tod.game_hour);
gdiSystem.update(registry, l2_dt, tod.time_scale);
relationshipSystem.update(registry);
// Add:
socialHierarchySystem.update(registry);
```

## File touch list
- `src/components.h` — SocialRank enum + SocialRankComponent struct
- `src/simulation_systems.h` — SocialHierarchySystem class + spawnCitizens() patch
- `src/main.cpp` — declare + call socialHierarchySystem at L2 gate
- `todo/TODO.md` — check off "Social hierarchy positioning"
