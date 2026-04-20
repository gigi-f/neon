# organ-health Specification

## Purpose
TBD - created by archiving change organ-health-vital-signs. Update Purpose after archive.
## Requirements
### Requirement: Organ Health Tracking
The system SHALL track the health of 5 critical organs: Heart, Lungs, Brain, Liver, Kidneys.
Each organ has a health value from 0.0 to 100.0, initialized at 100.0.

#### Scenario: Organ Initialization
- **WHEN** a `BiologyComponent` is created
- **THEN** it SHALL initialize the `OrganHealth` array/struct with all 5 organs at 100.0 health.

### Requirement: Vital Signs Tracking
The system SHALL track basic vital signs derived from or related to organ health:
Blood Pressure (Systolic/Diastolic), Heart Rate (bpm), and Oxygen Saturation (0.0-1.0).

#### Scenario: Vital Signs Initialization
- **WHEN** a `BiologyComponent` is created
- **THEN** it SHALL initialize `blood_pressure` to [120.0, 80.0], `heart_rate` to 80.0, and `oxygen_saturation` to 1.0.

