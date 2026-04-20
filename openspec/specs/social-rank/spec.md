## ADDED Requirements

### Requirement: SocialRankComponent
Every citizen NPC SHALL carry a `SocialRankComponent` with two fields:
- `rank`: `SocialRank` enum — tier from VAGRANT(0) to CORPORATE_ELITE(4)
- `prestige`: float [-1.0, +1.0] — smooth position within the tier; starts at 0.0

```
enum class SocialRank { VAGRANT = 0, SLUM_DWELLER = 1, WORKING_POOR = 2, MIDDLE_CLASS = 3, CORPORATE_ELITE = 4 };
struct SocialRankComponent { SocialRank rank = SocialRank::WORKING_POOR; float prestige = 0.0f; };
```

Declared in `components.h` under the `// ── L2 Cognitive ──` banner.

---

### Requirement: Rank Assignment at Spawn
`spawnCitizens()` SHALL assign `SocialRankComponent` immediately after `EconomicComponent` is assigned, using the following mapping:

| Condition (checked in order) | Rank |
|---|---|
| unemployed AND credits < 30 | VAGRANT |
| unemployed OR daily_wage < 8 | SLUM_DWELLER |
| daily_wage in [8, 15) | WORKING_POOR |
| daily_wage in [15, 25) | MIDDLE_CLASS |
| daily_wage >= 25 | CORPORATE_ELITE |

After rank is set, `CitizenComponent.speed` and initial glyph char SHALL be written per rank:

| Rank | speed (WU/s) | glyph char |
|---|---|---|
| VAGRANT | 18.0 | `.` |
| SLUM_DWELLER | 24.0 | `i` |
| WORKING_POOR | 30.0 | `i` |
| MIDDLE_CLASS | 36.0 | `I` |
| CORPORATE_ELITE | 44.0 | `I` |

#### Scenario: Unemployed broke citizen
- **WHEN** a citizen is spawned with no employer and credits < 30
- **THEN** their rank SHALL be VAGRANT and speed SHALL be 18.0

#### Scenario: High-wage citizen
- **WHEN** a citizen is spawned with daily_wage >= 25
- **THEN** their rank SHALL be CORPORATE_ELITE and speed SHALL be 44.0 with glyph `I`

---

### Requirement: SocialHierarchySystem
`SocialHierarchySystem::update(registry)` SHALL run at the L2 tick gate (~2 Hz).

**Pass 1 — rank refresh and dominance update** (per citizen with SocialRankComponent + EconomicComponent + CognitiveComponent):
- Recompute rank using the same mapping as spawn (EconomicComponent is live).
- If rank changed, update speed per the rank→speed table.
- Compute `dominance_target = (static_cast<int>(rank) / 4.0f) * 2.0f - 1.0f`
  (VAGRANT → -1.0, MIDDLE_CLASS → 0.0, CORPORATE_ELITE → +1.0)
- Nudge: `cog.dominance += (dominance_target - cog.dominance) * 0.05f`

#### Scenario: Newly unemployed citizen loses dominance
- **WHEN** a citizen becomes unemployed and drops to SLUM_DWELLER rank
- **THEN** their dominance SHALL drift toward −0.5 at 5% per L2 tick

**Pass 2 — proximity prestige exchange** (N² over citizens with SocialRankComponent + TransformComponent, radius 50 WU):
- For each pair (A, B) within 50 WU:
  - If rank(A) > rank(B): `A.prestige = min(1.0, A.prestige + 0.01)`, `B.prestige = max(-1.0, B.prestige - 0.005)`
  - If rank(A) < rank(B): symmetric inverse
  - If equal rank: no change

Prestige passively decays toward 0 each tick: `prestige *= 0.98f`

#### Scenario: Elite near vagrant
- **WHEN** a CORPORATE_ELITE citizen is within 50 WU of a VAGRANT
- **THEN** the elite's prestige SHALL increase and the vagrant's prestige SHALL decrease each L2 tick

---

### Requirement: Infrastructure Unchanged
All existing systems (CognitiveSystem, GDISystem, ScheduleSystem, RelationshipSystem, WageSystem, BiologySystem) SHALL NOT be modified.
The ScheduleSystem's glyph color overrides continue to apply on top of spawn-time glyph char choices.
