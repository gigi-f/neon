# Change: Witnessed Interruption

## Why

Player interference already changes local flow, but it has no social consequence. The next session needs one nearby worker/system to notice concrete interference before any city-wide surveillance, wanted level, or faction reaction exists.

## What Changes

- Record a current-scope local suspicion event when a worker witnesses the player taking expected workplace output.
- Record a current-scope local suspicion event when a worker near an affected route sees the player spoof a signpost with the Interference Torch.
- Surface the event through a small HUD/status readout so the player knows it was noticed.
- Preserve existing unwitnessed pickup, route restore, and non-target behavior.

## Impact

- Affected spec: `local-risk`
- Affected code: `src/components.h`, `src/world_builder.h`, `src/main.cpp`, `tests/world_builder_tests.cpp`, `todo/TODO.md`, `todo/completed_todo.md`
