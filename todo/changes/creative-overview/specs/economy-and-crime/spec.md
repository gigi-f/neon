## ADDED Requirements

### Requirement: Credits as Universal Currency
The game economy SHALL operate on a single currency: **Credits**. Credits SHALL be earned through employment, trade, crime, and faction activities. Credits SHALL be spent on goods, services, bribes, and faction dues. An NPC's credit balance SHALL affect their social status, housing options, and desperation level.

#### Scenario: Wealth affects lifestyle
- **WHEN** an NPC accumulates significant credits
- **THEN** their dialogue SHALL reflect wealth ("Credits buy silence, and I have plenty of both. What do you want?"), they SHALL live in better housing, and they SHALL be targeted by criminals

#### Scenario: Poverty drives desperation
- **WHEN** an NPC's credits drop to near-zero with unmet needs
- **THEN** their crime risk SHALL increase, they SHALL seek SQUAT housing, and their dialogue SHALL reflect desperation

### Requirement: Market System with Supply and Demand
Each zone SHALL maintain a local market with supply/demand dynamics for all item categories. Prices SHALL fluctuate based on:
- Local production output
- Consumer demand (driven by population needs)
- Material scarcity
- Crisis conditions (prices spike during emergencies)
- Black market competition

Per-zone economic indicators SHALL include: GDP, unemployment rate, tax revenue, wage index (1.0 = normal), average wealth, crime rate, and material scarcity per raw material type.

#### Scenario: Scarcity drives price spikes
- **WHEN** a biological outbreak increases demand for MEDICAL items while supply remains constant
- **THEN** medical item prices SHALL increase, NPCs SHALL compete for scarce supplies, and the economic stress indicator SHALL rise

#### Scenario: Market crash cascades
- **WHEN** unemployment rises sharply in an industrial zone (factory closures)
- **THEN** local GDP SHALL fall, consumer spending SHALL decrease, businesses SHALL close, and the crisis system SHALL evaluate whether an ECONOMIC_COLLAPSE threshold has been reached

### Requirement: Supply Chain Economy
The economy SHALL feature production chains: raw materials are extracted, processed in factories/workshops, and sold at market. The chain SHALL be:

**Extraction** (mines, salvage zones, labs) → **Production** (factories, workshops, clandestine labs) → **Distribution** (supply chains, deliveries) → **Market** (shops, traders, black market)

Disruption at any point in the chain SHALL cascade — a destroyed factory creates shortages downstream.

#### Scenario: Supply chain disruption
- **WHEN** an industrial zone suffers infrastructure failure (power outage)
- **THEN** production SHALL halt, downstream goods SHALL become scarce, prices SHALL rise in connected commercial zones, and unemployment SHALL spike locally

### Requirement: Crafting and Production
NPCs and the player SHALL be able to craft items at workstations:
- **CraftingBench**: General-purpose item creation
- **Forge**: Metal-working, weapons, tools
- **Laboratory**: Chemical processing, medical supplies, technology

Crafting SHALL require raw materials, time, and access to the appropriate workstation. Recipes SHALL define input materials and output items.

#### Scenario: NPC crafting economy
- **WHEN** an NPC with a crafting job arrives at their workplace (factory with forge)
- **THEN** they SHALL execute PRODUCE_GOODS, consuming raw materials from storage and creating finished items for market sale

### Requirement: Clandestine Drug Manufacturing
The city SHALL feature illegal drug production via clandestine labs (CLANDESTINE_LAB room type). Labs SHALL:
- Store raw chemicals
- Produce contraband drugs over time (production progress 0-1)
- Be subject to raids by law enforcement
- Generate high-value CONTRABAND items

#### Scenario: Lab raid
- **WHEN** law enforcement discovers and raids a clandestine lab
- **THEN** production SHALL halt, stored contraband SHALL be confiscated, and associated NPCs SHALL gain criminal records and increased wanted levels

### Requirement: Taxation System
The city SHALL collect taxes from employed agents based on income. Tax rates SHALL be influenced by the dominant faction's policies. Tax revenue SHALL fund public services (guards, infrastructure maintenance, transit).

#### Scenario: High taxation creates resentment
- **WHEN** tax rates are high and public services are poor
- **THEN** citizen frustration SHALL increase, anti-government sentiment SHALL grow, and the Rebel faction (Entropic Drift) SHALL gain recruitment advantage

### Requirement: Faction Bounties and Contracts
Instead of a traditional stock market, the economy SHALL feature **Faction Bounties and Contracts**. Players and NPCs can "invest" in a faction's growth by completing high-stakes objectives. Successfully completed contracts increase the faction's economic dominance in a zone, leading to:
- Better item availability for allies
- Specialized service unlocks
- Credit rewards and specialized equipment
#### Scenario: Faction dominance via contracts
- **WHEN** the player completes a series of contracts for the Syndicate
- **THEN** the Syndicate's economic power in the zone SHALL increase, black market prices SHALL drop for the player, and exclusive luxury items SHALL become available (e.g., the **Void-Wraith GhostDrone** firmware).

### Requirement: Wanted Level System
...
The player and NPCs SHALL have per-faction wanted levels (0-5 stars) plus a global notoriety baseline. Wanted level SHALL increase when crimes are committed and detected. Higher wanted levels SHALL trigger escalating responses:

| Stars | Response |
|-------|----------|
| 0 | No response |
| 1 | Guards observe suspiciously |
| 2 | Guards actively investigate |
| 3 | Guards pursue and attempt arrest |
| 4 | Faction-wide manhunt, restricted zone access |
| 5 | Shoot-on-sight, maximum enforcement |

#### Scenario: Crime detection increases wanted level
- **WHEN** the player commits a crime (theft, assault, contraband possession) and is observed by a witness or surveillance drone
- **THEN** a CrimeReport SHALL be generated, increasing wanted level with the relevant faction(s)

#### Scenario: Wanted level decay
- **WHEN** the player avoids further criminal activity and stays out of sight
- **THEN** wanted level SHALL gradually decay over time, faster at lower levels

### Requirement: Reputation System
The player SHALL maintain reputation with each faction on a -100 to +100 scale, mapped to tiers:

| Range | Tier | Effect |
|-------|------|--------|
| -100 to -70 | EXCOMMUNICATED | Active hostility, no services, attacked on sight |
| -70 to -30 | HOSTILE | Refused service, guards alerted |
| -30 to -10 | SUSPICIOUS | Higher prices, watched closely |
| -10 to +10 | NEUTRAL | Standard treatment |
| +10 to +30 | FAVORED | Discounts, better information |
| +30 to +70 | FRIENDLY | Access to faction facilities, missions |
| +70 to +100 | ALLY | Full faction benefits, leadership access |

A **fame meter** (0-1) SHALL track how widely known the player's actions are. High fame amplifies reputation effects — both positive and negative.

#### Scenario: Reputation affects trade
- **WHEN** the player has HOSTILE reputation with a faction
- **THEN** merchants aligned with that faction SHALL refuse to trade, and NPCs SHALL warn each other about the player's presence

#### Scenario: Fame amplifies reputation
- **WHEN** the player has high fame and performs a significant action (heroic or criminal)
- **THEN** the reputation change SHALL be amplified — the same act affects standing more because more people know about it

### Requirement: Political Economy (Faction Directives)
The city's political landscape is defined by the struggle for power between the five factions. There are no formal elections; instead, the dominant faction in a zone issues **Directives** that change local simulation parameters.

**Directives**: Policies imposed by the faction with the highest influence in a zone. They affect tax rates, guard aggression, zoning laws, and resource allocation.

**Power Struggle**: Factions compete for influence through territory control, successful operations, and public opinion.

**Public Opinion**: City-wide sentiment toward factions. Influenced by news events, crisis response, and the visibility of faction actions.

**Media Networks**: News outlets with biases that shape public opinion. Faction-aligned media promotes favorable narratives.

#### Scenario: Faction Directive changes city policy
- **WHEN** the Syndicate gains dominant influence in a zone
- **THEN** they SHALL issue a "Laissez-Faire" Directive, shifting tax policy to favor business interests, decreasing enforcement, and increasing black market tolerance

### Requirement: Item Provenance (High-Value Items Only)
To optimize performance, the system SHALL only track provenance (history of ownership) for **Unique, Luxury, and Contraband** items. Generic resources and common goods SHALL NOT track history.

#### Scenario: Luxury item history
- **WHEN** the player finds a luxury item in a slum pawnshop
- **THEN** the item's provenance SHALL reveal its journey — e.g., stolen from a corporate executive, passed through three owners — telling a story of the city's economic flows

### Requirement: Corruption and Backroom Deals
The political system SHALL feature corruption mechanics:

**Bribery**: Exchange credits for favorable actions (overlooking crimes, information).

**Favors**: Non-monetary obligations between NPCs and factions, repaid later — creating webs of mutual obligation.

**Backroom Deals**: Secret agreements between powerful entities with hidden effects on city systems.

**Discovery**: Corruption can be exposed through investigation, generating scandals that damage reputation and destabilize factions.

### Requirement: Directive Markets (Economic Foreknowledge)
The city's political and economic layers SHALL be linked through a **Directive Market** mechanic. Players with high-tier L3 (Economic) and L4 (Political) tools can exploit the delay between a Directive's conception and its enforcement.

1. **Directive Leak**: Using the **L4 Authority Scouter** on a high-tier Faction Terminal SHALL reveal "Projected Directives" — laws scheduled to take effect in 12-24 hours.
2. **Economic Speculation**: The **L3 Crypto-Forensic Lens** SHALL allow the player to "Short-Sell" or "Corner" markets based on this leak.
3. **High-Stakes Arbitrage**: Buying resources (L3) that will become illegal or extremely scarce under an upcoming Directive (L4) allows for massive profit margins once the Directive goes active.

#### Scenario: Short-selling a Crackdown
- **WHEN** the player discovers a projected "Consensus Crackdown" (which will make WEAPONRY contraband in a zone)
- **THEN** they SHALL buy local WEAPONRY items at standard price, wait for the Directive to activate, and sell them at 3x value on the newly established Syndicate black market.

#### Scenario: Corruption and Backroom Deals
- **WHEN** the player bribes a guard to overlook a crime
- **THEN** the guard SHALL stop pursuing, the player's wanted level SHALL not increase from that specific incident, but a FavorComponent SHALL be created — the guard may later demand reciprocation
