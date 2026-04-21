## Implementation

- [ ] Add a highest-priority backlog item for sandbox microcity pivot and phased NPC-cycle follow-up.
- [ ] Define deterministic sandbox district anchors for housing, workplace, market, leisure, and upper-class quarters.
- [ ] Wire transit connectivity so all required districts are reachable.
- [ ] Remove procedural city generation path and make the sandbox layout authoritative for this phase.
- [ ] Add validation coverage (tests and/or startup assertions) for district presence and transit connectivity.
- [ ] Validate with `openspec validate sandbox-microcity-map --strict` and repo build/tests (`cmake --build build`, `rtk test ctest --test-dir build --output-on-failure`).

## Follow-up Change (Planned Next)

- [ ] Implement NPC daily loop across sandbox districts: `sleep => transit => work => transit => leisure => home => repeat`.
- [ ] Add loop visibility cues (debug HUD/log entries) to verify routine progression in real time.
- [ ] Revisit player interaction hooks once loop stability is demonstrated.
