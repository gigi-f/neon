## 1. Specification

- [x] 1.1 Add route-flow recovery and persistence-boundary requirements.

## 2. Implementation

- [x] 2.1 Derive recovered flow readouts from restored route signposts.
- [x] 2.2 Show recovery on the affected signpost, path, and workplace flow readout.
- [x] 2.3 Keep recovery local and clear stale blockage/disruption text.
- [x] 2.4 Keep active blockage persistence derived from saved spoofed signposts, without adding recovery event persistence.

## 3. Tests And Bookkeeping

- [x] 3.1 Add blocked-to-recovered and stale-readout-clearing tests.
- [x] 3.2 Add save/load persistence-boundary tests for active blockage and restored clear state.
- [x] 3.3 Update `todo/TODO.md` Phase 70 status.
- [x] 3.4 Validate OpenSpec, build, CTest through rtk, and C++ diagnostics.
