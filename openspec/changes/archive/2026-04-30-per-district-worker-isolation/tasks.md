## 1. Worker Placement

- [x] 1.1 Spawn configured workers on local district labor routes before using fallback paths.
- [x] 1.2 Keep fixed workers on paths within their district as they move.

## 2. District Tags And Readouts

- [x] 2.1 Add district tags to multi-district worker, route, purpose, wage, local notice, and local witness readouts.
- [x] 2.2 Preserve existing one-district readout strings for current baseline tests.
- [x] 2.3 Keep AI playtest warps district-local for buildings and workers.

## 3. Isolation Boundaries

- [x] 3.1 Keep district A local suspicion from affecting district B worker readouts, wage spoof prompts, clinic ledger flags, or audit traces.
- [x] 3.2 Scope dependency disruption state and dependency scan/readouts to the affected district.
- [x] 3.3 Round-trip both district workers and district-local worker records through tiny save/load.

## 4. Verification

- [x] 4.1 Add deterministic world-builder and AI playtest coverage for per-district placement, labels, suspicion, wage, clinic, dependency, and save/load behavior.
- [x] 4.2 Run build, CTest, OpenSpec validation, cclsp diagnostics, feature-completion verification, and an adaptive terminal playtest.
