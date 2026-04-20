## ADDED Requirements

### Requirement: Item flags visible through scan panels
Item legal, value, uniqueness, and faction-relevance flags SHALL be visible when scanning item entities.

#### Scenario: Surface Scan shows item flags
- **WHEN** Surface Scan targets an item entity with one or more flags
- **THEN** the scan panel SHALL include compact item flag labels

#### Scenario: Financial Forensics shows item flags
- **WHEN** Financial Forensics targets an item entity with one or more flags
- **THEN** the scan panel SHALL include compact item flag labels
