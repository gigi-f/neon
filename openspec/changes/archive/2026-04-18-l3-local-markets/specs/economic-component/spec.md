## MODIFIED Requirements

### Requirement: DesireType enumeration
`DesireType` in `components.h` SHALL include two market-specific variants in addition to the existing values:
- `BUY_FOOD` — citizen wants to purchase food at a market (requires credits >= market price)
- `BUY_WATER` — citizen wants to purchase water at a market (requires credits >= market price)

The full enumeration SHALL be:
`enum class DesireType { NONE, SATISFY_HUNGER, SATISFY_THIRST, FIND_TRANSIT, BUY_FOOD, BUY_WATER };`

#### Scenario: BUY_FOOD scored higher than SATISFY_HUNGER when citizen can afford market
- **WHEN** citizen.hunger < 70 AND citizen.credits >= nearest FOOD market current_price
- **THEN** GDISystem SHALL score BUY_FOOD with urgency equal to SATISFY_HUNGER urgency + 0.1f (market preference bonus), making BUY_FOOD the winning desire

#### Scenario: SATISFY_HUNGER remains valid when no affordable market exists
- **WHEN** citizen.hunger < 70 AND no FOOD market has current_price <= citizen.credits
- **THEN** GDISystem SHALL score SATISFY_HUNGER normally and search free ItemComponent pickups

#### Scenario: BUY_WATER scored higher than SATISFY_THIRST when citizen can afford market
- **WHEN** citizen.thirst < 70 AND citizen.credits >= nearest WATER market current_price
- **THEN** GDISystem SHALL score BUY_WATER with urgency equal to SATISFY_THIRST urgency + 0.1f

#### Scenario: SATISFY_THIRST remains valid when no affordable market exists
- **WHEN** citizen.thirst < 70 AND no WATER market has current_price <= citizen.credits
- **THEN** GDISystem SHALL score SATISFY_THIRST normally and search free ItemComponent pickups
