## Context

`MarketComponent` already tracks stock, max stock, base price, and restock rate. `effectiveMarketPrice` computes scarcity-adjusted base price and is reused by NPC purchases and player trade. Player buy/sell flow now uses `PlayerMarketTradePreview` to separate selection from confirmation.

## Goals / Non-Goals

**Goals:**

- Preserve the existing scarcity formula as the base price.
- Add a per-market greed margin that raises buy prices and lowers sell payouts.
- Add a player-carried reputation value that lowers buy prices and improves sell payouts when positive, with the inverse when negative.
- Include enough detail in previews and tests to prove the adjusted price is the confirmed price.

**Non-Goals:**

- No faction-specific reputation tables yet.
- No haggling UI, multi-quantity cart, or merchant personality system.
- No changes to NPC purchase pricing beyond market greed, because NPC reputation is not modeled.

## Decisions

- Add `MarketComponent::greed_margin` as a small signed/positive multiplier input.
  - Rationale: market entities already own pricing data, and a single field keeps tuning visible in world generation and tests.

- Add `EconomicComponent::market_reputation` in the range `[-1.0f, 1.0f]`.
  - Rationale: this is the smallest player/NPC-compatible place to store broad market standing until faction reputation exists.

- Introduce helper functions for buy price and sell payout.
  - Rationale: confirmations can recompute current values and tests can assert the same helpers used by the HUD.

- Clamp final buy prices and sell payouts to non-negative values.
  - Rationale: extreme tuning should never pay the player for buying or charge the player for selling.

## Risks / Trade-offs

- Broad market reputation is less expressive than faction standing, but it satisfies the current trade loop without forcing the later faction system into this change.
- Greed affects NPC purchases if the existing `MarketSystem` price path is extended. That is acceptable because greed belongs to the market, while reputation remains player-specific.
