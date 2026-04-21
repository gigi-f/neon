## 1. Data Model
- [x] 1.1 Add `SpeechBubbleComponent` to `src/components.h`.

## 2. Conversation Integration
- [x] 2.1 Update `ConversationSystem` to assign/update speech bubbles for both speakers.
- [x] 2.2 Decay speech bubble TTL and clear expired lines.

## 3. Rendering
- [x] 3.1 Add glitch-text helper(s) in `src/main.cpp` for distance-based masking.
- [x] 3.2 Render speech bubbles near speaking NPCs in world-space.

## 4. Tests
- [x] 4.1 Extend conversation tests to verify speech bubble creation and expiration.

## 5. Validation
- [x] 5.1 Run `openspec validate conversation-glitch-bubbles --strict`.
- [x] 5.2 Build and run tests with `rtk test ctest --test-dir build --output-on-failure`.
