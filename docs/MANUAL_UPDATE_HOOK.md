# User Manual Update Hook

Feature additions must keep `docs/USER_MANUAL.md` current. This repository includes a tracked Git pre-commit hook that blocks feature-facing commits unless the user manual is staged with the same commit.

## Install

Run:

```sh
scripts/install-git-hooks.sh
```

This sets the repository-local Git `core.hooksPath` to `.githooks`.

## What The Hook Checks

The hook runs `scripts/check-user-manual-updated.sh --staged`.

It treats changes under these paths as feature-facing:

- `src/`
- `tests/`
- `openspec/specs/`
- `openspec/changes/`
- `todo/TODO.md`
- `assets/ART_DIRECTION.md`

When any of those paths are staged, `docs/USER_MANUAL.md` must also be staged. The hook ignores documentation-only changes and hook-maintenance changes.

## Override

For commits that intentionally do not affect player-facing behavior, run:

```sh
SKIP_USER_MANUAL_CHECK=1 git commit
```

Use the override for mechanical changes only, such as formatting, dead-code deletion, test-only refactors with no visible behavior change, or hook maintenance.

## Manual Check

To run the same check manually against staged files:

```sh
scripts/check-user-manual-updated.sh --staged
```

To check the current working tree:

```sh
scripts/check-user-manual-updated.sh --working-tree
```
