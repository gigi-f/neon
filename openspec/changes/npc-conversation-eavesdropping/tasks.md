## 1. Data Model
- [x] 1.1 Add `ConversationComponent` to `src/components.h` with `cooldown` and `last_partner` fields.
- [x] 1.2 Assign `ConversationComponent` during ambient citizen spawn.

## 2. Conversation System
- [x] 2.1 Implement `ConversationSystem` in `src/simulation_systems.h` with proximity/affinity/cooldown pairing rules.
- [x] 2.2 Add deterministic fragment template generation from schedule/market/danger/status context.
- [x] 2.3 Record HEARD_RUMOR memory entries and PAD adjustments for both participants.
- [x] 2.4 Emit eavesdrop intel alerts when player is within listen radius.

## 3. Alert & Main Loop Wiring
- [x] 3.1 Extend `SimulationAlertCategory` with `INTEL` and ensure HUD/Intel panel category labels render it.
- [x] 3.2 Instantiate and run `ConversationSystem` in L2 between relationship and rumor systems.

## 4. Tests
- [x] 4.1 Add `tests/conversation_system_tests.cpp` for cooldown gating and one-conversation-per-tick behavior.
- [x] 4.2 Add test coverage for eavesdrop alert emission when player is near.
- [x] 4.3 Register the test target in `CMakeLists.txt`.

## 5. Validation
- [x] 5.1 Run `openspec validate npc-conversation-eavesdropping --strict`.
- [x] 5.2 Build and run tests with `rtk test ctest --test-dir build --output-on-failure`.
