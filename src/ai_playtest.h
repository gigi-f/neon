#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "interior.h"
#include "save_state.h"
#include "world_builder.h"

inline constexpr float kAiPlaytestInteractionRangeWu = 18.0f;
inline constexpr float kAiPlaytestInspectionRangeWu = 22.0f;
inline constexpr float kAiPlaytestMoveDt = 0.20f;
inline constexpr float kAiPlaytestStepDt = 0.10f;

enum class AiPlaytestScenario {
    DEFAULT,
    SUSPICION
};

struct AiPlaytestSession {
    Registry registry;
    Entity player = MAX_ENTITIES;
    WorldConfig config{};
};

inline WorldConfig makeAiPlaytestConfig() {
    WorldConfig config = makeSandboxConfig();
    config.macro_count_x = 2;
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    config.fixed_worker_count = 2;
    config.carryable_object_count = 1;
    config.transit_enabled = true;
    config.transit_ride_seconds = 1.0f;
    config.world_phase_interval_seconds = 1.2f;
    return config;
}

inline const char* aiFacingName(Facing facing) {
    switch (facing) {
        case Facing::UP: return "UP";
        case Facing::DOWN: return "DOWN";
        case Facing::LEFT: return "LEFT";
        case Facing::RIGHT: return "RIGHT";
    }
    return "DOWN";
}

inline const char* aiLocationStateName(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "OUTSIDE";
        case PlayerLocationState::NEAR_HOUSING: return "NEAR HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "INSIDE HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "NEAR WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "INSIDE WORKPLACE";
        case PlayerLocationState::NEAR_SUPPLY: return "NEAR SUPPLY";
        case PlayerLocationState::INSIDE_SUPPLY: return "INSIDE SUPPLY";
        case PlayerLocationState::NEAR_TRANSIT: return "NEAR TRANSIT";
        case PlayerLocationState::INSIDE_TRANSIT: return "INSIDE TRANSIT";
    }
    return "OUTSIDE";
}

inline const char* aiLocationPrompt(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "";
        case PlayerLocationState::NEAR_HOUSING: return "E ENTER HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "E EXIT HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "E ENTER WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "E EXIT WORKPLACE";
        case PlayerLocationState::NEAR_SUPPLY: return "E ENTER SUPPLY";
        case PlayerLocationState::INSIDE_SUPPLY: return "E EXIT SUPPLY";
        case PlayerLocationState::NEAR_TRANSIT: return "E BOARD TRANSIT";
        case PlayerLocationState::INSIDE_TRANSIT: return "E LOOK OUT WINDOW";
    }
    return "";
}

inline const char* aiInspectionTargetName(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::NONE: return "NO TARGET";
        case InspectionTargetType::HOUSING: return "HOUSING";
        case InspectionTargetType::WORKPLACE: return "WORKPLACE";
        case InspectionTargetType::SUPPLY: return "SUPPLY";
        case InspectionTargetType::MARKET: return "MARKET";
        case InspectionTargetType::CLINIC: return "CLINIC";
        case InspectionTargetType::TRANSIT_STATION: return "TRANSIT STATION";
        case InspectionTargetType::PEDESTRIAN_PATH: return "PATH";
        case InspectionTargetType::ROUTE_SIGNPOST: return "SIGNPOST";
        case InspectionTargetType::WORKER: return "WORKER";
        case InspectionTargetType::HOUSING_INTERIOR: return "HOUSING INTERIOR";
        case InspectionTargetType::WORKPLACE_INTERIOR: return "WORKPLACE INTERIOR";
        case InspectionTargetType::SUPPLY_INTERIOR: return "SUPPLY INTERIOR";
        case InspectionTargetType::TRANSIT_INTERIOR: return "TRANSIT INTERIOR";
        case InspectionTargetType::CARRYABLE_OBJECT: return "CARRYABLE OBJECT";
    }
    return "NO TARGET";
}

inline std::string aiInspectionDetail(Registry& registry, const InspectionTarget& target) {
    if (target.entity == MAX_ENTITIES) return "Nothing close enough to read.";

    if (target.type == InspectionTargetType::PEDESTRIAN_PATH &&
        registry.alive(target.entity) &&
        registry.has<PathComponent>(target.entity)) {
        return pathInspectionReadout(registry, target.entity);
    }
    if ((target.type == InspectionTargetType::HOUSING ||
         target.type == InspectionTargetType::WORKPLACE ||
         target.type == InspectionTargetType::SUPPLY ||
         target.type == InspectionTargetType::MARKET ||
         target.type == InspectionTargetType::CLINIC) &&
        registry.alive(target.entity) &&
        registry.has<BuildingUseComponent>(target.entity)) {
        return buildingInspectionReadout(registry, target.entity);
    }
    if (target.type == InspectionTargetType::TRANSIT_STATION &&
        registry.alive(target.entity) &&
        registry.has<StationComponent>(target.entity)) {
        return stationReadout(registry, target.entity);
    }
    if (target.type == InspectionTargetType::TRANSIT_INTERIOR) {
        return "TRANSIT CAR: MOVING; WINDOW: DESTINATION PLATFORM";
    }
    if (target.type == InspectionTargetType::HOUSING_INTERIOR &&
        registry.alive(target.entity) &&
        registry.has<BuildingImprovementComponent>(target.entity)) {
        return housingInteriorReadout(registry);
    }
    if (target.type == InspectionTargetType::WORKPLACE_INTERIOR &&
        registry.alive(target.entity) &&
        registry.has<WorkplaceBenchComponent>(target.entity)) {
        return workplaceBenchLoopReadout(registry);
    }
    if (target.type == InspectionTargetType::SUPPLY_INTERIOR) {
        return "STOCK SHELF: Usable cache markers, no inventory system yet.";
    }
    if (target.type == InspectionTargetType::WORKER &&
        registry.alive(target.entity) &&
        registry.has<FixedActorComponent>(target.entity)) {
        return workerCarryReadout(registry, target.entity);
    }
    if (target.type == InspectionTargetType::ROUTE_SIGNPOST &&
        registry.alive(target.entity) &&
        registry.has<RouteSignpostComponent>(target.entity)) {
        return routeSignpostReadout(registry, target.entity);
    }
    if (target.type == InspectionTargetType::CARRYABLE_OBJECT &&
        registry.alive(target.entity) &&
        registry.has<CarryableComponent>(target.entity)) {
        return carryableObjectReadout(registry, target.entity);
    }
    return std::string(aiInspectionTargetName(target.type)) + " SIGNAL DETECTED";
}

inline Entity aiFirstPathBetweenRoles(Registry& registry,
                                      MicroZoneRole a,
                                      MicroZoneRole b) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        const auto& path = registry.get<PathComponent>(path_entity);
        if (path.kind != PathKind::PEDESTRIAN ||
            !registry.alive(path.from) ||
            !registry.alive(path.to) ||
            !registry.has<BuildingUseComponent>(path.from) ||
            !registry.has<BuildingUseComponent>(path.to)) {
            continue;
        }

        const MicroZoneRole from = registry.get<BuildingUseComponent>(path.from).role;
        const MicroZoneRole to = registry.get<BuildingUseComponent>(path.to).role;
        if ((from == a && to == b) || (from == b && to == a)) {
            return path_entity;
        }
    }
    return MAX_ENTITIES;
}

inline Entity aiFirstFixedWorkerInDistrict(Registry& registry, uint32_t district_id) {
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    std::sort(workers.begin(), workers.end());
    for (Entity worker : workers) {
        if (districtIdForEntity(registry, worker) == district_id) {
            return worker;
        }
    }
    return MAX_ENTITIES;
}

inline bool buildAiPlaytestSession(AiPlaytestSession& session,
                                   AiPlaytestScenario scenario = AiPlaytestScenario::DEFAULT) {
    session = AiPlaytestSession{};
    session.config = makeAiPlaytestConfig();
    buildWorld(session.registry, session.config);
    if (!validateWorld(session.registry, session.config)) {
        return false;
    }
    deriveInfrastructure(session.registry, session.config);
    spawnFixedActors(session.registry, session.config);

    session.player = session.registry.create();
    session.registry.assign<TransformComponent>(session.player, 0.0f, -115.0f, 12.0f, 12.0f);
    session.registry.assign<PlayerComponent>(session.player);
    session.registry.assign<InheritedGadgetComponent>(session.player);
    session.registry.assign<BuildingInteractionComponent>(session.player);
    session.registry.assign<InspectionComponent>(session.player);
    session.registry.assign<GlyphComponent>(session.player, std::string("@"),
        static_cast<uint8_t>(245), static_cast<uint8_t>(245), static_cast<uint8_t>(210),
        static_cast<uint8_t>(255), 1.0f, true, false);
    if (!validatePlayerSpawn(session.registry, session.player)) {
        return false;
    }

    if (scenario == AiPlaytestScenario::SUSPICION) {
        const Entity workplace = firstWorkplaceBenchBuilding(session.registry);
        const Entity object = firstCarryableObject(session.registry);
        const Entity path = aiFirstPathBetweenRoles(session.registry,
                                                   MicroZoneRole::WORKPLACE,
                                                   MicroZoneRole::SUPPLY);
        const Entity worker = path == MAX_ENTITIES ?
            firstFixedWorker(session.registry) :
            aiFirstFixedWorkerInDistrict(session.registry,
                                         districtIdForEntity(session.registry, path));
        if (workplace == MAX_ENTITIES || worker == MAX_ENTITIES ||
            object == MAX_ENTITIES || path == MAX_ENTITIES) {
            return false;
        }

        auto& worker_component = session.registry.get<FixedActorComponent>(worker);
        worker_component.path_entity = path;
        worker_component.route_t = routeTForPathEndpoint(session.registry, path, workplace);
        worker_component.direction = 0.0f;
        session.registry.get<TransformComponent>(worker) =
            transformOnPath(session.registry.get<TransformComponent>(path),
                            worker_component.route_t);

        session.registry.get<TransformComponent>(session.player) =
            session.registry.get<TransformComponent>(workplace);
        if (!enterBuildingInterior(session.registry, session.player, workplace)) {
            return false;
        }
        hideCarryableObject(session.registry, object);
        session.registry.get<WorkplaceBenchComponent>(workplace).state =
            WorkplaceBenchState::OUTPUT_READY;
        if (!takeWorkplaceOutput(session.registry, session.player)) {
            return false;
        }
        if (!localSuspicionActive(session.registry)) {
            recordLocalSuspicion(session.registry,
                                 worker,
                                 LocalSuspicionCause::MISSING_PART,
                                 workplace,
                                 object,
                                 path);
        }
        exitBuildingInterior(session.registry, session.player);
        session.registry.get<TransformComponent>(session.player) =
            session.registry.get<TransformComponent>(worker);
    }

    return true;
}

inline bool aiPlaytestFileExists(const std::string& path) {
    std::ifstream in(path);
    return static_cast<bool>(in);
}

inline bool saveAiPlaytestSession(AiPlaytestSession& session, const std::string& path) {
    return saveTinyStateToFile(session.registry, session.player, path) == TinySaveStatus::OK;
}

inline bool loadAiPlaytestSession(AiPlaytestSession& session,
                                  const std::string& path,
                                  std::string* error = nullptr) {
    if (!buildAiPlaytestSession(session)) {
        if (error) *error = "failed to build playtest world";
        return false;
    }
    if (!aiPlaytestFileExists(path)) {
        return true;
    }

    const TinySaveStatus status = loadTinyStateFromFile(session.registry, session.player, path);
    if (status != TinySaveStatus::OK) {
        if (error) *error = std::string("failed to load playtest state: ") + tinySaveStatusName(status);
        return false;
    }
    return true;
}

inline void advanceAiPlaytestSimulation(Registry& registry, float dt) {
    auto players = registry.view<PlayerComponent>();
    for (Entity player : players) {
        advanceTransitRide(registry, player, dt);
    }
    advanceWorldPhase(registry, dt);
    updateFixedActors(registry, dt);
    updateWorkerSupplyPickups(registry);
    updateWorkerSupplyDeliveryRoutes(registry, dt);
    updateWorkerWorkplaceBenchDropOffs(registry);
    updateWorkerWorkplaceBenchWork(registry);
    updateWorkerWorkplaceOutputPickups(registry);
    updateWorkerFinishedItemDeliveryRoutes(registry, dt);
    updateWorkerBuildingDeliveries(registry);
    updateWorkerReturnRoutes(registry, dt);
}

inline void moveAiPlaytestPlayer(Registry& registry,
                                 Entity player,
                                 float dx,
                                 float dy,
                                 float dt) {
    if (!registry.has<PlayerComponent>(player) || !registry.has<TransformComponent>(player)) return;
    if (dx == 0.0f && dy == 0.0f) return;

    auto& player_component = registry.get<PlayerComponent>(player);
    if (std::fabs(dx) > std::fabs(dy)) {
        player_component.facing = dx < 0.0f ? Facing::LEFT : Facing::RIGHT;
    } else {
        player_component.facing = dy < 0.0f ? Facing::UP : Facing::DOWN;
    }

    const float len = std::sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        auto& interaction = registry.get<BuildingInteractionComponent>(player);
        const auto layout = interiorLayoutForRole(interaction.building_role);
        interaction.interior_position = movedInteriorPosition(layout,
                                                              interaction.interior_position,
                                                              dx,
                                                              dy,
                                                              player_component.speed,
                                                              dt);
        return;
    }

    if (registry.has<TransitRideComponent>(player)) {
        auto& ride = registry.get<TransitRideComponent>(player);
        ride.interior_position = movedTransitInteriorPosition(ride.interior_position,
                                                              dx,
                                                              dy,
                                                              player_component.speed,
                                                              dt);
        return;
    }

    TransformComponent proposed = registry.get<TransformComponent>(player);
    proposed.x += dx * player_component.speed * dt;
    proposed.y += dy * player_component.speed * dt;
    if (!transformOverlapsSolid(registry, proposed, player)) {
        registry.get<TransformComponent>(player) = proposed;
    }
}

inline void toggleAiPlaytestBuildingInteraction(Registry& registry, Entity player) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player)) {
        return;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (registry.has<TransitRideComponent>(player)) {
        const auto& ride = registry.get<TransitRideComponent>(player);
        finishTransitRide(registry, player, !ride.doors_open);
        return;
    }
    if (interaction.inside_building) {
        exitBuildingInterior(registry, player);
        return;
    }

    const Entity nearest = nearestInteractableBuildingInRange(
        registry,
        registry.get<TransformComponent>(player),
        kAiPlaytestInteractionRangeWu);
    if (nearest != MAX_ENTITIES) {
        enterBuildingInterior(registry, player, nearest);
    }
}

inline void performAiPlaytestInspection(Registry& registry, Entity player) {
    if (registry.alive(player) &&
        registry.has<InspectionComponent>(player) &&
        registry.has<TransformComponent>(player)) {
        auto& inspection = registry.get<InspectionComponent>(player);
        const InspectionTarget target =
            playerInspectionTarget(registry, player, kAiPlaytestInspectionRangeWu);
        inspection.target_entity = target.entity;
        inspection.target_type = target.type;
        inspection.has_result = true;
    }
    useInheritedGadget(registry, player, kAiPlaytestInspectionRangeWu);
}

inline std::string normalizeAiPlaytestKey(std::string key) {
    std::string out;
    out.reserve(key.size());
    for (char c : key) {
        if (c == ' ') continue;
        out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (out == "ARROWUP") return "W";
    if (out == "ARROWDOWN") return "S";
    if (out == "ARROWLEFT") return "A";
    if (out == "ARROWRIGHT") return "D";
    return out;
}

inline bool applyAiPlaytestKey(AiPlaytestSession& session,
                               const std::string& raw_key,
                               std::string* result = nullptr) {
    const std::string key = normalizeAiPlaytestKey(raw_key);
    bool handled = true;
    if (key == "W") {
        moveAiPlaytestPlayer(session.registry, session.player, 0.0f, -1.0f, kAiPlaytestMoveDt);
    } else if (key == "S") {
        moveAiPlaytestPlayer(session.registry, session.player, 0.0f, 1.0f, kAiPlaytestMoveDt);
    } else if (key == "A") {
        moveAiPlaytestPlayer(session.registry, session.player, -1.0f, 0.0f, kAiPlaytestMoveDt);
    } else if (key == "D") {
        moveAiPlaytestPlayer(session.registry, session.player, 1.0f, 0.0f, kAiPlaytestMoveDt);
    } else if (key == "F") {
        auto& player_component = session.registry.get<PlayerComponent>(session.player);
        const bool inside =
            (session.registry.has<BuildingInteractionComponent>(session.player) &&
             session.registry.get<BuildingInteractionComponent>(session.player).inside_building) ||
            session.registry.has<TransitRideComponent>(session.player);
        if (player_component.carried_object != MAX_ENTITIES) {
            if (!inside &&
                !transformOverlapsSolid(session.registry,
                                        session.registry.get<TransformComponent>(session.player))) {
                session.registry.get<TransformComponent>(player_component.carried_object) =
                    session.registry.get<TransformComponent>(session.player);
                player_component.carried_object = MAX_ENTITIES;
            }
        } else if (playerCanTakeWorkplaceOutput(session.registry, session.player)) {
            takeWorkplaceOutput(session.registry, session.player);
        } else if (playerCanTakeSupplyObject(session.registry, session.player)) {
            takeSupplyObjectFromInterior(session.registry, session.player);
        } else {
            takeNearbyCarryableObject(session.registry,
                                      session.player,
                                      kAiPlaytestInteractionRangeWu);
        }
    } else if (key == "E") {
        if (playerInsideTransitInterior(session.registry, session.player)) {
            const auto& ride = session.registry.get<TransitRideComponent>(session.player);
            finishTransitRide(session.registry, session.player, !ride.doors_open);
        } else if (playerCanReturnSuspiciousWorkplaceOutput(session.registry, session.player)) {
            returnSuspiciousWorkplaceOutput(session.registry, session.player);
        } else if (playerCanHideSuspiciousItemInHousing(session.registry, session.player)) {
            hideSuspiciousItemInHousing(session.registry, session.player);
        } else if (playerCanImproveBuilding(session.registry, session.player)) {
            improveBuilding(session.registry, session.player);
        } else if (playerCanStoreSupplyAtShelter(session.registry, session.player)) {
            storeSupplyAtShelter(session.registry, session.player);
        } else if (playerCanStockWorkplaceBench(session.registry, session.player)) {
            stockWorkplaceBench(session.registry, session.player);
        } else if (playerCanWorkWorkplaceBench(session.registry, session.player)) {
            workWorkplaceBench(session.registry, session.player);
        } else if (playerCanBoardTransit(session.registry,
                                         session.player,
                                         kAiPlaytestInteractionRangeWu)) {
            enterTransitRide(session.registry,
                             session.player,
                             kAiPlaytestInteractionRangeWu,
                             session.config.transit_ride_seconds);
        } else {
            toggleAiPlaytestBuildingInteraction(session.registry, session.player);
        }
    } else if (key == "T") {
        const bool inside =
            (session.registry.has<BuildingInteractionComponent>(session.player) &&
             session.registry.get<BuildingInteractionComponent>(session.player).inside_building) ||
            session.registry.has<TransitRideComponent>(session.player);
        const Entity near_worker = nearestWorkerInRange(
            session.registry,
            session.registry.get<TransformComponent>(session.player),
            kAiPlaytestInteractionRangeWu);
        if (inside && playerInsideHousingInterior(session.registry, session.player)) {
            useLayLowInHousing(session.registry, session.player);
        } else if (near_worker != MAX_ENTITIES && !inside) {
            auto& actor = session.registry.get<FixedActorComponent>(near_worker);
            actor.acknowledged = !actor.acknowledged;
        }
    } else if (key == "SPACE") {
        performAiPlaytestInspection(session.registry, session.player);
    } else if (key == "G") {
        useInheritedGadgetSpoof(session.registry,
                                session.player,
                                kAiPlaytestInspectionRangeWu);
    } else if (key == "WAIT" || key == ".") {
    } else {
        handled = false;
    }

    if (handled) {
        advanceAiPlaytestSimulation(session.registry, kAiPlaytestStepDt);
        if (result) *result = "KEY " + key + " OK";
    } else if (result) {
        *result = "UNKNOWN KEY: " + raw_key;
    }
    return handled;
}

inline Entity aiFirstBuildingByRoleInCurrentDistrict(AiPlaytestSession& session,
                                                     MicroZoneRole role) {
    if (worldHasMultipleDistricts(session.registry)) {
        const Entity macro = macroForDistrictId(session.registry,
                                                playerCurrentDistrictId(session.registry,
                                                                        session.player));
        const Entity local = firstWorldBuilderBuildingByRoleInMacro(session.registry,
                                                                    macro,
                                                                    role);
        if (local != MAX_ENTITIES) {
            return local;
        }
    }
    return firstBuildingByRole(session.registry, role);
}

inline Entity aiFirstWorkerInCurrentDistrict(AiPlaytestSession& session) {
    auto workers = session.registry.view<FixedActorComponent, TransformComponent>();
    std::sort(workers.begin(), workers.end());
    const uint32_t district_id = playerCurrentDistrictId(session.registry, session.player);
    const Entity suspicious_worker =
        firstLocalSuspicionRecordWorkerInDistrict(session.registry, district_id);
    if (suspicious_worker != MAX_ENTITIES) {
        return suspicious_worker;
    }
    for (Entity worker : workers) {
        if (!worldHasMultipleDistricts(session.registry) ||
            districtIdForEntity(session.registry, worker) == district_id) {
            return worker;
        }
    }
    return workers.empty() ? MAX_ENTITIES : workers.front();
}

inline Entity aiFirstStationInCurrentDistrict(AiPlaytestSession& session) {
    auto stations = session.registry.view<StationComponent>();
    std::sort(stations.begin(), stations.end());
    const uint32_t district_id = playerCurrentDistrictId(session.registry, session.player);
    for (Entity station : stations) {
        if (!worldHasMultipleDistricts(session.registry) ||
            session.registry.get<StationComponent>(station).district_id == district_id) {
            return station;
        }
    }
    return stations.empty() ? MAX_ENTITIES : stations.front();
}

inline Entity aiFirstSignpostInCurrentDistrict(AiPlaytestSession& session) {
    auto signposts = session.registry.view<RouteSignpostComponent, TransformComponent>();
    std::sort(signposts.begin(), signposts.end());
    const uint32_t district_id = playerCurrentDistrictId(session.registry, session.player);
    for (Entity signpost : signposts) {
        if (!worldHasMultipleDistricts(session.registry) ||
            districtIdForEntity(session.registry, signpost) == district_id) {
            return signpost;
        }
    }
    return signposts.empty() ? MAX_ENTITIES : signposts.front();
}

inline bool warpAiPlaytestPlayer(AiPlaytestSession& session,
                                 const std::string& raw_target,
                                 std::string* result = nullptr) {
    std::string target = normalizeAiPlaytestKey(raw_target);
    Entity entity = MAX_ENTITIES;
    if (target == "HOUSING") {
        entity = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::HOUSING);
    } else if (target == "WORKPLACE") {
        entity = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::WORKPLACE);
    } else if (target == "SUPPLY") {
        entity = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::SUPPLY);
    } else if (target == "MARKET") {
        entity = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::MARKET);
    } else if (target == "CLINIC") {
        entity = aiFirstBuildingByRoleInCurrentDistrict(session, MicroZoneRole::CLINIC);
    } else if (target == "WORKER") {
        entity = aiFirstWorkerInCurrentDistrict(session);
    } else if (target == "CARRYABLE" || target == "OBJECT") {
        entity = firstCarryableObject(session.registry);
    } else if (target == "SIGNPOST") {
        entity = aiFirstSignpostInCurrentDistrict(session);
    } else if (target == "STATION" || target == "TRANSIT") {
        entity = aiFirstStationInCurrentDistrict(session);
    }

    if (entity == MAX_ENTITIES || !session.registry.has<TransformComponent>(entity)) {
        if (result) *result = "WARP FAILED: " + raw_target;
        return false;
    }
    if (session.registry.has<BuildingInteractionComponent>(session.player) &&
        session.registry.get<BuildingInteractionComponent>(session.player).inside_building) {
        exitBuildingInterior(session.registry, session.player);
    }
    if (session.registry.has<TransitRideComponent>(session.player)) {
        session.registry.remove<TransitRideComponent>(session.player);
    }
    TransformComponent destination = session.registry.get<TransformComponent>(entity);
    if (session.registry.has<BuildingUseComponent>(entity)) {
        destination.y += destination.height * 0.5f +
                         session.registry.get<TransformComponent>(session.player).height * 0.5f +
                         1.0f;
    }
    session.registry.get<TransformComponent>(session.player) = destination;
    if (result) *result = "WARP " + target + " OK";
    return true;
}

inline void aiPlaceMarker(std::vector<std::string>& map,
                          float x,
                          float y,
                          float min_x,
                          float min_y,
                          float scale,
                          char marker) {
    if (scale <= 0.0f) return;
    const int w = static_cast<int>(map.empty() ? 0 : map.front().size());
    const int h = static_cast<int>(map.size());
    const int gx = static_cast<int>(std::round((x - min_x) / scale));
    const int gy = static_cast<int>(std::round((y - min_y) / scale));
    if (gx >= 0 && gx < w && gy >= 0 && gy < h) {
        map[gy][gx] = marker;
    }
}

inline bool aiCellOverlapsTransform(float cell_x,
                                    float cell_y,
                                    float cell_size,
                                    const TransformComponent& transform) {
    const float half_cell = cell_size * 0.5f;
    const float cell_left = cell_x - half_cell;
    const float cell_right = cell_x + half_cell;
    const float cell_top = cell_y - half_cell;
    const float cell_bottom = cell_y + half_cell;
    const float left = transform.x - transform.width * 0.5f;
    const float right = transform.x + transform.width * 0.5f;
    const float top = transform.y - transform.height * 0.5f;
    const float bottom = transform.y + transform.height * 0.5f;
    return cell_left <= right && cell_right >= left &&
           cell_top <= bottom && cell_bottom >= top;
}

inline void aiPlaceViewCell(std::vector<std::string>& view,
                            std::vector<std::vector<int>>& priority,
                            int x,
                            int y,
                            char marker,
                            int marker_priority) {
    if (view.empty()) return;
    const int h = static_cast<int>(view.size());
    const int w = static_cast<int>(view.front().size());
    if (x < 0 || x >= w || y < 0 || y >= h) return;
    if (marker_priority >= priority[y][x]) {
        view[y][x] = marker;
        priority[y][x] = marker_priority;
    }
}

inline void aiPlaceViewPoint(std::vector<std::string>& view,
                             std::vector<std::vector<int>>& priority,
                             float world_x,
                             float world_y,
                             float origin_x,
                             float origin_y,
                             float cell_size,
                             char marker,
                             int marker_priority) {
    if (cell_size <= 0.0f) return;
    const int x = static_cast<int>(std::floor((world_x - origin_x) / cell_size));
    const int y = static_cast<int>(std::floor((world_y - origin_y) / cell_size));
    aiPlaceViewCell(view, priority, x, y, marker, marker_priority);
}

inline void aiPlaceViewTransform(std::vector<std::string>& view,
                                 std::vector<std::vector<int>>& priority,
                                 const TransformComponent& transform,
                                 float origin_x,
                                 float origin_y,
                                 float cell_size,
                                 char marker,
                                 int marker_priority) {
    if (view.empty() || cell_size <= 0.0f) return;
    const int h = static_cast<int>(view.size());
    const int w = static_cast<int>(view.front().size());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const float cell_x = origin_x + (static_cast<float>(x) + 0.5f) * cell_size;
            const float cell_y = origin_y + (static_cast<float>(y) + 0.5f) * cell_size;
            if (aiCellOverlapsTransform(cell_x, cell_y, cell_size, transform)) {
                aiPlaceViewCell(view, priority, x, y, marker, marker_priority);
            }
        }
    }
}

inline std::vector<std::string> aiPlaytestPlayerView(Registry& registry, Entity player) {
    constexpr int width = 33;
    constexpr int height = 17;
    constexpr float cell_size = 8.0f;
    std::vector<std::string> view(height, std::string(width, ' '));
    std::vector<std::vector<int>> priority(height, std::vector<int>(width, 0));

    if (!registry.alive(player) || !registry.has<TransformComponent>(player)) {
        return view;
    }

    const auto& player_transform = registry.get<TransformComponent>(player);
    const float origin_x = player_transform.x -
                           (static_cast<float>(width) * cell_size * 0.5f);
    const float origin_y = player_transform.y -
                           (static_cast<float>(height) * cell_size * 0.5f);

    auto paths = registry.view<PathComponent, TransformComponent>();
    for (Entity path : paths) {
        aiPlaceViewTransform(view,
                             priority,
                             registry.get<TransformComponent>(path),
                             origin_x,
                             origin_y,
                             cell_size,
                             '.',
                             10);
    }

    auto solids = registry.view<SolidComponent, TransformComponent>();
    for (Entity solid : solids) {
        if (!registry.get<SolidComponent>(solid).is_solid) continue;
        if (registry.has<BuildingUseComponent>(solid)) continue;
        aiPlaceViewTransform(view,
                             priority,
                             registry.get<TransformComponent>(solid),
                             origin_x,
                             origin_y,
                             cell_size,
                             '#',
                             20);
    }

    auto buildings = registry.view<BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        char marker = '?';
        switch (registry.get<BuildingUseComponent>(building).role) {
            case MicroZoneRole::HOUSING: marker = 'H'; break;
            case MicroZoneRole::WORKPLACE: marker = 'W'; break;
            case MicroZoneRole::SUPPLY: marker = 'S'; break;
            case MicroZoneRole::MARKET: marker = 'M'; break;
            case MicroZoneRole::CLINIC: marker = 'C'; break;
        }
        aiPlaceViewTransform(view,
                             priority,
                             registry.get<TransformComponent>(building),
                             origin_x,
                             origin_y,
                             cell_size,
                             marker,
                             30);
    }

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    for (Entity signpost : signposts) {
        const char marker = registry.get<RouteSignpostComponent>(signpost).spoofed ? '!' : '+';
        const auto& t = registry.get<TransformComponent>(signpost);
        aiPlaceViewPoint(view, priority, t.x, t.y, origin_x, origin_y, cell_size, marker, 50);
    }

    auto stations = registry.view<StationComponent, TransformComponent>();
    for (Entity station : stations) {
        const auto& t = registry.get<TransformComponent>(station);
        aiPlaceViewPoint(view, priority, t.x, t.y, origin_x, origin_y, cell_size, 'T', 55);
    }

    auto carryables = registry.view<CarryableComponent, TransformComponent>();
    for (Entity object : carryables) {
        if (carryableObjectIsHeld(registry, object)) continue;
        const auto& t = registry.get<TransformComponent>(object);
        if (std::fabs(t.x) > 10000.0f || std::fabs(t.y) > 10000.0f) continue;
        aiPlaceViewPoint(view, priority, t.x, t.y, origin_x, origin_y, cell_size, 'o', 60);
    }

    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity worker : workers) {
        const auto& t = registry.get<TransformComponent>(worker);
        aiPlaceViewPoint(view, priority, t.x, t.y, origin_x, origin_y, cell_size, 'w', 70);
    }

    const int player_x = width / 2;
    const int player_y = height / 2;
    int facing_x = player_x;
    int facing_y = player_y;
    char facing_marker = 'v';
    if (registry.has<PlayerComponent>(player)) {
        switch (registry.get<PlayerComponent>(player).facing) {
            case Facing::UP: --facing_y; facing_marker = '^'; break;
            case Facing::DOWN: ++facing_y; facing_marker = 'v'; break;
            case Facing::LEFT: --facing_x; facing_marker = '<'; break;
            case Facing::RIGHT: ++facing_x; facing_marker = '>'; break;
        }
    }
    if (facing_x >= 0 && facing_x < width &&
        facing_y >= 0 && facing_y < height &&
        priority[facing_y][facing_x] < 50) {
        aiPlaceViewCell(view, priority, facing_x, facing_y, facing_marker, 90);
    }
    aiPlaceViewCell(view, priority, player_x, player_y, '@', 100);
    return view;
}

inline std::vector<std::string> aiPlaytestMap(Registry& registry, Entity player) {
    constexpr int width = 49;
    constexpr int height = 21;
    std::vector<std::string> map(height, std::string(width, ' '));

    float min_x = 0.0f;
    float min_y = 0.0f;
    float max_x = 1.0f;
    float max_y = 1.0f;
    bool initialized = false;
    auto include_transform = [&](const TransformComponent& t) {
        const float left = t.x - t.width * 0.5f;
        const float right = t.x + t.width * 0.5f;
        const float top = t.y - t.height * 0.5f;
        const float bottom = t.y + t.height * 0.5f;
        if (!initialized) {
            min_x = left;
            max_x = right;
            min_y = top;
            max_y = bottom;
            initialized = true;
            return;
        }
        min_x = std::min(min_x, left);
        max_x = std::max(max_x, right);
        min_y = std::min(min_y, top);
        max_y = std::max(max_y, bottom);
    };

    auto transforms = registry.view<TransformComponent>();
    for (Entity entity : transforms) {
        if (registry.has<CarryableComponent>(entity) &&
            carryableObjectIsHeld(registry, entity)) {
            continue;
        }
        const auto& transform = registry.get<TransformComponent>(entity);
        if (std::fabs(transform.x) > 10000.0f || std::fabs(transform.y) > 10000.0f) {
            continue;
        }
        include_transform(transform);
    }
    if (registry.alive(player) && registry.has<TransformComponent>(player)) {
        include_transform(registry.get<TransformComponent>(player));
    }
    min_x -= 12.0f;
    max_x += 12.0f;
    min_y -= 12.0f;
    max_y += 12.0f;
    const float scale = std::max((max_x - min_x) / static_cast<float>(width - 1),
                                 (max_y - min_y) / static_cast<float>(height - 1));

    auto paths = registry.view<PathComponent, TransformComponent>();
    for (Entity path : paths) {
        const auto& t = registry.get<TransformComponent>(path);
        const int samples = std::max(2, static_cast<int>(std::max(t.width, t.height) / 12.0f));
        for (int i = 0; i <= samples; ++i) {
            const float u = samples == 0 ? 0.0f : static_cast<float>(i) / static_cast<float>(samples);
            const float x = t.x - t.width * 0.5f + t.width * u;
            const float y = t.y - t.height * 0.5f + t.height * u;
            aiPlaceMarker(map, x, y, min_x, min_y, scale, '.');
        }
    }

    auto buildings = registry.view<BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        char marker = '?';
        switch (registry.get<BuildingUseComponent>(building).role) {
            case MicroZoneRole::HOUSING: marker = 'H'; break;
            case MicroZoneRole::WORKPLACE: marker = 'W'; break;
            case MicroZoneRole::SUPPLY: marker = 'S'; break;
            case MicroZoneRole::MARKET: marker = 'M'; break;
            case MicroZoneRole::CLINIC: marker = 'C'; break;
        }
        const auto& t = registry.get<TransformComponent>(building);
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, marker);
    }

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    for (Entity signpost : signposts) {
        const auto& t = registry.get<TransformComponent>(signpost);
        const char marker = registry.get<RouteSignpostComponent>(signpost).spoofed ? '!' : '+';
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, marker);
    }
    auto stations = registry.view<StationComponent, TransformComponent>();
    for (Entity station : stations) {
        const auto& t = registry.get<TransformComponent>(station);
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, 'T');
    }
    auto carryables = registry.view<CarryableComponent, TransformComponent>();
    for (Entity object : carryables) {
        if (carryableObjectIsHeld(registry, object)) continue;
        const auto& t = registry.get<TransformComponent>(object);
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, 'o');
    }
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    for (Entity worker : workers) {
        const auto& t = registry.get<TransformComponent>(worker);
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, 'w');
    }
    if (registry.alive(player) && registry.has<TransformComponent>(player)) {
        const auto& t = registry.get<TransformComponent>(player);
        aiPlaceMarker(map, t.x, t.y, min_x, min_y, scale, '@');
    }
    return map;
}

inline std::string aiPlaytestActionLine(Registry& registry, Entity player) {
    const PlayerLocationState location =
        playerLocationState(registry, player, kAiPlaytestInteractionRangeWu);
    const bool inside =
        (registry.has<BuildingInteractionComponent>(player) &&
         registry.get<BuildingInteractionComponent>(player).inside_building) ||
        registry.has<TransitRideComponent>(player);
    const Entity near_worker = nearestWorkerInRange(
        registry,
        registry.get<TransformComponent>(player),
        kAiPlaytestInteractionRangeWu);
    const Entity near_carryable = nearestCarryableObjectInRange(
        registry,
        registry.get<TransformComponent>(player),
        kAiPlaytestInteractionRangeWu);
    const auto& player_component = registry.get<PlayerComponent>(player);

    std::ostringstream out;
    out << "LOCATION:" << aiLocationStateName(location) << " ";
    if (playerCanReturnSuspiciousWorkplaceOutput(registry, player)) {
        out << "E RETURN SUSPECT PART " << workplaceBenchReadout(registry);
    } else if (playerInsideTransitInterior(registry, player)) {
        const auto& ride = registry.get<TransitRideComponent>(player);
        out << (ride.doors_open ? "E EXIT TRANSIT " : "E LOOK OUT WINDOW ")
            << transitRideReadout(registry, player);
    } else if (playerCanLayLowInHousing(registry, player)) {
        out << "T LAY LOW " << shelterSupplyReadoutForPlayer(registry, player);
    } else if (playerCanHideSuspiciousItemInHousing(registry, player)) {
        out << "E HIDE SUSPECT PART";
    } else if (playerCanImproveBuilding(registry, player)) {
        out << "E IMPROVE BUILDING " << buildingImprovementReadout(registry);
    } else if (playerCanStoreSupplyAtShelter(registry, player)) {
        out << "E STORE SUPPLY " << shelterSupplyReadoutForPlayer(registry, player);
    } else if (playerCanStockWorkplaceBench(registry, player)) {
        out << "E STOCK BENCH " << workplaceBenchReadout(registry);
    } else if (playerCanWorkWorkplaceBench(registry, player)) {
        out << "E WORK BENCH " << workplaceBenchReadout(registry);
    } else if (playerCanBoardTransit(registry, player, kAiPlaytestInteractionRangeWu)) {
        out << "E BOARD TRANSIT";
    } else if (player_component.carried_object != MAX_ENTITIES) {
        out << aiLocationPrompt(location) << " F DROP "
            << carryableObjectLabel(registry, player_component.carried_object);
    } else if (playerCanTakeSupplyObject(registry, player)) {
        out << aiLocationPrompt(location) << " F TAKE SUPPLY";
    } else if (playerCanTakeWorkplaceOutput(registry, player)) {
        out << aiLocationPrompt(location) << " F TAKE PART";
    } else if (near_carryable != MAX_ENTITIES && !inside) {
        out << aiLocationPrompt(location) << " F PICK UP "
            << carryableObjectLabel(registry, near_carryable);
    } else if (near_worker != MAX_ENTITIES && !inside) {
        const bool ack = registry.get<FixedActorComponent>(near_worker).acknowledged;
        out << aiLocationPrompt(location) << " "
            << (ack ? "T DISMISS WORKER [WORKER ACKNOWLEDGED]" : "T TALK WORKER");
    } else {
        out << aiLocationPrompt(location);
    }
    out << " [CARRIED:";
    if (player_component.carried_object != MAX_ENTITIES) {
        out << carryableObjectLabel(registry, player_component.carried_object);
    } else {
        out << "NONE";
    }
    out << "]";
    return out.str();
}

inline std::string aiPlaytestGadgetResultReadout(Registry& registry, Entity player) {
    if (!playerHasInheritedGadget(registry, player)) {
        return "DEBUGGER RESULT: UNAVAILABLE";
    }

    const auto& gadget = registry.get<InheritedGadgetComponent>(player);
    if (gadget.last_result.empty()) {
        return "DEBUGGER RESULT: IDLE";
    }

    const char* label = "DEBUGGER RESULT: ";
    if (gadget.last_result_kind == InheritedGadgetResultKind::INTERFERENCE_TORCH) {
        label = "INTERFERENCE TORCH RESULT: ";
    } else if (gadget.last_result_kind == InheritedGadgetResultKind::ACTION) {
        label = "ACTION RESULT: ";
    }
    std::string result = std::string(label);
    if (gadget.last_result_target_entity != MAX_ENTITIES) {
        result += "ON ";
        result += inheritedGadgetTargetLabel(gadget.last_result_target_type);
        result += " entity=" + std::to_string(gadget.last_result_target_entity) + ": ";
    }
    result += gadget.last_result;
    return result;
}

inline std::string aiPlaytestSnapshot(Registry& registry, Entity player) {
    std::ostringstream out;
    const auto& player_transform = registry.get<TransformComponent>(player);
    const auto& player_component = registry.get<PlayerComponent>(player);
    const PlayerLocationState location =
        playerLocationState(registry, player, kAiPlaytestInteractionRangeWu);
    const bool inside =
        (registry.has<BuildingInteractionComponent>(player) &&
         registry.get<BuildingInteractionComponent>(player).inside_building) ||
        registry.has<TransitRideComponent>(player);
    const InspectionTarget target =
        playerInspectionTarget(registry, player, kAiPlaytestInspectionRangeWu);

    out << "=== NEON AI PLAYTEST ===\n";
    out << "COMMANDS: snapshot | key W/A/S/D/E/F/T/SPACE/G | step N | reset [default|suspicion] | warp TARGET\n";
    out << "TARGETS: HOUSING WORKPLACE SUPPLY MARKET CLINIC STATION WORKER SIGNPOST CARRYABLE\n";
    out << "PLAYER: x=" << player_transform.x
        << " y=" << player_transform.y
        << " facing=" << aiFacingName(player_component.facing)
        << " location=\"" << aiLocationStateName(location) << "\""
        << " inside=" << (inside ? 1 : 0)
        << " carrying="
        << (player_component.carried_object != MAX_ENTITIES
                ? carryableObjectLabel(registry, player_component.carried_object)
                : "NONE")
        << " " << playerDistrictReadout(registry, player)
        << "\n";
    if (playerInsideTransitInterior(registry, player)) {
        out << "TRANSIT: " << transitRideReadout(registry, player) << "\n";
    }
    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        const auto& interaction = registry.get<BuildingInteractionComponent>(player);
        out << "INTERIOR: role=" << roleDisplayName(interaction.building_role)
            << " local_x=" << interaction.interior_position.x
            << " local_y=" << interaction.interior_position.y << "\n";
    }
    out << "ACTION: " << aiPlaytestActionLine(registry, player) << "\n";
    out << "GADGET: " << inheritedGadgetReadout(registry, player)
        << " | " << inheritedGadgetPromptReadout(registry, player, kAiPlaytestInspectionRangeWu)
        << " | " << inheritedGadgetSpoofPromptReadout(registry, player, kAiPlaytestInspectionRangeWu)
        << "\n";
    out << "TARGET: " << aiInspectionTargetName(target.type)
        << " entity=" << target.entity << "\n";
    out << "TARGET_DETAIL: " << worldPhaseReadout(registry) << "; "
        << aiInspectionDetail(registry, target) << "\n";
    out << "TARGET_DEBUGGER_SCAN: " << inheritedGadgetScanResult(registry, target) << "\n";
    out << "DEBUGGER_RESULT: " << aiPlaytestGadgetResultReadout(registry, player) << "\n";

    const std::string local_notice = localSuspicionHudReadout(registry);
    out << "SYSTEMS: " << productionLoopSummaryReadout(registry)
        << " | " << playerDistrictReadout(registry, player)
        << " | " << worldPhaseReadout(registry)
        << " | " << shelterSupplyReadoutForPlayer(registry, player)
        << " | " << workplaceBenchReadout(registry)
        << " | " << buildingImprovementReadout(registry);
    if (!local_notice.empty()) {
        out << " | " << local_notice;
    }
    out << "\n";

    out << "-- PLAYER VIEW 33x17 CELL=8WU CENTERED ON @ "
        << worldPhaseReadout(registry) << " --\n";
    out << "LEGEND: @ player ^v<> facing H housing W workplace S supply M market C clinic "
        << "T transit # solid . path w worker + signpost ! spoofed o object\n";
    for (const std::string& row : aiPlaytestPlayerView(registry, player)) {
        out << row << "\n";
    }

    out << "-- NEARBY --\n";
    auto transforms = registry.view<TransformComponent>();
    std::vector<std::pair<float, Entity>> nearby;
    for (Entity entity : transforms) {
        if (entity == player) continue;
        const float distance = aabbDistance(player_transform, registry.get<TransformComponent>(entity));
        if (distance <= 48.0f) {
            nearby.push_back({distance, entity});
        }
    }
    std::sort(nearby.begin(), nearby.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    for (const auto& [distance, entity] : nearby) {
        InspectionTargetType type = InspectionTargetType::NONE;
        if (registry.has<BuildingUseComponent>(entity)) {
            type = inspectionTypeForRole(registry.get<BuildingUseComponent>(entity).role);
        } else if (registry.has<FixedActorComponent>(entity)) {
            type = InspectionTargetType::WORKER;
        } else if (registry.has<RouteSignpostComponent>(entity)) {
            type = InspectionTargetType::ROUTE_SIGNPOST;
        } else if (registry.has<PathComponent>(entity)) {
            type = InspectionTargetType::PEDESTRIAN_PATH;
        } else if (registry.has<CarryableComponent>(entity)) {
            type = InspectionTargetType::CARRYABLE_OBJECT;
        }
        if (type == InspectionTargetType::NONE) continue;
        out << "- " << aiInspectionTargetName(type)
            << " entity=" << entity
            << " distance=" << distance << "\n";
    }
    return out.str();
}

inline std::string aiPlaytestSnapshot(const AiPlaytestSession& session) {
    return aiPlaytestSnapshot(const_cast<Registry&>(session.registry), session.player);
}

inline bool parseAiPlaytestScenario(const std::string& raw, AiPlaytestScenario& scenario) {
    const std::string value = normalizeAiPlaytestKey(raw);
    if (value.empty() || value == "DEFAULT") {
        scenario = AiPlaytestScenario::DEFAULT;
        return true;
    }
    if (value == "SUSPICION") {
        scenario = AiPlaytestScenario::SUSPICION;
        return true;
    }
    return false;
}
