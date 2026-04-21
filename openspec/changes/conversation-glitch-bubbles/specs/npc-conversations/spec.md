## ADDED Requirements

### Requirement: Live Speech Bubble State
When an NPC participates in a conversation event, the simulation SHALL attach transient speech-bubble state containing the emitted fragment and a non-zero TTL.

#### Scenario: Conversation activates bubble text
- **WHEN** two NPCs complete a conversation turn
- **THEN** both participants SHALL expose active speech bubble text with positive TTL.

### Requirement: Bubble Expiration
Speech bubbles SHALL self-expire after TTL reaches zero.

#### Scenario: Bubble line times out
- **WHEN** enough simulation time elapses after a conversation
- **THEN** the NPC speech bubble SHALL no longer render active text.

### Requirement: Glitch-Text Audibility Rendering
World rendering SHALL display conversation speech as glitch-text above speaking NPCs, with readability increasing as the player gets closer.

#### Scenario: Nearby player sees clearer text
- **WHEN** the player is near a speaking NPC
- **THEN** the speech bubble SHALL reveal more original characters than at long distance.
