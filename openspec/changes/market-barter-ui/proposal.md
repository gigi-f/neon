## Why

Markets already simulate stock, scarcity pricing, and NPC purchases, but the player cannot deliberately trade with market entities. This blocks the next playable-loop TODO: turning visible L3 economy data into a player decision with a clear cost and consequence.

## What Changes

- Add a focused player market trade interaction for nearby `MarketComponent` entities.
- Let the player buy FOOD, WATER, and MEDICAL units into discrete inventory when they have credits, market stock, and inventory space.
- Let the player sell carried FOOD, WATER, and MEDICAL items back to a nearby market for a reduced price.
- Show a clear pending trade preview before confirmation, including item, buy/sell mode, price, wallet balance, stock, and denial reason when unavailable.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `district-markets`: Adds player-facing buy/sell trade behavior on top of existing stock, price, and NPC market transactions.

## Impact

- Affected code: `src/components.h`, `src/simulation_systems.h`, `src/inventory.h`, `src/main.cpp`, and focused tests.
- No new external dependencies.
- Existing NPC market behavior and pricing formula remain compatible.
