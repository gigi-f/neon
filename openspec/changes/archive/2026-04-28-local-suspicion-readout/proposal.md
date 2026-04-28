# Change: Local Suspicion Readout

## Why

The current local risk slice records worker suspicion and shows a HUD notice, but the state is not yet inspectable after the moment of witnessing. The next session needs suspicion to be readable and bounded through existing `SPACE` inspection surfaces before any de-escalation or persistence work.

## What Changes

- Add compact ordinary inspection lines for local suspicion on the witnessing worker and affected workplace.
- Add Debugger inspection detail for the worker, affected workplace, and affected route/signpost.
- Keep suspicion local, current-scope, and separate from active flow blockage/readout state.
- Preserve no-suspicion readout behavior and volatile last-result HUD behavior.

## Impact

- Affected spec: `local-risk`
- Affected code: `src/world_builder.h`, `tests/world_builder_tests.cpp`, `todo/TODO.md`, `todo/completed_todo.md`
