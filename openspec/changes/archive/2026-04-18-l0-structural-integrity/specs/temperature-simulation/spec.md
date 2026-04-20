## Purpose

Extends the thermal simulation with environmental weather states like Acid Rain.

## Requirements

## ADDED Requirements

### Requirement: Environmental Weather States
The `TemperatureSystem` SHALL manage a global `WeatherState` enum that includes: `CLEAR`, `OVERCAST`, `ACID_RAIN`, and `SMOG`.

#### Scenario: Weather transitions to Acid Rain
- **WHEN** the ambient humidity (calculated from temperature and zone) is high and the temperature is below 25.0°C
- **THEN** there SHALL be a probabilistic chance to transition to `ACID_RAIN`.

#### Scenario: Acid Rain impacts L1 Biology
- **WHEN** the global weather state is `ACID_RAIN`
- **THEN** any NPC with `is_exposed = True` SHALL receive a minor health penalty of 0.5 per L2 tick.
