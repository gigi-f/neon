## Why

The market trade loop now lets the player buy and sell, but prices still only reflect stock scarcity. The TODO calls for trade prices to also respond to reputation and a simple merchant greed margin so market choices become legible economic decisions.

## What Changes

- Extend market pricing with a greed multiplier carried by each market.
- Add player reputation standing that adjusts buy prices and sell payouts.
- Surface the final adjusted trade price in the existing trade preview.
- Keep scarcity as the base signal so current NPC and player market behavior remains predictable.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `district-markets`: Market prices include scarcity, market greed, and player reputation modifiers for player trade previews and confirmations.

## Impact

- Affects `MarketComponent`, player economic state, player trade preview/execution helpers, HUD trade text, world market defaults, and market trade tests.
- No new dependencies or persistence format.
