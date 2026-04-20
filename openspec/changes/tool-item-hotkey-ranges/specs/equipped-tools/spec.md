## MODIFIED Requirements

### Requirement: Numeric hotkey equipment model
The player SHALL have a current equipped slot that can represent no item, a survival consumable, or a carried scan tool. Numeric hotkey `0` SHALL remain assigned to no slot. Numeric hotkeys `1` through `5` SHALL be reserved for scan tools, and numeric hotkeys `6` through `9` SHALL be reserved for item or consumable slots.

#### Scenario: Tool hotkey selects carried scan tool
- **WHEN** the player presses a configured numeric hotkey from `1` through `5` for a carried scan tool
- **THEN** the equipped slot SHALL change to that scan tool

#### Scenario: Item hotkey quick-uses carried item
- **WHEN** the player presses a configured numeric hotkey from `6` through `9` for a carried consumable item
- **THEN** the matching item SHALL be consumed through the existing quick-use path
- **AND** the equipped slot SHALL reflect that item slot

#### Scenario: Empty hotkey clears equipped slot
- **WHEN** the player presses hotkey `0`
- **THEN** the equipped slot SHALL become empty

---

### Requirement: Default tool and item hotkey layout
The default numeric hotkey assignments SHALL bind the five scan tools to `1` through `5`, bind current consumable item slots to `6` through `8`, leave `9` available for future item bindings, and keep `0` empty.

#### Scenario: Default hotkeys reserve primary keys for tools
- **WHEN** a new player equipment component is created
- **THEN** hotkey `1` SHALL bind Surface Scan
- **AND** hotkey `2` SHALL bind Biological Audit
- **AND** hotkey `3` SHALL bind Cognitive Profile
- **AND** hotkey `4` SHALL bind Financial Forensics
- **AND** hotkey `5` SHALL bind Structural Analysis

#### Scenario: Default hotkeys reserve later keys for items
- **WHEN** a new player equipment component is created
- **THEN** hotkey `6` SHALL bind Food
- **AND** hotkey `7` SHALL bind Water
- **AND** hotkey `8` SHALL bind Medical
- **AND** hotkey `9` SHALL remain unassigned
