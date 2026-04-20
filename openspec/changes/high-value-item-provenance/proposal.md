## Why

High-value and risky goods currently only expose compact flags. The next TODO asks for lightweight ownership/source provenance so stolen, unique, quest, and high-value items can carry consequences without making every commodity expensive to track.

## What Changes

- Add an item provenance record for tracked items only.
- Preserve provenance through pickup, inventory inspection, dropping, and market purchase/sale paths.
- Treat picking up another owner's tracked item as stolen.
- Expose provenance details through Surface Scan and Financial Forensics.
- Mark the high-value provenance TODO complete after validation.

## Capabilities

### New Capabilities

- `item-provenance`: Tracks ownership/source metadata for stolen, unique, quest, faction-relevant, and high-value goods.

### Modified Capabilities

- `discrete-inventory`: Inventory entries preserve and inspect item provenance when present.
- `item-flag-visibility`: Scan panels display provenance details for tracked item entities.
- `district-markets`: Player market purchase/sale helpers preserve provenance for tracked items.

## Impact

- Affects `src/components.h`, `src/inventory.h`, `src/main.cpp`, `src/simulation_systems.h`, inventory/market tests, OpenSpec specs, `todo/TODO.md`, and the user manual.
- No new external dependencies or persistence format.
