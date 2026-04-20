## Overview

Add a lightweight `SimulationAlertSystem` that observes state already produced by existing systems and appends player-facing alert records to a bounded stack. The main loop owns the stack, passes it to systems that can emit events, and renders the visible recent entries in the HUD.

## Data Model

`SimulationAlert` stores:
- severity (`INFO`, `WARNING`, `DANGER`)
- category (`WEATHER`, `FLOOD`, `INFECTION`, `STRUCTURE`)
- message text
- `game_hour`
- display TTL in real seconds

`SimulationAlertStack` owns a bounded `std::deque<SimulationAlert>` and supports:
- `push(...)`
- `update(dt)` to age visible alerts
- `recent()` for rendering and later Intel Log integration

## Event Sources

- Weather: compare previous and current `TimeOfDayComponent.weather` after `TimeOfDaySystem` and `TemperatureSystem` have run.
- Flooding: emit when the debug flood toggle changes `TimeOfDayComponent.is_flooded`.
- Infection: extend `PathogenSystem::update()` with an optional alert stack. Emit when a pathogen crosses a severity threshold from below to above, using a per-component alert tier to avoid repeats.
- Collapse: extend `StructuralDecaySystem::update()` with an optional alert stack. Emit when a `CollapsedComponent` is first assigned.

## Rendering

Render up to four non-expired alerts below the existing survival HUD. Use the existing bitmap font and severity colors. Do not add panels, modal UI, or full-screen visual effects.

## Test Strategy

Add deterministic unit tests for:
- bounded stack behavior and TTL expiry
- weather/flood alert emission helper behavior
- infection severity crossing emits once per tier
- structural collapse emits when integrity reaches zero
