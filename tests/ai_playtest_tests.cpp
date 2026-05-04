#include <cassert>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <string>

#include "ai_playtest.h"

static void advanceAiStationToBoarding(Registry& registry, Entity station) {
    assert(station != MAX_ENTITIES);
    for (int i = 0; i < 120; ++i) {
        if (transitSignalBoardingNow(registry.get<StationComponent>(station))) {
            return;
        }
        advanceAiPlaytestSimulation(registry, kAiPlaytestStepDt);
    }
    assert(false && "AI station signal should reach boarding state");
}

static void setAllAiStationsBoarding(Registry& registry) {
    auto stations = registry.view<StationComponent>();
    for (Entity station : stations) {
        auto& signal = registry.get<StationComponent>(station);
        signal.signal_elapsed_seconds = transitSignalArrivingEnd(signal) + 0.01f;
    }
}

static void testDefaultSnapshotExposesAiReadableState() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("=== NEON AI PLAYTEST ===") != std::string::npos);
    assert(snapshot.find("COMMANDS: snapshot | key W/A/S/D/E/F/T/SPACE/G") != std::string::npos);
    assert(snapshot.find("PLAYER:") != std::string::npos);
    assert(snapshot.find("GADGET:") != std::string::npos);
    assert(snapshot.find("DEBUGGER_TERMINAL: CLOSED") != std::string::npos);
    assert(snapshot.find("-- PLAYER VIEW 33x17 CELL=8WU CENTERED ON @ PHASE: DAY") != std::string::npos);
    assert(snapshot.find("TARGET_DETAIL: PHASE: DAY") != std::string::npos);
    assert(snapshot.find("LEGEND: @ player ^v<> facing") != std::string::npos);
    assert(snapshot.find("TARGETS: HOUSING WORKPLACE SUPPLY MARKET CLINIC LISTING STATION") !=
           std::string::npos);
    assert(snapshot.find("DISTRICT:") != std::string::npos);
    assert(snapshot.find("T transit") != std::string::npos);
    assert(snapshot.find('@') != std::string::npos);
    assert(std::count(snapshot.begin(), snapshot.end(), 'w') >= 2);

    const auto view = aiPlaytestPlayerView(session.registry, session.player);
    assert(view.size() == 17);
    assert(view[0].size() == 33);
    assert(view[8][16] == '@');
}

static void testDebuggerTerminalOpensInAiSnapshotAfterScan() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "SPACE", &result));
    assert(result == "KEY SPACE OK");

    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("DEBUGGER_TERMINAL: OPEN") != std::string::npos);
    assert(snapshot.find("MOTHER'S DEBUGGER // TERMINAL") != std::string::npos);
    assert(snapshot.find("TARGET: CLINIC") != std::string::npos);
    assert(snapshot.find("CLINIC LAYOUT") != std::string::npos);
}

static void testDebuggerResultStaysReadableAcrossTerminalWindowStates() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "SPACE", &result));
    const std::string open_snapshot = aiPlaytestSnapshot(session);
    assert(open_snapshot.find("DEBUGGER_TERMINAL: OPEN") != std::string::npos);
    assert(open_snapshot.find("DEBUGGER_RESULT: DEBUGGER RESULT: ON CLINIC") !=
           std::string::npos);
    assert(open_snapshot.find("TARGET_DEBUGGER_SCAN: B:CLINIC; CLINIC PURPOSE: PUBLIC HEALTH") !=
           std::string::npos);

    auto& terminal = session.registry.get<DebuggerTerminalComponent>(session.player);
    minimizeDebuggerTerminal(terminal);
    const std::string minimized_snapshot = aiPlaytestSnapshot(session);
    assert(minimized_snapshot.find("DEBUGGER_TERMINAL: OPEN MINIMIZED") !=
           std::string::npos);
    assert(minimized_snapshot.find("DEBUGGER_RESULT: DEBUGGER RESULT: ON CLINIC") !=
           std::string::npos);
    assert(minimized_snapshot.find("CLINIC LAYOUT") != std::string::npos);

    closeDebuggerTerminal(terminal);
    const std::string closed_snapshot = aiPlaytestSnapshot(session);
    assert(closed_snapshot.find("DEBUGGER_TERMINAL: CLOSED") != std::string::npos);
    assert(closed_snapshot.find("DEBUGGER_RESULT: DEBUGGER RESULT: ON CLINIC") !=
           std::string::npos);
    assert(closed_snapshot.find("TARGET_DEBUGGER_SCAN: B:CLINIC; CLINIC PURPOSE: PUBLIC HEALTH") !=
           std::string::npos);
}

static void testKeyboardInterferenceWorksWhileDebuggerTerminalIsOpen() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "SPACE", &result));
    const std::string scanned = aiPlaytestSnapshot(session);
    assert(scanned.find("DEBUGGER_TERMINAL: OPEN") != std::string::npos);
    assert(scanned.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(scanned.find("G INTERFERENCE TORCH SPOOF CLINIC ACCESS") !=
           std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string spoofed = aiPlaytestSnapshot(session);
    assert(spoofed.find("DEBUGGER_TERMINAL: OPEN") != std::string::npos);
    assert(spoofed.find("INTERFERENCE TORCH RESULT: ON CLINIC") !=
           std::string::npos);
    assert(spoofed.find("CLINIC ACCESS: GHOST CLEARANCE") != std::string::npos);
    assert(spoofed.find("G INTERFERENCE TORCH RESTORE CLINIC ACCESS") !=
           std::string::npos);
}

static void testDefaultClinicTargetExposesLayout() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("TARGET: CLINIC") != std::string::npos);
    assert(snapshot.find("CLINIC LAYOUT: INTAKE PUBLIC; TREATMENT PUBLIC; RECORDS STAFF; SERVICE STAFF") !=
           std::string::npos);
    assert(snapshot.find("CLINIC BOUNDARY: RECORDS STAFF ONLY; ACCESS: DENIED") !=
           std::string::npos);
    assert(snapshot.find("ACTION: LOCATION:OUTSIDE E CLINIC RECORDS BOUNDARY DENIED") !=
           std::string::npos);
    assert(snapshot.find("CLINIC LEDGER") == std::string::npos);
}

static void testShelterFixtureExposesListingInterestLoop() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SHELTER));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "LISTING", &result));
    std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("TARGET: SHELTER LISTING") != std::string::npos);
    assert(snapshot.find("SHELTER LISTING:") != std::string::npos);
    assert(snapshot.find("HOME BASE: UNCHANGED") != std::string::npos);
    assert(snapshot.find("INTEREST: UNMARKED") != std::string::npos);
    assert(snapshot.find("E MARK SHELTER INTEREST") !=
           std::string::npos);

    assert(applyAiPlaytestKey(session, "SPACE", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("TARGET_DEBUGGER_SCAN:") != std::string::npos);
    assert(snapshot.find("SHELTER LISTING; TYPE:") != std::string::npos);
    assert(snapshot.find("TRANSFER: LOCKED") != std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("ACTION RESULT: ON SHELTER LISTING") != std::string::npos);
    assert(snapshot.find("SHELTER INTEREST MARKED: RECORD INQUIRY OPENED") !=
           std::string::npos);
    assert(snapshot.find("INTEREST: MARKED (VOLATILE)") != std::string::npos);
    assert(snapshot.find("E CLEAR SHELTER INTEREST") !=
           std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("SHELTER INTEREST CLEARED: RECORD INQUIRY REMOVED") !=
           std::string::npos);
    assert(snapshot.find("INTEREST: UNMARKED") != std::string::npos);
}

static void testSuspicionFixtureLetsAiLayLowInHousing() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    Entity housing = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    session.registry.get<ShelterStockComponent>(housing).current_supply = 1;

    std::string result;
    assert(warpAiPlaytestPlayer(session, "HOUSING", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");
    const std::string ready = aiPlaytestSnapshot(session);
    assert(ready.find("location=\"INSIDE HOUSING\"") != std::string::npos);
    assert(ready.find("T LAY LOW") != std::string::npos);
    assert(ready.find("BUILDING SUPPLY: 1/1") != std::string::npos);

    assert(applyAiPlaytestKey(session, "T", &result));
    assert(result == "KEY T OK");
    const std::string laid_low = aiPlaytestSnapshot(session);
    assert(laid_low.find("ACTION RESULT: ON HOUSING INTERIOR") != std::string::npos);
    assert(laid_low.find("LAY LOW: LOCAL NOTICE QUIETED") != std::string::npos);
    assert(laid_low.find("BUILDING SUPPLY: 0/1") != std::string::npos);
    assert(laid_low.find("WORKER SAW MISSING PART") == std::string::npos);
    assert(laid_low.find("carrying=PART") != std::string::npos);
}

static void testSyntheticKeysMutateSameGameState() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    const float start_y = session.registry.get<TransformComponent>(session.player).y;
    std::string result;
    assert(applyAiPlaytestKey(session, "W", &result));
    assert(result == "KEY W OK");
    assert(session.registry.get<TransformComponent>(session.player).y < start_y);

    assert(warpAiPlaytestPlayer(session, "HOUSING", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");
    assert(session.registry.get<BuildingInteractionComponent>(session.player).inside_building);
    assert(playerLocationState(session.registry,
                               session.player,
                               kAiPlaytestInteractionRangeWu) ==
           PlayerLocationState::INSIDE_HOUSING);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(!session.registry.get<BuildingInteractionComponent>(session.player).inside_building);
    const std::string near_housing = aiPlaytestActionLine(session.registry, session.player);
    assert(near_housing.find("E ENTER HOUSING") != std::string::npos);

    assert(warpAiPlaytestPlayer(session, "WORKER", &result));
    const std::string near_worker = aiPlaytestActionLine(session.registry, session.player);
    assert(near_worker.find("T TALK WORKER") != std::string::npos);
    assert(applyAiPlaytestKey(session, "T", &result));
    assert(result == "KEY T OK");
    const Entity worker = firstFixedWorker(session.registry);
    assert(worker != MAX_ENTITIES);
    assert(session.registry.get<FixedActorComponent>(worker).acknowledged);
}

static void testTransitLookOutWindowChoiceMovesToDestination() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "STATION", &result));
    setAllAiStationsBoarding(session.registry);
    const uint32_t origin_district = playerCurrentDistrictId(session.registry, session.player);
    assert(aiPlaytestActionLine(session.registry, session.player).find("E BOARD TRANSIT") !=
           std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");
    assert(session.registry.has<TransitRideComponent>(session.player));
    const auto ride = session.registry.get<TransitRideComponent>(session.player);
    assert(ride.destination_district_id != origin_district);
    const float interior_x = ride.interior_position.x;

    const std::string riding = aiPlaytestSnapshot(session);
    assert(riding.find("location=\"INSIDE TRANSIT\"") != std::string::npos);
    assert(riding.find("E LOOK OUT WINDOW") != std::string::npos);
    assert(riding.find("DOORS: CLOSED") != std::string::npos);

    assert(applyAiPlaytestKey(session, "D", &result));
    assert(session.registry.has<TransitRideComponent>(session.player));
    assert(session.registry.get<TransitRideComponent>(session.player).interior_position.x >
           interior_x);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");
    assert(!session.registry.has<TransitRideComponent>(session.player));
    assert(playerCurrentDistrictId(session.registry, session.player) ==
           ride.destination_district_id);
    const std::string arrived = aiPlaytestSnapshot(session);
    assert(arrived.find("location=\"NEAR TRANSIT\"") != std::string::npos);
    assert(arrived.find("TRANSIT STATION") != std::string::npos);
}

static void testTransitStationSignalWaitThenBoard() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "STATION", &result));
    const Entity station = nearestStationInRange(session.registry,
                                                 session.registry.get<TransformComponent>(session.player),
                                                 kAiPlaytestInteractionRangeWu);
    assert(station != MAX_ENTITIES);
    auto& signal = session.registry.get<StationComponent>(station);
    signal.signal_elapsed_seconds = transitSignalBoardingEnd(signal);
    assert(!transitSignalBoardingNow(signal));

    const std::string wait_line = aiPlaytestActionLine(session.registry, session.player);
    assert(wait_line.find("E WAIT TRANSIT") != std::string::npos);
    assert(wait_line.find("TRAIN: DOORS CLOSING") != std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(!session.registry.has<TransitRideComponent>(session.player));
    assert(inheritedGadgetResultReadout(session.registry, session.player).find("WAIT") !=
           std::string::npos);

    for (int i = 0; i < 90 && !transitSignalBoardingNow(signal); ++i) {
        assert(applyAiPlaytestKey(session, "WAIT", &result));
    }
    assert(transitSignalBoardingNow(signal));
    const std::string board_line = aiPlaytestActionLine(session.registry, session.player);
    assert(board_line.find("E BOARD TRANSIT") != std::string::npos);
    assert(applyAiPlaytestKey(session, "E", &result));
    assert(session.registry.has<TransitRideComponent>(session.player));
    assert(inheritedGadgetResultReadout(session.registry, session.player).find("BOARDING NOW") !=
           std::string::npos);
}

static void testTransitWaitChoiceOpensDoorsBeforeExit() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "STATION", &result));
    setAllAiStationsBoarding(session.registry);
    const uint32_t origin_district = playerCurrentDistrictId(session.registry, session.player);
    assert(applyAiPlaytestKey(session, "E", &result));
    assert(session.registry.has<TransitRideComponent>(session.player));
    const uint32_t destination_district =
        session.registry.get<TransitRideComponent>(session.player).destination_district_id;
    assert(destination_district != origin_district);

    for (int i = 0; i < 10; ++i) {
        assert(applyAiPlaytestKey(session, "WAIT", &result));
        assert(result == "KEY WAIT OK");
    }

    assert(session.registry.has<TransitRideComponent>(session.player));
    assert(session.registry.get<TransitRideComponent>(session.player).doors_open);
    const std::string stopped = aiPlaytestSnapshot(session);
    assert(stopped.find("E EXIT TRANSIT") != std::string::npos);
    assert(stopped.find("DOORS: OPEN") != std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");
    assert(!session.registry.has<TransitRideComponent>(session.player));
    assert(playerCurrentDistrictId(session.registry, session.player) == destination_district);
}

static void testSignpostWarpUsesCurrentDistrictAfterTransit() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "STATION", &result));
    setAllAiStationsBoarding(session.registry);
    const uint32_t origin_district = playerCurrentDistrictId(session.registry, session.player);
    assert(applyAiPlaytestKey(session, "E", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    const uint32_t destination_district =
        playerCurrentDistrictId(session.registry, session.player);
    assert(destination_district != origin_district);

    const Entity local_signpost = aiFirstSignpostInCurrentDistrict(session);
    assert(local_signpost != MAX_ENTITIES);
    assert(districtIdForEntity(session.registry, local_signpost) == destination_district);
    assert(warpAiPlaytestPlayer(session, "SIGNPOST", &result));
    assert(result == "WARP SIGNPOST OK");
    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("DISTRICT: " + districtLabel(destination_district)) !=
           std::string::npos);
    assert(routeSignpostReadout(session.registry, local_signpost)
               .find(districtLabel(destination_district) + ":ROUTE") !=
           std::string::npos);
    assert(routeSignpostReadout(session.registry, local_signpost)
               .find(districtLabel(origin_district) + ":ROUTE") ==
           std::string::npos);
}

static void testSuspicionFixtureLetsAiExerciseWageSpoofLoop() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    const std::string before = aiPlaytestSnapshot(session);
    assert(before.find("LOCAL NOTICE: A:WORKER SAW MISSING PART") != std::string::npos);
    assert(before.find("G INTERFERENCE TORCH SPOOF WAGE RECORD") != std::string::npos);
    assert(before.find("DOCK RISK: ACTIVE") != std::string::npos);

    std::string result;
    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    assert(inheritedGadgetResultReadout(session.registry, session.player).find(
               "SPOOFED WAGE RECORD: INCIDENT CLEARED") != std::string::npos);
    const std::string spoofed = aiPlaytestSnapshot(session);
    assert(spoofed.find("INTERFERENCE TORCH RESULT: ON WORKER") != std::string::npos);
    assert(warpAiPlaytestPlayer(session, "WORKER", &result));
    const std::string after = aiPlaytestSnapshot(session);
    assert(after.find("G INTERFERENCE TORCH RESTORE WAGE RECORD") != std::string::npos);
    assert(after.find("DOCK RISK: CLEARED") != std::string::npos);
}

static void testSuspicionFixtureExposesClinicAccessLedgerLoop() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    const std::string flagged = aiPlaytestSnapshot(session);
    assert(flagged.find("TARGET: CLINIC") != std::string::npos);
    assert(flagged.find("CLINIC LAYOUT: INTAKE PUBLIC; TREATMENT PUBLIC; RECORDS STAFF; SERVICE STAFF") !=
           std::string::npos);
    assert(flagged.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(flagged.find("G INTERFERENCE TORCH SPOOF CLINIC ACCESS") != std::string::npos);
    assert(flagged.find("GHOST CLEARANCE") == std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string spoofed = aiPlaytestSnapshot(session);
    assert(spoofed.find("INTERFERENCE TORCH RESULT: ON CLINIC") != std::string::npos);
    assert(spoofed.find("CLINIC ACCESS: GHOST CLEARANCE") != std::string::npos);
    assert(spoofed.find("G INTERFERENCE TORCH RESTORE CLINIC ACCESS") !=
           std::string::npos);

    assert(warpAiPlaytestPlayer(session, "WORKER", &result));
    const std::string worker_mismatch = aiPlaytestSnapshot(session);
    assert(worker_mismatch.find("TARGET: WORKER") != std::string::npos);
    assert(worker_mismatch.find("WAGE IMPACT: INCIDENT LOGGED") != std::string::npos);
    assert(worker_mismatch.find("DOCK RISK: ACTIVE") != std::string::npos);
    assert(worker_mismatch.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") !=
           std::string::npos);
    assert(worker_mismatch.find("LOCAL WITNESS: A:WORKER") != std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string worker_wage_spoofed = aiPlaytestSnapshot(session);
    assert(worker_wage_spoofed.find("TARGET: WORKER") != std::string::npos);
    assert(worker_wage_spoofed.find("WAGE IMPACT: RECORD ALTERED") != std::string::npos);
    assert(worker_wage_spoofed.find("DOCK RISK: CLEARED") != std::string::npos);
    assert(worker_wage_spoofed.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") !=
           std::string::npos);
    assert(worker_wage_spoofed.find("LOCAL WITNESS: A:WORKER") != std::string::npos);

    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string restored = aiPlaytestSnapshot(session);
    assert(restored.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(restored.find("GHOST CLEARANCE") == std::string::npos);
}

static void testSuspicionFixtureExposesClinicRestrictedBoundary() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("ACTION RESULT: ON CLINIC") != std::string::npos);
    assert(snapshot.find("CLINIC ACCESS DENIED: RECORDS STAFF ONLY") != std::string::npos);
    assert(snapshot.find("inside=0") != std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("ACTION RESULT: ON CLINIC") != std::string::npos);
    assert(snapshot.find("CLINIC RECORDS BOUNDARY OPEN: GHOST CLEARANCE ACCEPTED") !=
           std::string::npos);
    assert(snapshot.find("INTERIOR: role=CLINIC") != std::string::npos);
    assert(snapshot.find("inside=1") != std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("RESTORED CLINIC ACCESS: WORK RECORD FLAGGED") != std::string::npos);
    assert(snapshot.find("CLINIC BOUNDARY: RECORDS STAFF ONLY; ACCESS: DENIED") !=
           std::string::npos);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(applyAiPlaytestKey(session, "E", &result));
    snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("CLINIC ACCESS DENIED: RECORDS STAFF ONLY") != std::string::npos);
    assert(snapshot.find("inside=0") != std::string::npos);
}

static void testPlaytestStateFileRoundTrip() {
    const std::string path = "/tmp/neon_ai_playtest_state_test.txt";
    std::remove(path.c_str());

    AiPlaytestSession saved;
    assert(buildAiPlaytestSession(saved));
    assert(warpAiPlaytestPlayer(saved, "SIGNPOST"));
    const Entity signpost = aiFirstSignpostInCurrentDistrict(saved);
    assert(signpost != MAX_ENTITIES);
    const uint32_t district_id = districtIdForEntity(saved.registry, signpost);
    saved.registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    assert(toggleDependencyDisruption(saved.registry, kWorkplaceDependsOnSupply, district_id));
    assert(warpAiPlaytestPlayer(saved, "WORKER"));
    const float saved_x = saved.registry.get<TransformComponent>(saved.player).x;
    const float saved_y = saved.registry.get<TransformComponent>(saved.player).y;
    assert(saveAiPlaytestSession(saved, path));

    AiPlaytestSession loaded;
    std::string error;
    assert(loadAiPlaytestSession(loaded, path, &error));
    assert(std::fabs(loaded.registry.get<TransformComponent>(loaded.player).x - saved_x) <
           0.001f);
    assert(std::fabs(loaded.registry.get<TransformComponent>(loaded.player).y - saved_y) <
           0.001f);
    assert(dependencyDisrupted(loaded.registry, kWorkplaceDependsOnSupply, district_id));
    assert(routeSignpostSpoofed(loaded.registry, aiFirstSignpostInCurrentDistrict(loaded)));

    std::remove(path.c_str());
}

int main() {
    testDefaultSnapshotExposesAiReadableState();
    testDebuggerTerminalOpensInAiSnapshotAfterScan();
    testDebuggerResultStaysReadableAcrossTerminalWindowStates();
    testKeyboardInterferenceWorksWhileDebuggerTerminalIsOpen();
    testDefaultClinicTargetExposesLayout();
    testShelterFixtureExposesListingInterestLoop();
    testSyntheticKeysMutateSameGameState();
    testTransitLookOutWindowChoiceMovesToDestination();
    testTransitStationSignalWaitThenBoard();
    testTransitWaitChoiceOpensDoorsBeforeExit();
    testSignpostWarpUsesCurrentDistrictAfterTransit();
    testSuspicionFixtureLetsAiExerciseWageSpoofLoop();
    testSuspicionFixtureExposesClinicAccessLedgerLoop();
    testSuspicionFixtureExposesClinicRestrictedBoundary();
    testSuspicionFixtureLetsAiLayLowInHousing();
    testPlaytestStateFileRoundTrip();
    return 0;
}
