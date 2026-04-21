## Context

The sandbox pivot should be delivered incrementally by district subsection. Housing is the first subsection because it is the foundation for NPC home/sleep state and future spawn-origination behavior.

## Goals / Non-Goals

**Goals:**

- Create a deterministic housing district with clear zoning and building representation.
- Ensure the housing district is connected to the transit spine.
- Validate startup invariants for housing and transit station indexing.

**Non-Goals:**

- No workplace, market, leisure, or upper-class district implementation in this change.
- No NPC full daily-cycle behavior yet.

## Approach

1. Replace world generation with a housing-focused sandbox layout.
2. Build a fixed maglift track and station set, with one station serving housing.
3. Add housing district building(s) with residential zoning and structural/power metadata.
4. Add a startup validation helper used by runtime and tests.

## Risks

- Minimal district coverage means some systems (economy/social variety) are intentionally underrepresented until later subsections land.
