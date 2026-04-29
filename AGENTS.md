# Neon Agent Instructions

## Git Workflow

- Work directly on `main` by default for this repository.
- Do not create or switch to feature branches unless the user explicitly asks for a branch.
- When asked to commit completed work, commit it directly to `main` after verifying the workspace status and relevant tests.
- Before committing, confirm the current branch is `main`; if it is not, pause and make the branch situation explicit instead of committing somewhere else.

## Feature Completion Verification

- Gameplay feature work is not complete until the AI playtest harness is current with the feature. If a feature adds or changes verbs, targets, prompts, readouts, map glyphs, scenarios, persistence state, or anything an AI tester needs to observe, update `src/ai_playtest.h`, `tools/ai_playtest.cpp`, and `tests/ai_playtest_tests.cpp` in the same change.
- Gameplay feature verification must include an interactive terminal playtest, not only predetermined unit tests or scripted smoke runs. Use `./build/neon_ai_playtest play default --transcript .neon_feature_playtest.txt` and drive the session by reading the snapshot, choosing the next action, and observing the result. Use `reset suspicion` inside play mode when the feature touches local-risk, suspicion, debugger, or wage/debt behavior.
- Before calling a gameplay feature complete, run `tools/verify_feature_completion.sh --transcript .neon_feature_playtest.txt --check-changed` in addition to the normal build/tests. The configured `.githooks/pre-commit` hook enforces the same gate for staged gameplay/source changes.
