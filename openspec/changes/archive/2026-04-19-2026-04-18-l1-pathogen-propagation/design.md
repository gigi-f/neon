## Context

`BiologySystem` already runs at the L1 gate and owns direct health, organ, and vital-sign pressure. Citizens have `TransformComponent` positions outdoors and may carry `InteriorComponent` when inside a building. This change adds disease state without changing the existing death/event pipeline.

## Goals / Non-Goals

**Goals:**
- Represent active infection/exposure as a compact component.
- Spread pathogens through close outdoor proximity.
- Spread pathogens among entities sharing the same `InteriorComponent.building_entity`.
- Progress infection into health drain, lung stress, fever-like heart-rate increase, and oxygen reduction.
- Cascade symptomatic infections into L2 PAD mood and a one-shot illness memory.
- Tint infected glyphs so active pathogen state is visible during normal play.
- Seed rare infected citizens during ambient spawning.

**Non-Goals:**
- No rumor, quarantine, or policy cascade.
- No pathogen inventory, masks, medicine, quarantine, or policy systems.
- No stochastic per-frame random transmission; use deterministic exposure accumulation for stable behavior.

## Decisions

### D1: Pathogen state lives only on exposed/infected entities

Entities without `PathogenComponent` are susceptible and pay no per-entity disease storage cost. A new exposure assigns the component; repeat exposures to the same strain increase `infection_load`.

### D2: Deterministic exposure dose

Each infectious source contributes dose based on:
- outdoor proximity within 24 WU;
- same building via `InteriorComponent`;
- source `infectiousness`;
- current `time_scale * dt`.

This keeps the simulation reproducible enough for debugging and avoids adding another random source inside the L1 loop.

### D3: Shared interiors ignore distance

Entities in the same building are treated as sharing air. The dose is lower than direct proximity but applies regardless of exact local coordinates, matching the coarse current interior representation.

### D4: Progression is direct L1 biology pressure

After incubation, severity rises with infection load and drains health, lungs, and oxygen saturation. Immune response slowly clears load. Critical death remains handled by `BiologySystem` on a later L1 tick when health or critical organs fail.

### D5: L2 impact stays inside PathogenSystem

`PathogenSystem::update()` accepts an L2 gate flag. When true, symptomatic infections lower PAD pleasure, raise arousal, and record one `BECAME_ILL` memory once symptoms become severe. This keeps the upward cascade close to the biological source while preserving L2 tick cadence.

### D6: Infected count is component-based

`PathogenSystem::infectedCount()` reports entities carrying `PathogenComponent`. The debug HUD uses this small counter directly, matching the deterministic test expectation.

### D7: Infection visibility uses glyph tint

Entities carrying `PathogenComponent` render with a warm orange/yellow glyph tint. The tint intensity rises with severity. This gives immediate visual confirmation without adding a separate diagnostic mode or input binding.

## Risks / Trade-offs

- The current shared-space model is coarse because interiors do not have rooms or airflow yet.
- A pair loop over biological entities is acceptable at current ambient counts; sparse-set storage remains a later optimization if entity counts grow.
- Seeded infection is intentionally rare to avoid making every run immediately dominated by disease.
- The L2 cascade is intentionally personal only; social spread of outbreak fear belongs in a later rumor or crisis system.
