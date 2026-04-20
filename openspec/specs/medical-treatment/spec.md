## Purpose
Make medical consumables injury-aware so they treat the worst active injury before falling back to direct health restoration.

## Requirements

### Requirement: MEDICAL item heals worst injury first
In `ConsumableSystem::update()`, the MEDICAL branch (currently `bio.health += restore_value`) SHALL change to injury-aware consumption:

1. If the consumer entity has `InjuryComponent` with at least one active slot:
   a. Find the slot with the highest `severity`.
   b. Reduce that slot's `severity` by `0.5f` (clamped to 0.0).
   c. If `severity` reaches 0.0, clear the slot (`type = InjuryType::NONE`).
   d. Do NOT add to `bio.health` in this case.
2. If no `InjuryComponent` or all slots are inactive:
   - Fall back to `bio.health = std::min(100.0f, bio.health + ic.restore_value)` (unchanged behavior).

The same rule applies to both the NPC `update()` path and the `playerPickup()` path.

#### Scenario: MEDICAL clears a laceration
- **WHEN** a citizen with a LACERATION (severity 0.4) consumes a MEDICAL item
- **THEN** the LACERATION severity SHALL decrease to 0.0 and the slot SHALL be cleared

#### Scenario: MEDICAL reduces but does not clear severe injury
- **WHEN** a citizen with INTERNAL_BLEEDING (severity 0.9) consumes a MEDICAL item
- **THEN** severity SHALL become 0.4 and the slot SHALL remain active

#### Scenario: MEDICAL heals health when no injuries present
- **WHEN** a citizen with no active injuries consumes a MEDICAL item (restore_value = 40)
- **THEN** `bio.health` SHALL increase by 40 (clamped to 100), matching pre-existing behavior

#### Scenario: MEDICAL targets worst injury when multiple present
- **WHEN** a citizen has LACERATION (severity 0.3) and INTERNAL_BLEEDING (severity 0.7)
- **THEN** the INTERNAL_BLEEDING slot SHALL be reduced (worst first)
