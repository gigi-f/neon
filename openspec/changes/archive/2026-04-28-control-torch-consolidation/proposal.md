# Change: Control Torch Consolidation

## Why

The ordinary `SPACE` inspection and inherited debugger scan now overlap enough that they feel like two versions of the same action. The player should have one inspect/read action and one infrastructure-interference action.

## What Changes

- Make `SPACE` perform Debugger inspection while preserving the existing ordinary target/readout detail.
- Make plain `G` trigger the existing signpost/dependency interference action that previously required `Shift+G`.
- Present the infrastructure-messing action as the `INTERFERENCE TORCH`.
- Remove player-facing `G SCAN` and `Shift+G` prompts from active controls.

## Impact

- Affected specs: `route-purpose-readouts`, `torch-controls`
- Affected code: `src/main.cpp`, `src/world_builder.h`, `tests/world_builder_tests.cpp`, `todo/TODO.md`
