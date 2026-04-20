## Context

The scan focus marker is rendered whenever `scanTarget` is set. After target focus became possible before firing a scan tool, that same marker represented both "focused target" and "opened scan panel target."

## Goals / Non-Goals

**Goals:**

- Make pre-fire focus visually distinct from active scan results.
- Keep the active scan panel marker readable and unchanged in meaning.

**Non-Goals:**

- New art assets, animations, or input changes.
- Changing target selection rules.

## Decisions

- Use amber corner brackets for pre-fire focus. This reads as a provisional lock without occupying as much visual weight as the active marker.
- Keep the cyan double rectangle for active scan panels. It already reads as the committed scan result and aligns with the existing scan HUD color.

## Risks / Trade-offs

- Corner brackets are minimal and may be subtle at small scales -> Mitigated by using a high-contrast amber color and minimum corner length.
