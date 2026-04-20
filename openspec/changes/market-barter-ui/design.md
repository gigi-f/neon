## Context

`MarketSystem` already owns stock restocking, scarcity pricing, and NPC purchases against nearby market buildings. Player inventory is now discrete, player wallets use `EconomicComponent`, and Financial Forensics already exposes market stock. The missing piece is a direct player interaction that uses those same systems.

## Goals / Non-Goals

**Goals:**

- Reuse the current `MarketComponent` pricing formula for player buy/sell previews.
- Keep player trade deterministic and testable through small helpers outside the SDL loop.
- Prefer nearby market entities and keep inventory capacity, wallet balance, and stock as explicit denial states.
- Provide enough HUD text for confirmation before purchase or sale.

**Non-Goals:**

- No full shop menu, multi-quantity cart, reputation modifier, or greed margin tuning beyond an explicit sell discount.
- No provenance tracking for high-value goods; that remains the next TODO after basic trade.
- No new dependency or persistence format.

## Decisions

- Add player trade helpers beside `MarketSystem` in `simulation_systems.h`.
  - Rationale: price and stock rules already live there, and tests can include the header directly.
  - Alternative considered: put everything in `main.cpp`; rejected because SDL event code is hard to unit test.

- Use a single pending trade preview with mode, item type, market entity, price, stock, wallet, and denial reason.
  - Rationale: the TODO asks to expose the result before confirmation, so selection and execution must be separate.
  - Alternative considered: one-key immediate buy; rejected because it hides price and inventory-space failures until after input.

- Keyboard mapping stays narrow: `B` cycles buy offers, `N` cycles sell offers, `Y` confirms, `Escape` clears.
  - Rationale: avoids introducing a full modal UI while making the transaction preview explicit.
  - Alternative considered: mouse/shop menu; rejected as larger than the current renderer/HUD style.

- Sell value is a constant 50% of current market effective price.
  - Rationale: enough to distinguish buy from sell and avoid profitable same-tick arbitrage.
  - Alternative considered: reputation/greed margin now; deferred to a later TODO that includes reputation.

## Risks / Trade-offs

- Key collisions with future systems -> Keep the mapping local and easy to replace with a later menu.
- Player can buy only one unit at a time -> This is intentional for the first playable loop and avoids cart complexity.
- Market stock is fractional due to restock -> Execution treats stock `>= 1.0f` as enough for a player unit so HUD and inventory remain discrete.
