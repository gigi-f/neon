## ADDED Requirements

### Requirement: Hybrid Dialogue Architecture
The game SHALL use a hybrid dialogue system combining two complementary approaches:

1. **Procedural Grammar Engine**: Rule-based text generation that produces contextually appropriate dialogue from templates, grammar rules, and NPC state variables. This handles the majority of NPC speech — greetings, complaints, observations, rumors, and ambient conversation.

2. **Ink-Based Authored Paths ("Gold Paths")**: Hand-written branching dialogue trees for significant NPCs (faction leaders, quest-givers, key story moments) using the Ink scripting language. These provide narrative depth and player choice at critical junctures.

The two systems SHALL coexist — most dialogue is procedural (cheap, scalable, context-aware), while key moments are authored (rich, branching, meaningful).

#### Scenario: Ambient procedural dialogue
- **WHEN** the player walks through a populated area
- **THEN** nearby NPCs SHALL be engaged in procedurally generated conversations reflecting their current state — complaining about weather, discussing faction politics, haggling over prices, sharing rumors

#### Scenario: Gold Path encounter
- **WHEN** the player initiates dialogue with a faction leader or key NPC
- **THEN** the system SHALL load an Ink-scripted branching conversation with meaningful choices that affect faction standing, reputation, and story progression

### Requirement: Dialogue Varies by NPC State
Procedurally generated dialogue SHALL vary based on:
- **Social Status**: Wealthy NPCs speak differently from impoverished ones ("Credits buy silence, and I have plenty of both" vs. "My stomach hasn't felt full in days")
- **Faction Affiliation**: Government NPCs use clinical corporate speech; Rebels use revolutionary rhetoric; Void-Walkers use mystical alien cadence
- **Work Conditions**: Employed vs. unemployed, shift length, pay level
- **Relationship to Player**: Stranger, acquaintance, friend, enemy
- **Personality Archetype**: Paranoid, verbose, laconic, optimistic, nihilistic
- **Needs State**: Desperate hunger alters tone, high frustration makes speech hostile, low socialization makes NPCs eager to talk
- **Current Activity**: Working, commuting, worshipping, fleeing

#### Scenario: Faction-colored dialogue
- **WHEN** the player speaks with a Consensus follower during work hours
- **THEN** their dialogue SHALL use corporate language, reference schedules and efficiency, and express satisfaction with order — or frustration if their routine was disrupted

#### Scenario: Desperation changes tone
- **WHEN** the player speaks with an NPC whose hunger is critically low
- **THEN** their dialogue SHALL be dominated by need — shorter sentences, references to food scarcity, willingness to beg, barter, or threaten for food

### Requirement: Six Speech Profiles
Every speaking entity SHALL be assigned one of six speech profiles that define their vocabulary, sentence structure, and verbal mannerisms:

| Profile | Character | Used By |
|---------|----------|------|
| CORPORATE | Formal, bureaucratic, optimization-focused | Consensus/Government followers, executives |
| STREET/SLANG | Informal, revolutionary, street-smart | Rebel followers, slum residents, criminals |
| ABORRAX | Alien, clinical, post-biological | Silicon Maw followers, augmented beings |
| ARCHON | Mystical, precise, cosmic references | Void-Walker followers, xeno-cultists |
| SYNDICATE | Transactional, calculating, deal-oriented | Syndicate followers, merchants, fixers |
| STANDARD | Neutral, everyday urban speech | Unaligned citizens, default |


#### Scenario: Speech profile identification
- **WHEN** the player overhears two NPCs speaking
- **THEN** their speech profiles SHALL be identifiable — a CORPORATE speaker's formality contrasts clearly with a STREET speaker's slang, immediately conveying faction alignment

### Requirement: Dialogue Topic Categories
Conversations SHALL cover the following topic categories, selected based on NPC state and context:
- **Greeting**: Initial contact, varies by relationship tier
- **Work**: Employment conditions, wages, shift complaints
- **Family**: Home life, relationships, shared housing
- **Lore**: City history, the Great Erasure, urban legends
- **Rumor**: Unverified information about events, people, factions
- **Religion**: Faith, doctrine, holy days, worship
- **Buy Info**: Intelligence for sale — price tips, faction secrets, location intel
- **Faction Doctrine**: Proselytizing, recruitment, philosophical arguments
- **Needs Complaints**: Hunger, thirst, frustration, loneliness
- **Weather/Environment**: Reactions to acid rain, smog, temperature
- **Threats**: Intimidation, warnings, hostile demands
- **Solidarity**: Mutual support, community, shared struggle
- **Negotiation**: Trade offers, barter, deal-making

#### Scenario: Topic selection based on context
- **WHEN** two NPCs who are strangers meet in a commercial zone
- **THEN** their conversation topics SHALL be drawn from Greeting, Work, Weather, and potentially Rumor — not intimate topics like Family

### Requirement: Information System
The city SHALL feature an information economy where knowledge flows through NPC social networks. Information SHALL be typed:

| Type | Character |
|------|-----------|
| RUMOR | Unverified social knowledge — may be true or false |
| PROPAGANDA | Faction-crafted messaging designed to influence opinion |
| INTELLIGENCE | Verified factual information — locations, events, capabilities |
| PRICE_TIP | Market information — where goods are cheap/expensive |
| GOSSIP | Personal information about specific NPCs — relationships, activities |

Information SHALL spread organically through conversations. An NPC who learns a RUMOR SHALL share it with their social network, degrading in accuracy with each retelling.

#### Scenario: Rumor propagation
- **WHEN** an NPC witnesses an unusual event (xeno sighting, faction conflict)
- **THEN** they SHALL generate a RUMOR and share it in subsequent conversations, with the rumor spreading outward through social connections and becoming less accurate over time

#### Scenario: Intelligence has value
- **WHEN** the player seeks INTELLIGENCE about a faction's activities
- **THEN** they SHALL need to find NPCs willing to sell or share it, with the quality and accuracy of information varying by the source's knowledge and relationship

### Requirement: Audibility & Eavesdropping Model
Hearing and analyzing conversations SHALL be a core investigative pillar of Neon Oubliette. NPCs SHALL engage in "real" dialogue related to their internal state (needs, faction directives, current task) or social links. The player SHALL be able to overhear these conversations based on proximity, environmental barriers, and their **Hearing Augmentation Level**.

#### Visual Presentation: The "Glitch-Text" HUD
Dialogue SHALL be rendered as floating text above the speaker's head. The clarity of the text SHALL be determined by the **Audibility Level**. Incomplete clarity SHALL be represented by missing letters replaced by dashes (`-`), simulating distance or muffling.

| Audibility Level | Condition | Presentation Style |
|-----------------|-----------|--------------------|
| **CLEAR** | Direct address or < 3 WU distance. | "The delivery is late. I'm worried." |
| **OVERHEARD** | 3-8 WU distance, no walls. | "T-- d-l-v-ry -s l-te. I'm w-rr--d." |
| **MUFFLED** | 1 wall between OR 8-15 WU distance. | "T-- --l-v--y -s ----. -'m w----ed." |
| **INAUDIBLE** | 2+ walls between OR > 15 WU distance. | `[...]` |
#### Hearing Augmentation Tiers (Neural-Audio Implant)
The player SHALL be able to upgrade their "Neural-Audio Implant" via firmware modules or ripper-doc services to improve eavesdropping performance. 

- **Tier 1 (Baseline)**: Standard audibility ranges. **FREE (Passive)**. 
- **Tier 2 (Enhanced)**: Decreases "Missing Letter" % by 50%. Extends audible range by +5 WU. **Neural Cost: 1% per 5 seconds**.
- **Tier 3 (Master)**: Allows "Filtering" (L2 Cognitive link). Clear audio through 1 wall. **Neural Cost: 3% per 5 seconds**.

#### Requirement: Neural-Audio Recorder (Investigative Device)
The player SHALL be able to acquire a **Neural-Audio Recorder** (Illicit Tier 2 Hardware). This device allows for the capture and subsequent analysis of distorted or muffled dialogue.

1. **Capture**: When the player is within range of **MUFFLED** dialogue, they can trigger **RECORD**.
2. **The "Fragmented File"**: The recorder saves a digital file containing the "Glitch-Text" (e.g., `T-- --l-v--y -s ----`).
3. **Decryption/Analysis**:
   - **L2 Resonate Tool**: Using the Master-tier L2 tool on a file can "resolve" up to 40% of the missing letters.
   - **NPC Fixers**: The player can pay a **Syndicate Fixer** to fully decrypt a recording, revealing hidden **INTELLIGENCE** or **LORE** that was impossible to hear in real-time.

#### Scenario: Recording a clandestine meeting
- **WHEN** the player overhears a high-tier conversation through a wall but lacks Tier 3 Hearing.
- **THEN** they SHALL activate the **Neural-Audio Recorder**, capturing the muffled fragment. Later, they pay a fixer to resolve the text, discovering the location of an AGI Core Node.

#### Scenario: Eavesdropping for Secrets & Lore
...
- **WHEN** the player successfully overhears a conversation in a **PRIVATE_SPACE** (Bedroom, Supervisor Office, Clandestine Lab)
- **THEN** the dialogue SHALL have a higher probability of containing **INTELLIGENCE**, **PRICE_TIPS**, or **LORE** fragments for free.
- **Cognitive Reward**: Overhearing high-tier secrets SHALL automatically update the player's **L2 Cognitive** information database, mirroring the "Mind Map" functionality of the Resonator tool.

#### Scenario: The "Missing Letters" reveal
- **WHEN** the player moves closer to a speaker or activates a high-tier Hearing Augment
- **THEN** the dashes (`-`) in the floating text SHALL "resolve" into letters in real-time, simulating the neural implant clarifying the signal.


### Requirement: Graffiti as Environmental Narrative
The city SHALL feature graffiti as a form of environmental storytelling tied to faction territory control. Graffiti types:

| Type | Glyphs | Purpose |
|------|--------|---------|
| TAG | *, ~ | Quick identity marks — "I was here" |
| TERRITORY | #, $, ^ | Faction claims — "This zone belongs to us" |
| MURAL | &, @ | Artistic expression — large, meaningful |
| WARNING | !, ? | Practical — danger zones, safe houses |
| SLUR | %, X | Hostile — faction rivalries, ethnic tension |

Graffiti SHALL be colored according to the faction that created it (Government: blue, Rebel: red, Maw: green, Void: cyan, Syndicate: red/gold). Graffiti density and type SHALL indicate which faction controls or contests a zone.

#### Scenario: Graffiti as faction intelligence
- **WHEN** the player enters a zone with heavy TERRITORY graffiti from two different factions
- **THEN** they SHALL understand the zone is contested — expect higher tension, more guard patrols, and potential faction conflict

### Requirement: World Myths and Flavor Text
The game SHALL embed a layer of urban mythology — stories that citizens tell each other, half-believed truths that give the city depth and mystery:

- "The Oubliette wasn't built; it was grown from a single seed of pure data"
- "There's a level below the sewers where the city's original heart still beats"
- "The AGI leaders were once human. Hard to believe looking at Aura-9"
- "The 'Great Erasure' wasn't an accident. They wanted us to forget what came before"
- "They say if you walk the same three blocks for ten years, the city lets you leave"

These myths SHALL appear in NPC dialogue (Lore topic), graffiti murals, faction propaganda, and environmental storytelling. Some MAY have kernels of truth discoverable through deep exploration.

#### Scenario: Myth in dialogue
- **WHEN** the player asks an NPC about city lore
- **THEN** the NPC SHALL share one or more myths, filtered through their faction affiliation and personality — a Consensus follower dismisses them as superstition, a Void-Walker treats them as sacred prophecy

### Requirement: Flavor Text Samples
The procedural grammar engine SHALL produce dialogue that captures the following tonal range (representative samples):

**Morning**: "The Oubliette is waking up. Time to earn some credits."
**Anxiety**: "Another 12-hour shift... I don't know if I can take it."
**Revolutionary**: "The Consensus is a cage. We must reclaim our labor!"
**Environmental**: "Acid rain. It eats through the concrete. And us."
**Need**: "The Supply of food is scarce here. My stomach hasn't felt full in days."
**Wealth**: "Credits buy silence, and I have plenty of both. What do you want?"
**Community**: "The Oubliette is a beast, but we're a pack. Don't forget that."
**Alien**: "[UNINTELLIGIBLE CHITTERING / FEEDBACK]"
**Mystical**: "The stars whisper of your arrival, traveler. Why seek the hollow truth?"

#### Scenario: Tone matches world state
- **WHEN** the city is experiencing an economic crisis
- **THEN** ambient dialogue SHALL shift toward anxiety, desperation, and anger — the grammar engine SHALL weight crisis-related templates higher
