## Context

Currently, the L1 Biology simulation uses a generic `BiologyComponent` with fields for `health`, `hunger`, `thirst`, and `fatigue`. When `health` hits zero, the entity dies. This lacks the granularity needed for complex interactions such as specific injuries affecting specific functions, pathogens attacking particular systems, or nuanced death scenarios (e.g., bleeding out vs. brain damage). By introducing `OrganHealth` and vital signs, the simulation can support a deeper layer of systemic interplay.

## Goals / Non-Goals

**Goals:**
- Provide per-organ health tracking for Heart, Lungs, Brain, Liver, and Kidneys.
- Implement basic vital signs (Blood Pressure, Heart Rate, Oxygen Saturation).
- Enable overall health to dynamically derive from or degrade based on organ status.
- Ensure the data fits cleanly within the ECS `BiologyComponent` for memory cache coherence.

**Non-Goals:**
- Full biomechanical simulation of every human organ.
- Realistic disease pathogen modeling (this is left for another phase).
- Complex surgical interventions beyond simple consumable medical items.

## Decisions

- **Embedded vs. Separate Component:** We will embed `OrganHealth` and `VitalSigns` structures directly into `BiologyComponent`. This avoids ECS bloat with too many fine-grained components and keeps biological data localized for cache-friendly updates in `L1_BiologySystem`.
- **Organ Metrics:** Organs will be represented by an array or structured fields of floats [0.0f - 100.0f]. We will use 5 primary organs: Heart, Lungs, Brain, Liver, and Kidneys.
- **Vital Signs:** Vitals will be derived from organ health and metabolic state. 
    - `blood_pressure`: Float array [Systolic, Diastolic].
    - `heart_rate`: Float (bpm).
    - `oxygen_saturation`: Float (percentage 0.0-1.0 or 0-100).
- **Health Cascade:** If a critical organ (Brain, Heart) drops below a threshold, `health` drops rapidly. If non-critical organs drop, `health` drops slowly.

## Risks / Trade-offs

- **Risk: Increased Cache Pressure:** Adding floats for 5 organs and 3 vital signs increases the size of `BiologyComponent`. 
  - *Mitigation:* The size increase is about 32-40 bytes. This is acceptable for L1 logic, but we should monitor L1 processing time during the tick-rate gating.
- **Risk: Complexity in Balancing:** Tuning decay rates for 5 organs plus vitals can lead to unpredictable death spirals.
  - *Mitigation:* Start with very slow baseline decay rates for organs and rely heavily on explicit events (injuries, starvation) to impact them.
