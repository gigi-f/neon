## Overview

Implement a small `FeedbackState` owned by the main loop. Gameplay paths push typed cues into it, and rendering consumes its current visual state. This keeps the first implementation local, deterministic, and independent of a future audio backend.

## Data Model

`FeedbackCue` names the hookable action:
- `PICKUP`
- `CONSUME`
- `SCAN`
- `WARNING`
- `DENIED`
- `IMPACT`
- `HAZARD_DAMAGE`

`FeedbackState` stores:
- a bounded recent cue stack for audio/log hooks
- remaining shake duration and magnitude
- remaining flash duration, color, and alpha

## Event Sources

- Pickup: successful survival item pickup.
- Consume: successful survival inventory use.
- Scan: scan panel hotkeys.
- Denied: empty survival inventory use, or an interact press that finds no item/building/vehicle action.
- Warning: weather and flood alert transitions.
- Impact: blocked player movement while movement input is active.
- Hazard damage: player health drops during the biology tick.

## Rendering

Apply shake as a small deterministic offset to the world camera center before world rendering. Apply flash as a translucent full-screen overlay after the world and before HUD text. The HUD and scan panel should remain legible.

## Non-Goals

- No SDL audio device initialization yet.
- No particle systems.
- No broad refactor of movement, collision, or inventory systems.

## Test Strategy

Add deterministic tests for:
- bounded cue retention
- TTL expiry
- cue-to-effect mapping for shake/flash
- deterministic shake offset decay
