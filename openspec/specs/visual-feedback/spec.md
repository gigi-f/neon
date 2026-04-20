# visual-feedback Specification

## Purpose
TBD - created by archiving change visual-feedback-polish. Update Purpose after archive.
## Requirements
### Requirement: Bounded Feedback Cues

The game SHALL maintain a bounded recent feedback cue stack for future audio dispatch and deterministic tests.

#### Scenario: Old cues are evicted

- **WHEN** more cues are pushed than the configured cue capacity
- **THEN** the oldest cues SHALL be removed first.

### Requirement: Action Sound Hooks

Player actions SHALL emit named feedback cues that can later be connected to sound playback.

#### Scenario: Successful pickup emits pickup cue

- **WHEN** the player picks up a survival item
- **THEN** a `PICKUP` feedback cue SHALL be recorded.

#### Scenario: Empty quick-use emits denied cue

- **WHEN** the player attempts to consume an empty survival inventory slot
- **THEN** a `DENIED` feedback cue SHALL be recorded.

### Requirement: Warning And Damage Visual Feedback

Warnings and player damage SHALL trigger short visual feedback.

#### Scenario: Warning cue flashes the screen

- **WHEN** a `WARNING` cue is pushed
- **THEN** the feedback state SHALL expose a non-zero flash alpha.

#### Scenario: Hazard damage shakes and flashes

- **WHEN** the player's health decreases during a biology tick
- **THEN** a `HAZARD_DAMAGE` cue SHALL be recorded and visual shake SHALL become active.

### Requirement: Impact Feedback

Blocked player movement SHALL trigger a small impact feedback effect.

#### Scenario: Player movement is blocked

- **WHEN** the player is actively moving and collision prevents expected movement
- **THEN** an `IMPACT` cue SHALL be recorded and visual shake SHALL become active.

### Requirement: HUD Compatibility

Visual feedback SHALL not prevent existing debug, survival, alert, scan, or layer overlay HUD text from rendering.

#### Scenario: Flash overlay is active

- **WHEN** the flash overlay is active
- **THEN** HUD and scan panel rendering SHALL occur after the flash overlay.

