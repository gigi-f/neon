## Context

The Neon Oubliette simulation requires a dynamic environment to challenge the various simulation layers (Structural L0, Biological L1). Currently, we have weather states (CLEAR, OVERCAST, ACID_RAIN, SMOG) defined in `WeatherState` and referenced in `TimeOfDayComponent`, but they do not transition dynamically based on the environment or have significant, coordinated effects across all systems. The TODO list explicitly calls out "Environmental hazard events (flooding placeholder, smog, acid rain)".

## Goals / Non-Goals

**Goals:**
- Implement a `HazardSystem` or extend `TemperatureSystem`/`AtmosphereSystem` to handle the procedural triggering and duration of environmental hazards.
- Implement the effects of `ACID_RAIN` on structural decay (accelerated decay).
- Implement the effects of `SMOG` on the `AtmosphereGridComponent` and subsequent L1 biology impacts.
- Implement a `FLOODING` placeholder hazard that restricts movement or applies a debuff in specific lowland areas.

**Non-Goals:**
- Highly complex meteorological modeling (e.g., dynamic wind vectors driving smog plumes).
- Permanent structural destruction beyond what the current `StructuralDecaySystem` provides.

## Decisions

- **Hazard Triggering**: The `TimeOfDaySystem` will manage global weather state transitions based on simple probabilistic rules during phase changes (e.g., DAY to DUSK).
- **Acid Rain**: The `StructuralDecaySystem` already has a spec for accelerated decay under `ACID_RAIN`. We will implement this behavior.
- **Smog**: When `SMOG` is active, the `AtmosphereSystem` will globally reduce the `air_quality` of the `AtmosphereGridComponent` cells, triggering health penalties in `BiologySystem`.
- **Flooding (Placeholder)**: We will add a simple random event that flags certain low-elevation `ZoneType`s (e.g., SLUMS) as flooded, reducing NPC movement speed via a modifier.

## Risks / Trade-offs

- **Risk: Rapid NPC death during smog events.**
  - Mitigation: Tune the health penalty to be slow, acting more as a continuous drain on L1 resources (medical/food) than an instant kill.
- **Risk: Performance cost of checking weather states for every entity.**
  - Mitigation: Cache the current weather state locally in the systems, avoiding constant global lookups.
