## Why

The current equipment defaults put consumables on `1-3` and scan tools on `4-8`, but the TODO now asks for scan tools to occupy the primary tool band and carried items to occupy the later quick slots. This keeps the five layer-reading tools together and leaves `6-9` for consumables and future inventory items.

## What Changes

- Move default scan-tool hotkeys to `1-5`.
- Move current consumable quick-use defaults to `6-8`, leaving `9` available for a future item binding.
- Keep `0` permanently assigned to no equipped slot.
- Constrain runtime hotkey rebinding so scan tools can only bind to `1-5`, item/consumable slots can only bind to `6-9`, and invalid assignments leave the existing binding unchanged.
- Update tests, user-facing control docs, and the TODO state.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `equipped-tools`: Numeric hotkeys use fixed tool and item ranges.
- `hotkey-rebinding`: Rebinding respects the same range rules as the default layout.

## Impact

- Affected code: `src/equipment.h`, `src/main.cpp`, `tests/equipment_tests.cpp`, `docs/USER_MANUAL.md`, and `todo/TODO.md`.
- No new external dependencies.
