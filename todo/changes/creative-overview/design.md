## Context

Neon Oubliette is a procedurally-generated mega-city simulation set in a cyberpunk dystopia controlled by five competing post-human intelligences. The original game was built as a Notcurses terminal application in C++20 with EnTT ECS. It proved out a deeply ambitious creative vision: five causally-linked simulation layers, 76 interconnected systems, procedural dialogue via Ink scripting, and a living economy with political simulation.

The game is now being rebuilt as an SDL2 windowed application. The developer is satisfied with the entity systems and creative design but needs to rethink major technical systems. This creative overview serves as the canonical reference document — the game's creative bible — ensuring nothing is lost in translation.

**The title**: "Neon" (cyberpunk, artificial hope, the glow of surveillance) + "Oubliette" (French for a dungeon with only a trapdoor above — a place of deliberate forgetting). The city is a neon-lit prison that society has chosen to forget.

## Goals / Non-Goals

**Goals:**
- Capture the complete creative identity of Neon Oubliette in a form that is independent of any rendering backend or engine architecture.
- Preserve the entity system vocabulary faithfully — all agent tasks, item categories, creature types, faction definitions, and religion data as validated by the developer.
- Document the five-layer simulation cascade as a *narrative design tool*, not a technical specification.
- Establish the tone, aesthetic, and design philosophy that all future implementation must honor.
- Create a reference that a new developer could read to understand what the game *is* without ever seeing the original codebase.

**Non-Goals:**
- Specifying C++/SDL2 implementation details, ECS architecture, or system scheduler design.
- Defining rendering approaches, shader pipelines, or asset formats.
- Prescribing specific data structures, serialization formats, or network protocols.
- Locking down tick rates, update frequencies, or performance budgets.
- Redesigning any creative systems — this document captures what exists, not what might change.

## Decisions

### 1. Creative Document, Not Technical Spec
This overview deliberately avoids implementation language. Where the original codebase has `BiologyComponent` with specific fields, this document describes "what a living body means in the Oubliette" — the creative intent behind the data. The successor team should derive their own technical designs from these creative requirements.

**Rationale**: The developer is rethinking major technical systems. Preserving the *what* and *why* while leaving the *how* open prevents the new architecture from being constrained by legacy decisions.

### 2. Entity System Preserved As-Is
The agent task vocabulary (40+ tasks), item categories (10 market types, 5 raw materials), creature taxonomy (4 xeno types, 7 life stages, 7 species), and faction/religion definitions are documented verbatim from the validated original.

**Rationale**: The developer explicitly stated satisfaction with the entity systems. These represent hundreds of hours of design iteration and should be carried forward without creative reinterpretation.

### 3. Simulation Layers as Narrative Framework
The five layers (Physics, Biology, Cognitive, Economic, Political) are framed as a *storytelling architecture* — each layer represents a different scale of consequence, and events cascade upward through them. The specific tick rates and update scheduling are left to the technical design.

**Rationale**: The causal cascade is the core creative innovation of Neon Oubliette. It's what makes a mugging in an alley ripple into a political crisis. This must survive regardless of whether the successor uses the same phase-locked simulation approach.

### 4. Eight Spec Domains
The creative content is organized into eight specification domains that together cover the entire creative surface area:

| Spec | Covers |
|------|--------|
| `world-and-setting` | The city as place — geography, atmosphere, zones, verticality |
| `factions-and-leaders` | The five powers — AIs, philosophies, directives, aesthetics |
| `religions-and-belief` | The six faiths — dogma, worship, faction entanglement |
| `entities-and-creatures` | Everything that lives — species, agents, items, drones |
| `economy-and-crime` | Everything that flows — markets, crime, reputation, politics |
| `simulation-layers` | The causal engine — how consequences propagate |
| `dialogue-and-narrative` | How the city speaks — grammar, Ink, speech, information |
| `crisis-and-emergence` | How the city breaks — crises, cascades, emergent stories |

**Rationale**: These domains are cross-cutting enough to be useful references during implementation (a developer building the economy doesn't need to read the dialogue spec) but comprehensive enough that together they capture the full creative vision.
### 5. Design Philosophy as Binding Constraint
Three rules from the original design are elevated to binding constraints for the successor:
1. **Every system must have a visible symptom** — if the player can't see it happening, it doesn't exist.
2. **Every system must have a player verb** — if the player can't act on it, it's backdrop, not gameplay.
3. **Every system must have procedural depth** — if it doesn't interact with other systems to create emergence, it's a static feature, not a simulation.

### 6. Open Source Asset Architecture
To maintain the clinical "Medical Diagnostic" aesthetic while ensuring rapid development:
- **Iconography**: The project SHALL use an open-source, thin-stroke icon library (e.g., Lucide, Phosphor, or Heroicons).
- **Rationale**: This provides a consistent visual vocabulary for the diagnostic inspection tools across all five simulation layers without custom asset bottlenecks.

### 7. Player Backstory: The Engineer's Legacy
The player character is the child of a high-level **Pre-AGI System Architect** who foresaw the city's transformation into an Oubliette.
- **The Items**: The five investigative tools are "Legacy Firmware" prototypes developed by the player's mother before she disappeared during the **Great Erasure**.
- **The Secret**: These tools are illegal, unsanctioned, and invisible to the city's current surveillance—as long as the player is discrete. They are the only "unfiltered" lenses through which the true state of the city can be seen.
- **The Motivation**: The player is a "Ghost" in the machine, using their mother's tools to survive the slums, earn credits, and eventually penetrate the AGI Cores to find the truth behind her disappearance and the city's root code.

### 8. Player Death: The Erasure
In the Oubliette, death is not the end of the simulation, but a moment of "Systemic Erasure." When the player's **L1 Biology (Health)** reaches zero, the city's automated systems begin recycling the physical remains.

#### The Death Cascade (All 5 Layers)
- **L0 Physics (The Body)**: The physical corpse is removed from the world. If the player had high-tier L0 tools, their "Battery Capacity" is permanently reduced by 5% until repaired at a high-end tech shop.
- **L1 Biology (The Recovery)**: The player "wakes up" in a random **Slum Squat** or **Safe House** in a different district. They are at 10% Health and 90% Frustration.
- **L2 Cognitive (The Memory Loss)**: The player loses 25% of their accumulated **Information (Rumors/Intelligence)**. The city "forgets" some of what you were investigating.
- **L3 Economic (The Asset Strip)**: All physical items (L3) and 50% of on-hand Credits are lost. They are left as a "Lootable Corpse" in the world, which NPCs (especially **Scavenger Thieves**) will prioritize robbing.
- **L4 Political (The Ghost Signature)**: If the player died in a restricted zone, their **Wanted Level** with the dominant faction spikes. Faction security detected the "Illegal Firmware" (the Tools) during the death event.

#### The "Mother's Legacy" Exception
The player **NEVER** loses their five investigative tools. These are hard-coded into their neural architecture. However, if the player dies frequently, the tools' "Signal Signature" becomes more visible to the AGI, increasing the passive "Heat" generated during tool use until a "Neural Scrub" is performed.

## Risks / Trade-offs
**Rationale**: These three rules are what separate Neon Oubliette from a static cyberpunk RPG. They ensure the successor remains a living simulation rather than drifting toward scripted content.

## Risks / Trade-offs

- **[Risk] Creative Drift** → Without the original codebase as anchor, the successor could gradually deviate from the established creative vision during implementation.
  - *Mitigation*: This document serves as the canonical reference. Specs use normative language (SHALL/MUST) for creative requirements that must be honored.

- **[Risk] Scope Paralysis** → The sheer breadth of creative systems (76 original systems across 5 layers) could overwhelm the successor team, leading to analysis paralysis.
  - *Mitigation*: The tasks artifact will prioritize creative systems by dependency order, allowing incremental buildout.

- **[Trade-off] Fidelity vs. Freedom** → Preserving entity systems verbatim may constrain the successor's ability to simplify or innovate.
  - *Mitigation*: The entity vocabulary is preserved as a creative reference, not a rigid schema. The successor can adjust granularity as needed while honoring the creative intent (e.g., consolidating some agent tasks if the simulation doesn't need 40+ distinct behaviors at launch).

- **[Trade-off] Completeness vs. Readability** → A truly complete creative document would be unwieldy. Some details are necessarily compressed.
  - *Mitigation*: Each spec is self-contained and focused on one domain. Cross-references between specs are explicit.
