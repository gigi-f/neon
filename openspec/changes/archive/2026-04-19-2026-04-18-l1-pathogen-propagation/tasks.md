## 1. Spec

- [x] 1.1 Add OpenSpec proposal, design, and pathogen propagation requirements.

## 2. Components

- [x] 2.1 Add `PathogenComponent` to `src/components.h`.

## 3. Systems

- [x] 3.1 Add `PathogenSystem::update()` for exposure and progression.
- [x] 3.2 Seed rare infected citizens in `AmbientSpawnSystem::spawnCitizens()`.
- [x] 3.3 Wire `PathogenSystem` into the L1 gate in `main.cpp`.

## 4. Validation

- [x] 4.1 Build the project.
- [x] 4.2 Run cclsp diagnostics on touched C++ files.
- [x] 4.3 Add deterministic PathogenSystem exposure/progression test scaffolding.

## 5. L2 Cascade + Debug

- [x] 5.1 Add pathogen upward cascade to L2 PAD mood and one-shot illness memory.
- [x] 5.2 Add small debug HUD infected counter.
- [x] 5.3 Add visible infected glyph tint.

## 6. Archive

- [x] 6.1 Add shared-interior deterministic pathogen test coverage.
- [x] 6.2 Validate and archive the OpenSpec change.
