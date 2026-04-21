## Context

Neon currently initializes a broad procedural city. The next gameplay milestone needs a controlled map where system interactions are inspectable and repeatable. This change pivots map creation to a deterministic sandbox microcity that still represents the eventual full-city structure.

## Goals / Non-Goals

**Goals:**

- Author a compact map that includes housing, workplace, market, leisure, and upper-class quarters.
- Connect all districts with usable transit so NPC route chains can be validated.
- Preserve enough world fidelity to exercise scanning, economy, social, and transit systems.
- Stage the NPC daily-cycle behavior as the immediate follow-on phase.

**Non-Goals:**

- No large-scale procedural city tuning in this phase.
- No new district taxonomy beyond the five required roles.
- No deep player-role redesign in this planning change.

## Proposed Approach

1. Remove the procedural city layout path and replace it with deterministic sandbox district anchors and connector roads/transit.
2. Map each required district role to explicit world coordinates and spawn rules.
3. Define transit links that guarantee route reachability between all districts.
4. Add instrumentation checks (debug/HUD or tests) that confirm required districts and transit links exist on load.
5. Implement NPC routine loop in the next phase on top of the stable sandbox topology.

## Phase Plan

- **Phase 1 (this change):** sandbox layout pivot + district/transit guarantees.
- **Phase 2 (next change):** NPC full routine cycle (`sleep => transit => work => transit => leisure => home => repeat`).
- **Phase 3:** player role tuning against validated sandbox loop.

## Risks / Trade-offs

- Reduced procedural variety may temporarily hide scale-related issues.
- Dense sandbox clustering can create artificial congestion; this is acceptable while validating loop readability.
- Existing TODO items tied to large procedural expansion should remain deferred until sandbox loop goals are met.
