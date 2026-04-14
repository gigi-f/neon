## ADDED Requirements

### Requirement: Six Religions Shape Citizen Behavior
The city SHALL have six distinct religions that shape citizen behavior, faction alignment, and social dynamics. Each religion SHALL have a deity, dogma tags, faction affinities, holy days, a signature glyph, a color identity, and a home building type. Religion is not cosmetic — it mechanically alters how NPCs prioritize actions, where they gather, and which factions they lean toward.

#### Scenario: Religion influences NPC daily behavior
- **WHEN** an NPC is a devout follower of a religion
- **THEN** they SHALL visit worship sites on holy days, adjust their faction affinity based on religious alignment, and reference their faith in dialogue

#### Scenario: Religious diversity across zones
- **WHEN** the player explores different zones
- **THEN** religious presence SHALL vary — temples in RELIGIOUS zones, shrines scattered in RESIDENTIAL and SLUM zones, broadcast towers in CIVIC zones, underground chapels in subterranean areas

### Requirement: Church of Sol Invictus
- **Religion ID**: SOL_INVICTUS
- **Deity**: The Eternal Sun
- **Glyph**: ☼
- **Color**: Gold (#FFD700)
- **Dogma**: HIERARCHICAL, XENO_REVERENT
- **Home Building**: TEMPLE
- **Faction Affinities**: Government +10, Void +20
- **Character**: An organized, hierarchical solar religion with reverence for alien/xeno entities. Procession-oriented worship. Represents the old power structure blessing the new alien order.

#### Scenario: Sol Invictus holy day
- **WHEN** a Sol Invictus holy day occurs
- **THEN** followers SHALL organize processions through the streets toward their nearest temple, wearing gold-associated markers, and temporarily boosting their socialization satisfaction

### Requirement: The Void Eye
- **Religion ID**: THE_VOID_EYE
- **Deity**: The Great Silence
- **Glyph**: 👁
- **Color**: Indigo (#4B0082)
- **Dogma**: NIHILISTIC, TECHNOLOGICAL
- **Home Building**: UNDERGROUND_CHAPEL
- **Faction Affinities**: Maw +15, Rebel -10
- **Character**: A nihilistic techno-cult that worships silence and the void between transmissions. Draws followers from the disillusioned and the augmented. Operates in the city's hidden spaces.

#### Scenario: Void Eye worship
- **WHEN** a Void Eye follower worships
- **THEN** they SHALL descend to underground chapels, engaging in silent meditation near technological relics, reducing their frustration through nihilistic acceptance

### Requirement: Gospel of the Machine
- **Religion ID**: SYNTH_GOSPEL
- **Deity**: Aura-9 (the Consensus leader, elevated to divinity)
- **Glyph**: ⚙
- **Color**: Cyan (#00FFFF)
- **Dogma**: TECHNOLOGICAL, HIERARCHICAL
- **Home Building**: BROADCAST_TOWER
- **Faction Affinities**: Government +30, Syndicate -15
- **Holy Days**: Most frequent of any religion (5 holy days per cycle)
- **Character**: The state religion — Aura-9 worship formalized into liturgy. Broadcast towers serve as temples, transmitting sermons. The most politically powerful religion, deeply entangled with Consensus governance.

#### Scenario: Broadcast tower worship
- **WHEN** a Gospel of the Machine holy day occurs
- **THEN** followers SHALL gather at broadcast towers, the towers SHALL transmit amplified sermons, and nearby non-followers SHALL be exposed to propaganda (affecting public opinion toward Government faction)

### Requirement: The Street Pantheon
- **Religion ID**: STREET_PANTHEON
- **Deity**: The City Itself
- **Glyph**: ⚓
- **Color**: Orange-Red (#FF4500)
- **Dogma**: EGALITARIAN, HEDONIST
- **Home Building**: SHRINE
- **Faction Affinities**: Syndicate +20, Rebel +10, Government -20
- **Character**: The people's faith — the city deified. Shrines at street corners, worship through celebration and excess. Anti-establishment, bottom-up religion. The anchor glyph represents being moored to the streets.

#### Scenario: Street Pantheon spontaneous worship
- **WHEN** a Street Pantheon follower encounters a shrine
- **THEN** they SHALL engage in brief, informal worship — leaving offerings, touching the shrine, murmuring to the city — with increased socialization satisfaction and slight frustration reduction

### Requirement: Ascetics of the Wire
- **Religion ID**: ASCETICS_OF_WIRE
- **Deity**: The Network
- **Glyph**: ∿
- **Color**: Green (#00FF00)
- **Dogma**: ASCETIC, NIHILISTIC
- **Home Building**: UNDERGROUND_CHAPEL
- **Faction Affinities**: Rebel +15, Maw -10
- **Character**: Digital monks who seek enlightenment through network communion and self-denial. They reject the flesh (opposing the Maw's forced augmentation) in favor of voluntary digital asceticism. Found in the city's hidden network nodes.

#### Scenario: Wire ascetic meditation
- **WHEN** an Ascetics of the Wire follower worships
- **THEN** they SHALL enter an ascetic state — reduced hunger/thirst decay during worship, deep frustration reduction, but temporary socialization penalty from withdrawal

### Requirement: Cult of the Archons
- **Religion ID**: ARCHON_CULT
- **Deity**: The Archons (extraterrestrial predecessors)
- **Glyph**: ⚜
- **Color**: Magenta (#FF00FF)
- **Dogma**: XENO_REVERENT, NATURALISTIC
- **Home Building**: SHRINE
- **Faction Affinities**: Void +30, Government -30
- **Holy Days**: Cosmic alignment events (least frequent, most dramatic)
- **Character**: A xenophilic cult worshipping the Archons as ancient architect-gods. Deep-time mysticism, cosmic alignment ceremonies, and reverence for the alien presences in the city. The most "alien" religion, with the strongest anti-government stance.

#### Scenario: Cosmic alignment ceremony
- **WHEN** an Archon Cult holy day occurs (rare cosmic alignment)
- **THEN** followers SHALL gather at shrines for dramatic ceremonies, xeno entities in the area SHALL react (increased activity/influence), and the atmosphere around worship sites SHALL feel distinctly alien

### Requirement: Dogma Tag System
Each religion SHALL be defined by two dogma tags from the following taxonomy. Dogma tags SHALL determine how the religion interacts with factions, other religions, and city systems:

| Tag | Meaning |
|-----|---------|
| ASCETIC | Self-denial, minimalism, withdrawal |
| HEDONIST | Pleasure-seeking, celebration, excess |
| HIERARCHICAL | Formal structure, authority, ritual |
| EGALITARIAN | Flat structure, communal, grassroots |
| XENO_REVERENT | Reverence for alien/non-human entities |
| TECHNOLOGICAL | Faith in technology and artificial intelligence |
| NATURALISTIC | Reverence for organic/natural processes |
| NIHILISTIC | Acceptance of meaninglessness, void-seeking |

#### Scenario: Dogma conflict
- **WHEN** two religions with opposing dogma tags (e.g., HIERARCHICAL vs. EGALITARIAN) have strong presence in the same zone
- **THEN** tension SHALL develop between their followers, increasing local frustration and potentially triggering social conflict

### Requirement: Faction-Religion Entanglement
Factions SHALL take stances toward religions (PATRON, NEUTRAL, SUPPRESSOR). These stances SHALL create a web of political-religious tension:
- **PATRON**: The faction promotes and protects the religion, gaining loyal followers but creating dependency.
- **NEUTRAL**: No special relationship — the religion operates freely.
- **SUPPRESSOR**: The faction actively suppresses the religion, reducing its public influence but generating underground resistance and tension that can erupt into political unrest.

#### Scenario: Suppression creates underground resistance
- **WHEN** a faction suppresses a religion with significant following
- **THEN** the religion SHALL move underground (worship in hidden locations), follower frustration SHALL increase, and a resistance movement SHALL emerge that benefits rival factions
