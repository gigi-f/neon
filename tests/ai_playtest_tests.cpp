#include <cassert>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <string>

#include "ai_playtest.h"

static void testDefaultSnapshotExposesAiReadableState() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("=== NEON AI PLAYTEST ===") != std::string::npos);
    assert(snapshot.find("COMMANDS: snapshot | key W/A/S/D/E/F/T/SPACE/G") != std::string::npos);
    assert(snapshot.find("PLAYER:") != std::string::npos);
    assert(snapshot.find("GADGET:") != std::string::npos);
    assert(snapshot.find("-- PLAYER VIEW 33x17 CELL=8WU CENTERED ON @ PHASE: DAY") != std::string::npos);
    assert(snapshot.find("TARGET_DETAIL: PHASE: DAY") != std::string::npos);
    assert(snapshot.find("LEGEND: @ player ^v<> facing") != std::string::npos);
    assert(snapshot.find('@') != std::string::npos);
    assert(std::count(snapshot.begin(), snapshot.end(), 'w') >= 2);

    const auto view = aiPlaytestPlayerView(session.registry, session.player);
    assert(view.size() == 17);
    assert(view[0].size() == 33);
    assert(view[8][16] == '@');
}

static void testSuspicionFixtureLetsAiLayLowInHousing() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    Entity housing = firstBuildingByRole(session.registry, MicroZoneRole::HOUSING);
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

static void testSuspicionFixtureLetsAiExerciseWageSpoofLoop() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session, AiPlaytestScenario::SUSPICION));

    const std::string before = aiPlaytestSnapshot(session);
    assert(before.find("LOCAL NOTICE: WORKER SAW MISSING PART") != std::string::npos);
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
    assert(worker_mismatch.find("LOCAL WITNESS: WORKER") != std::string::npos);

    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string worker_wage_spoofed = aiPlaytestSnapshot(session);
    assert(worker_wage_spoofed.find("TARGET: WORKER") != std::string::npos);
    assert(worker_wage_spoofed.find("WAGE IMPACT: RECORD ALTERED") != std::string::npos);
    assert(worker_wage_spoofed.find("DOCK RISK: CLEARED") != std::string::npos);
    assert(worker_wage_spoofed.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") !=
           std::string::npos);
    assert(worker_wage_spoofed.find("LOCAL WITNESS: WORKER") != std::string::npos);

    assert(warpAiPlaytestPlayer(session, "CLINIC", &result));
    assert(applyAiPlaytestKey(session, "G", &result));
    assert(result == "KEY G OK");
    const std::string restored = aiPlaytestSnapshot(session);
    assert(restored.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(restored.find("GHOST CLEARANCE") == std::string::npos);
}

static void testPlaytestStateFileRoundTrip() {
    const std::string path = "/tmp/neon_ai_playtest_state_test.txt";
    std::remove(path.c_str());

    AiPlaytestSession saved;
    assert(buildAiPlaytestSession(saved));
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

    std::remove(path.c_str());
}

int main() {
    testDefaultSnapshotExposesAiReadableState();
    testSyntheticKeysMutateSameGameState();
    testSuspicionFixtureLetsAiExerciseWageSpoofLoop();
    testSuspicionFixtureExposesClinicAccessLedgerLoop();
    testSuspicionFixtureLetsAiLayLowInHousing();
    testPlaytestStateFileRoundTrip();
    return 0;
}
