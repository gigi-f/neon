## 1. District And Station Layout

- [x] 1.1 Allow configured two-macro authored sandbox layouts without changing the one-district baseline.
- [x] 1.2 Add paired station entities with district IDs, stable IDs, transit link IDs, transforms, glyphs, and inspection readouts.
- [x] 1.3 Keep each district's housing, workplace, supply, market, clinic, signposts, and paths self-contained.

## 2. Transit Ride Interaction

- [x] 2.1 Let `E` near a station board a transit interior instead of instantly teleporting.
- [x] 2.2 Let the player move inside the transit interior while ride time advances.
- [x] 2.3 Let `E` while doors are closed look out the window and exit at the destination platform.
- [x] 2.4 Let waiting advance the ride until doors open, then let `E` exit normally.

## 3. Persistence, AI Playtest, And Verification

- [x] 3.1 Save/load active transit ride origin, destination, elapsed time, doors state, exterior position, and interior position.
- [x] 3.2 Expose station targets, transit interior state, current district, and prompts through AI playtest snapshots.
- [x] 3.3 Cover one-district/no-transit baseline, two-district stations, window exit, wait-for-doors exit, carried-object travel, and tiny save/load.
- [x] 3.4 Run build, CTest, OpenSpec validation, interactive AI playtest, and feature-completion verification.
