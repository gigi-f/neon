# Requirements

### Requirement: Transit Arrival Hurrying
NPCs with a `GoalComponent` targeting a transit station SHALL switch their `MovementSpeed` to "HURRY" if a transit vehicle of their `TransitRouteComponent` is currently at the station or within a `HURRY_THRESHOLD` radius.

#### Scenario: Running to Catch the Train
- **WHEN** a train is detected at a station and an NPC is within 10 World Units (WU) of that station
- **THEN** the NPC's `MovementSpeed` SHALL increase to its maximum speed until the NPC reaches the boarding zone.

### Requirement: Transit Stop Clustering
NPCs waiting for transit SHALL cluster near `TransitStationComponent` or `BusStopComponent` entities in real-time, effectively queuing for the arrival of the next vehicle based on proximity.

#### Scenario: Pedestrian Congestion at Bus Stop
- **WHEN** multiple NPCs are waiting for the "City Express Bus"
- **THEN** they SHALL position themselves in a cluster immediately adjacent to the bus stop marker's collision radius.
