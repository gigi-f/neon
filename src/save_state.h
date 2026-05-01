#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "components.h"
#include "ecs.h"
#include "fixed_actor_system.h"
#include "world_builder.h"

enum class TinySaveStatus {
    OK,
    MISSING_FILE,
    INVALID_FORMAT,
    INVALID_WORLD
};

struct SavedTransform {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct SavedSpoofedSignpost {
    uint64_t path_from_stable_id = 0;
    uint64_t path_to_stable_id = 0;
    uint64_t endpoint_stable_id = 0;
};

struct SavedDependencyDisruption {
    MicroZoneRole dependent_role = MicroZoneRole::WORKPLACE;
    MicroZoneRole provider_role = MicroZoneRole::SUPPLY;
    uint32_t district_id = 0;
    bool disrupted = false;
};

struct SavedLocalSuspicion {
    bool present = false;
    bool active = false;
    LocalSuspicionCause cause = LocalSuspicionCause::NONE;
    LocalSuspicionResolution resolution = LocalSuspicionResolution::NONE;
    uint64_t workplace_stable_id = 0;
    bool target_is_carryable = false;
    uint64_t target_path_from_stable_id = 0;
    uint64_t target_path_to_stable_id = 0;
    uint64_t target_endpoint_stable_id = 0;
    uint64_t path_from_stable_id = 0;
    uint64_t path_to_stable_id = 0;
    uint64_t resolution_stable_id = 0;
    bool institutional_log_recovered = false;
};

struct SavedWorker {
    bool present = false;
    uint64_t path_from_stable_id = 0;
    uint64_t path_to_stable_id = 0;
    float route_t = 0.0f;
    float direction = 1.0f;
    bool acknowledged = false;
    bool carrying = false;
    bool wage_record_spoofed = false;
};

struct SavedWorkerSuspicion {
    size_t worker_index = 0;
    SavedLocalSuspicion suspicion{};
};

struct TinySaveState {
    SavedTransform player{};
    bool player_carrying = false;

    bool inside_building = false;
    MicroZoneRole building_role = MicroZoneRole::HOUSING;
    uint64_t building_stable_id = 0;
    SavedTransform exterior_position{};
    SavedTransform interior_position{};

    bool has_carryable = false;
    ItemKind carryable_kind = ItemKind::SUPPLY;
    SavedTransform carryable_position{};

    int shelter_supply = 0;
    WorkplaceBenchState workplace_bench_state = WorkplaceBenchState::EMPTY;
    bool building_improved = false;

    bool has_worker = false;
    uint64_t worker_path_from_stable_id = 0;
    uint64_t worker_path_to_stable_id = 0;
    float worker_route_t = 0.0f;
    float worker_direction = 1.0f;
    bool worker_acknowledged = false;
    bool worker_carrying = false;
    bool worker_wage_record_spoofed = false;
    std::vector<SavedWorker> workers;

    std::vector<SavedSpoofedSignpost> spoofed_signposts;
    std::vector<SavedDependencyDisruption> dependency_disruptions;
    SavedLocalSuspicion local_suspicion{};
    std::vector<SavedWorkerSuspicion> local_suspicions;

    WorldPhase world_phase = WorldPhase::DAY;
    float world_phase_elapsed_seconds = 0.0f;
    float world_phase_interval_seconds = 240.0f;

    bool in_transit = false;
    uint64_t transit_origin_station_stable_id = 0;
    uint64_t transit_destination_station_stable_id = 0;
    float transit_elapsed_seconds = 0.0f;
    float transit_stop_interval_seconds = 4.0f;
    bool transit_doors_open = false;
    SavedTransform transit_exterior_position{};
    SavedTransform transit_interior_position{};
};

inline SavedTransform savedTransformFromTransform(const TransformComponent& transform) {
    return SavedTransform{transform.x, transform.y, transform.width, transform.height};
}

inline TransformComponent transformFromSavedTransform(const SavedTransform& transform) {
    return TransformComponent{transform.x, transform.y, transform.width, transform.height};
}

inline const char* roleSaveName(MicroZoneRole role) {
    switch (role) {
        case MicroZoneRole::HOUSING: return "HOUSING";
        case MicroZoneRole::WORKPLACE: return "WORKPLACE";
        case MicroZoneRole::SUPPLY: return "SUPPLY";
        case MicroZoneRole::MARKET: return "MARKET";
        case MicroZoneRole::CLINIC: return "CLINIC";
    }
    return "HOUSING";
}

inline bool roleFromSaveName(const std::string& name, MicroZoneRole& role) {
    if (name == "HOUSING") {
        role = MicroZoneRole::HOUSING;
        return true;
    }
    if (name == "WORKPLACE") {
        role = MicroZoneRole::WORKPLACE;
        return true;
    }
    if (name == "SUPPLY") {
        role = MicroZoneRole::SUPPLY;
        return true;
    }
    if (name == "MARKET") {
        role = MicroZoneRole::MARKET;
        return true;
    }
    if (name == "CLINIC") {
        role = MicroZoneRole::CLINIC;
        return true;
    }
    return false;
}

inline const char* localSuspicionCauseSaveName(LocalSuspicionCause cause) {
    switch (cause) {
        case LocalSuspicionCause::NONE: return "NONE";
        case LocalSuspicionCause::MISSING_PART: return "MISSING_PART";
        case LocalSuspicionCause::ROUTE_TAMPERING: return "ROUTE_TAMPERING";
    }
    return "NONE";
}

inline bool localSuspicionCauseFromSaveName(const std::string& name,
                                            LocalSuspicionCause& cause) {
    if (name == "NONE") {
        cause = LocalSuspicionCause::NONE;
        return true;
    }
    if (name == "MISSING_PART") {
        cause = LocalSuspicionCause::MISSING_PART;
        return true;
    }
    if (name == "ROUTE_TAMPERING") {
        cause = LocalSuspicionCause::ROUTE_TAMPERING;
        return true;
    }
    return false;
}

inline const char* localSuspicionResolutionSaveName(LocalSuspicionResolution resolution) {
    switch (resolution) {
        case LocalSuspicionResolution::NONE: return "NONE";
        case LocalSuspicionResolution::RETURNED_OUTPUT: return "RETURNED_OUTPUT";
        case LocalSuspicionResolution::CORRECTED_ROUTE: return "CORRECTED_ROUTE";
        case LocalSuspicionResolution::HIDDEN_ITEM: return "HIDDEN_ITEM";
        case LocalSuspicionResolution::LAID_LOW: return "LAID_LOW";
    }
    return "NONE";
}

inline bool localSuspicionResolutionFromSaveName(const std::string& name,
                                                 LocalSuspicionResolution& resolution) {
    if (name == "NONE") {
        resolution = LocalSuspicionResolution::NONE;
        return true;
    }
    if (name == "RETURNED_OUTPUT") {
        resolution = LocalSuspicionResolution::RETURNED_OUTPUT;
        return true;
    }
    if (name == "CORRECTED_ROUTE") {
        resolution = LocalSuspicionResolution::CORRECTED_ROUTE;
        return true;
    }
    if (name == "HIDDEN_ITEM") {
        resolution = LocalSuspicionResolution::HIDDEN_ITEM;
        return true;
    }
    if (name == "LAID_LOW") {
        resolution = LocalSuspicionResolution::LAID_LOW;
        return true;
    }
    return false;
}

inline const char* worldPhaseSaveName(WorldPhase phase) {
    switch (phase) {
        case WorldPhase::DAY: return "DAY";
        case WorldPhase::NIGHT: return "NIGHT";
    }
    return "DAY";
}

inline bool worldPhaseFromSaveName(const std::string& name, WorldPhase& phase) {
    if (name == "DAY") {
        phase = WorldPhase::DAY;
        return true;
    }
    if (name == "NIGHT") {
        phase = WorldPhase::NIGHT;
        return true;
    }
    return false;
}

inline Entity buildingByStableId(Registry& registry, uint64_t stable_id) {
    if (stable_id == 0) return MAX_ENTITIES;

    auto buildings = registry.view<BuildingComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingComponent>(building).stable_id == stable_id) {
            return building;
        }
    }
    return MAX_ENTITIES;
}

inline Entity firstShelterStockEntity(Registry& registry) {
    auto shelters = registry.view<ShelterStockComponent, BuildingUseComponent>();
    for (Entity shelter : shelters) {
        if (registry.get<BuildingUseComponent>(shelter).role == MicroZoneRole::HOUSING) {
            return shelter;
        }
    }
    return MAX_ENTITIES;
}

inline Entity firstWorkplaceBenchEntity(Registry& registry) {
    auto benches = registry.view<WorkplaceBenchComponent, BuildingUseComponent>();
    for (Entity workplace : benches) {
        if (registry.get<BuildingUseComponent>(workplace).role == MicroZoneRole::WORKPLACE) {
            return workplace;
        }
    }
    return MAX_ENTITIES;
}

inline Entity firstFixedWorker(Registry& registry) {
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    return workers.empty() ? MAX_ENTITIES : workers.front();
}

inline std::vector<Entity> fixedWorkersInSaveOrder(Registry& registry) {
    auto workers = registry.view<FixedActorComponent, TransformComponent>();
    std::vector<Entity> ordered(workers.begin(), workers.end());
    std::sort(ordered.begin(), ordered.end());
    return ordered;
}

inline uint64_t stableIdForEntity(Registry& registry, Entity entity) {
    if (!registry.alive(entity) || !registry.has<BuildingComponent>(entity)) {
        return 0;
    }
    return registry.get<BuildingComponent>(entity).stable_id;
}

inline Entity pedestrianPathByEndpointStableIds(Registry& registry,
                                                uint64_t from_stable_id,
                                                uint64_t to_stable_id) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        const auto& path = registry.get<PathComponent>(path_entity);
        if (path.kind != PathKind::PEDESTRIAN) continue;

        const uint64_t from = stableIdForEntity(registry, path.from);
        const uint64_t to = stableIdForEntity(registry, path.to);
        const bool same_direction = from == from_stable_id && to == to_stable_id;
        const bool reverse_direction = from == to_stable_id && to == from_stable_id;
        if (same_direction || reverse_direction) {
            return path_entity;
        }
    }
    return MAX_ENTITIES;
}

inline Entity routeSignpostByStableIds(Registry& registry,
                                       uint64_t path_from_stable_id,
                                       uint64_t path_to_stable_id,
                                       uint64_t endpoint_stable_id) {
    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        const auto& signpost = registry.get<RouteSignpostComponent>(marker);
        if (!registry.alive(signpost.path_entity) ||
            !registry.has<PathComponent>(signpost.path_entity)) {
            continue;
        }

        const auto& path = registry.get<PathComponent>(signpost.path_entity);
        const uint64_t from = stableIdForEntity(registry, path.from);
        const uint64_t to = stableIdForEntity(registry, path.to);
        const bool same_direction = from == path_from_stable_id && to == path_to_stable_id;
        const bool reverse_direction = from == path_to_stable_id && to == path_from_stable_id;
        if ((same_direction || reverse_direction) &&
            stableIdForEntity(registry, signpost.endpoint_entity) == endpoint_stable_id) {
            return marker;
        }
    }
    return MAX_ENTITIES;
}

inline TinySaveState captureTinySaveState(Registry& registry, Entity player) {
    TinySaveState state;
    if (!registry.alive(player) ||
        !registry.has<PlayerComponent>(player) ||
        !registry.has<TransformComponent>(player)) {
        return state;
    }

    state.player = savedTransformFromTransform(registry.get<TransformComponent>(player));
    const auto& player_component = registry.get<PlayerComponent>(player);
    state.player_carrying = player_component.carried_object != MAX_ENTITIES;

    if (registry.has<BuildingInteractionComponent>(player)) {
        const auto& interaction = registry.get<BuildingInteractionComponent>(player);
        state.inside_building = interaction.inside_building;
        state.building_role = interaction.building_role;
        state.building_stable_id = stableIdForEntity(registry, interaction.building_entity);
        state.exterior_position = savedTransformFromTransform(interaction.exterior_position);
        state.interior_position = savedTransformFromTransform(interaction.interior_position);
    }

    const Entity carryable = firstCarryableObject(registry);
    if (carryable != MAX_ENTITIES) {
        state.has_carryable = true;
        state.carryable_kind = registry.get<CarryableComponent>(carryable).kind;
        state.carryable_position = savedTransformFromTransform(registry.get<TransformComponent>(carryable));
    }

    const Entity shelter = firstShelterStockEntity(registry);
    if (shelter != MAX_ENTITIES) {
        const auto& stock = registry.get<ShelterStockComponent>(shelter);
        state.shelter_supply = std::clamp(stock.current_supply, 0, stock.capacity);
    }

    const Entity workplace = firstWorkplaceBenchEntity(registry);
    if (workplace != MAX_ENTITIES) {
        state.workplace_bench_state = registry.get<WorkplaceBenchComponent>(workplace).state;
    }

    const Entity improved_building = firstBuildingImprovementBuilding(registry);
    if (improved_building != MAX_ENTITIES) {
        state.building_improved =
            registry.get<BuildingImprovementComponent>(improved_building).improved;
    }

    const Entity phase_entity = worldPhaseEntity(registry);
    if (phase_entity != MAX_ENTITIES) {
        const auto& phase = registry.get<WorldPhaseComponent>(phase_entity);
        state.world_phase = phase.phase;
        state.world_phase_elapsed_seconds = std::max(0.0f, phase.elapsed_seconds);
        state.world_phase_interval_seconds = std::max(1.0f, phase.interval_seconds);
    }

    if (registry.has<TransitRideComponent>(player)) {
        const auto& ride = registry.get<TransitRideComponent>(player);
        state.in_transit = true;
        if (registry.alive(ride.origin_station) &&
            registry.has<StationComponent>(ride.origin_station)) {
            state.transit_origin_station_stable_id =
                registry.get<StationComponent>(ride.origin_station).stable_id;
        }
        if (registry.alive(ride.destination_station) &&
            registry.has<StationComponent>(ride.destination_station)) {
            state.transit_destination_station_stable_id =
                registry.get<StationComponent>(ride.destination_station).stable_id;
        }
        state.transit_elapsed_seconds = std::max(0.0f, ride.elapsed_seconds);
        state.transit_stop_interval_seconds = std::max(1.0f, ride.stop_interval_seconds);
        state.transit_doors_open = ride.doors_open;
        state.transit_exterior_position = savedTransformFromTransform(ride.exterior_position);
        state.transit_interior_position = savedTransformFromTransform(ride.interior_position);
    }

    const std::vector<Entity> workers = fixedWorkersInSaveOrder(registry);
    for (Entity worker : workers) {
        const auto& worker_component = registry.get<FixedActorComponent>(worker);
        SavedWorker saved_worker;
        saved_worker.present = true;
        saved_worker.route_t = std::clamp(worker_component.route_t, 0.0f, 1.0f);
        if (std::fabs(worker_component.direction) <= 0.001f) {
            saved_worker.direction = 0.0f;
        } else {
            saved_worker.direction = worker_component.direction > 0.0f ? 1.0f : -1.0f;
        }
        saved_worker.acknowledged = worker_component.acknowledged;
        saved_worker.carrying = worker_component.carried_object != MAX_ENTITIES;
        saved_worker.wage_record_spoofed = worker_component.wage_record_spoofed;
        if (registry.alive(worker_component.path_entity) &&
            registry.has<PathComponent>(worker_component.path_entity)) {
            const auto& path = registry.get<PathComponent>(worker_component.path_entity);
            saved_worker.path_from_stable_id = stableIdForEntity(registry, path.from);
            saved_worker.path_to_stable_id = stableIdForEntity(registry, path.to);
        }
        state.workers.push_back(saved_worker);
        if (!state.has_worker) {
            state.has_worker = true;
            state.worker_route_t = saved_worker.route_t;
            state.worker_direction = saved_worker.direction;
            state.worker_acknowledged = saved_worker.acknowledged;
            state.worker_carrying = saved_worker.carrying;
            state.worker_wage_record_spoofed = saved_worker.wage_record_spoofed;
            state.worker_path_from_stable_id = saved_worker.path_from_stable_id;
            state.worker_path_to_stable_id = saved_worker.path_to_stable_id;
        }
    }

    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        const auto& signpost = registry.get<RouteSignpostComponent>(marker);
        if (!signpost.spoofed ||
            !registry.alive(signpost.path_entity) ||
            !registry.has<PathComponent>(signpost.path_entity)) {
            continue;
        }

        const auto& path = registry.get<PathComponent>(signpost.path_entity);
        state.spoofed_signposts.push_back(SavedSpoofedSignpost{
            stableIdForEntity(registry, path.from),
            stableIdForEntity(registry, path.to),
            stableIdForEntity(registry, signpost.endpoint_entity)});
    }

    auto disruptions = registry.view<DependencyDisruptionComponent>();
    for (Entity disruption : disruptions) {
        const auto& component = registry.get<DependencyDisruptionComponent>(disruption);
        if (!component.disrupted) {
            continue;
        }
        state.dependency_disruptions.push_back(SavedDependencyDisruption{
            component.dependent_role,
            component.provider_role,
            component.district_id,
            component.disrupted});
    }

    for (size_t worker_index = 0; worker_index < workers.size(); ++worker_index) {
        const Entity suspicion_worker = workers[worker_index];
        if (!registry.has<LocalSuspicionComponent>(suspicion_worker)) {
            continue;
        }
        const auto& suspicion = registry.get<LocalSuspicionComponent>(suspicion_worker);
        if (!localSuspicionRecordExists(suspicion)) {
            continue;
        }
        SavedWorkerSuspicion saved_record;
        saved_record.worker_index = worker_index;
        auto& saved_suspicion = saved_record.suspicion;
        saved_suspicion.present = true;
        saved_suspicion.active = localSuspicionImmediateActive(suspicion);
        saved_suspicion.cause = suspicion.cause;
        saved_suspicion.resolution = suspicion.resolution;
        saved_suspicion.workplace_stable_id =
            stableIdForEntity(registry, suspicion.workplace_entity);
        saved_suspicion.resolution_stable_id =
            stableIdForEntity(registry, suspicion.resolution_entity);
        saved_suspicion.institutional_log_recovered =
            suspicion.institutional_log_recovered;
        if (suspicion.target_entity != MAX_ENTITIES &&
            registry.alive(suspicion.target_entity) &&
            registry.has<CarryableComponent>(suspicion.target_entity)) {
            saved_suspicion.target_is_carryable = true;
        } else if (suspicion.target_entity != MAX_ENTITIES &&
                   registry.alive(suspicion.target_entity) &&
                   registry.has<RouteSignpostComponent>(suspicion.target_entity)) {
            const auto& signpost =
                registry.get<RouteSignpostComponent>(suspicion.target_entity);
            if (registry.alive(signpost.path_entity) &&
                registry.has<PathComponent>(signpost.path_entity)) {
                const auto& path = registry.get<PathComponent>(signpost.path_entity);
                saved_suspicion.target_path_from_stable_id =
                    stableIdForEntity(registry, path.from);
                saved_suspicion.target_path_to_stable_id =
                    stableIdForEntity(registry, path.to);
                saved_suspicion.target_endpoint_stable_id =
                    stableIdForEntity(registry, signpost.endpoint_entity);
            }
        }
        if (suspicion.path_entity != MAX_ENTITIES &&
            registry.alive(suspicion.path_entity) &&
            registry.has<PathComponent>(suspicion.path_entity)) {
            const auto& path = registry.get<PathComponent>(suspicion.path_entity);
            saved_suspicion.path_from_stable_id =
                stableIdForEntity(registry, path.from);
            saved_suspicion.path_to_stable_id =
                stableIdForEntity(registry, path.to);
        }
        if (!state.local_suspicion.present) {
            state.local_suspicion = saved_suspicion;
        }
        state.local_suspicions.push_back(saved_record);
    }

    return state;
}

inline void writeSavedTransform(std::ostream& out, const SavedTransform& transform) {
    out << transform.x << ' ' << transform.y << ' '
        << transform.width << ' ' << transform.height;
}

inline bool readSavedTransform(std::istream& in, SavedTransform& transform) {
    return static_cast<bool>(in >> transform.x >> transform.y >> transform.width >> transform.height);
}

inline std::string serializeTinySaveState(const TinySaveState& state) {
    std::ostringstream out;
    out << "NEON_TINY_SAVE_V12\n";
    out << "PLAYER ";
    writeSavedTransform(out, state.player);
    out << ' ' << (state.player_carrying ? 1 : 0) << "\n";

    out << "BUILDING "
        << (state.inside_building ? 1 : 0) << ' '
        << roleSaveName(state.building_role) << ' '
        << state.building_stable_id << ' ';
    writeSavedTransform(out, state.exterior_position);
    out << ' ';
    writeSavedTransform(out, state.interior_position);
    out << "\n";

    out << "CARRYABLE " << (state.has_carryable ? 1 : 0) << ' '
        << itemKindSaveName(state.carryable_kind) << ' ';
    writeSavedTransform(out, state.carryable_position);
    out << "\n";

    out << "SHELTER " << state.shelter_supply << "\n";
    out << "WORKPLACE_BENCH " << workplaceBenchStateSaveName(state.workplace_bench_state) << "\n";
    out << "BUILDING_IMPROVEMENT " << (state.building_improved ? 1 : 0) << "\n";

    out << "WORLD_PHASE "
        << worldPhaseSaveName(state.world_phase) << ' '
        << std::max(0.0f, state.world_phase_elapsed_seconds) << ' '
        << std::max(1.0f, state.world_phase_interval_seconds) << "\n";
    out << "TRANSIT "
        << (state.in_transit ? 1 : 0) << ' '
        << state.transit_origin_station_stable_id << ' '
        << state.transit_destination_station_stable_id << ' '
        << std::max(0.0f, state.transit_elapsed_seconds) << ' '
        << std::max(1.0f, state.transit_stop_interval_seconds) << ' '
        << (state.transit_doors_open ? 1 : 0) << ' ';
    writeSavedTransform(out, state.transit_exterior_position);
    out << ' ';
    writeSavedTransform(out, state.transit_interior_position);
    out << "\n";

    std::vector<SavedWorker> workers = state.workers;
    if (workers.empty() && state.has_worker) {
        workers.push_back(SavedWorker{
            state.has_worker,
            state.worker_path_from_stable_id,
            state.worker_path_to_stable_id,
            state.worker_route_t,
            state.worker_direction,
            state.worker_acknowledged,
            state.worker_carrying,
            state.worker_wage_record_spoofed});
    }
    out << "WORKERS " << workers.size() << "\n";
    for (const auto& worker : workers) {
        out << "WORKER "
            << (worker.present ? 1 : 0) << ' '
            << worker.path_from_stable_id << ' '
            << worker.path_to_stable_id << ' '
            << worker.route_t << ' '
            << worker.direction << ' '
            << (worker.acknowledged ? 1 : 0) << ' '
            << (worker.carrying ? 1 : 0) << ' '
            << (worker.wage_record_spoofed ? 1 : 0) << "\n";
    }
    out << "SPOOFED_SIGNPOSTS " << state.spoofed_signposts.size() << "\n";
    for (const auto& signpost : state.spoofed_signposts) {
        out << "SPOOFED_SIGNPOST "
            << signpost.path_from_stable_id << ' '
            << signpost.path_to_stable_id << ' '
            << signpost.endpoint_stable_id << "\n";
    }
    out << "DEPENDENCY_DISRUPTIONS " << state.dependency_disruptions.size() << "\n";
    for (const auto& disruption : state.dependency_disruptions) {
        out << "DEPENDENCY_DISRUPTION "
            << roleSaveName(disruption.dependent_role) << ' '
            << roleSaveName(disruption.provider_role) << ' '
            << disruption.district_id << ' '
            << (disruption.disrupted ? 1 : 0) << "\n";
    }
    const std::vector<SavedWorkerSuspicion> suspicions = !state.local_suspicions.empty()
        ? state.local_suspicions
        : (state.local_suspicion.present
              ? std::vector<SavedWorkerSuspicion>{SavedWorkerSuspicion{0, state.local_suspicion}}
              : std::vector<SavedWorkerSuspicion>{});
    out << "LOCAL_SUSPICIONS " << suspicions.size() << "\n";
    for (const auto& record : suspicions) {
        const auto& suspicion = record.suspicion;
        out << "LOCAL_SUSPICION "
            << record.worker_index << ' '
            << (suspicion.present ? 1 : 0) << ' '
            << (suspicion.active ? 1 : 0) << ' '
            << localSuspicionCauseSaveName(suspicion.cause) << ' '
            << localSuspicionResolutionSaveName(suspicion.resolution) << ' '
            << suspicion.workplace_stable_id << ' '
            << (suspicion.target_is_carryable ? "CARRYABLE" : "SIGNPOST") << ' '
            << suspicion.target_path_from_stable_id << ' '
            << suspicion.target_path_to_stable_id << ' '
            << suspicion.target_endpoint_stable_id << ' '
            << suspicion.path_from_stable_id << ' '
            << suspicion.path_to_stable_id << ' '
            << suspicion.resolution_stable_id << ' '
            << (suspicion.institutional_log_recovered ? 1 : 0) << "\n";
    }
    out << "END\n";
    return out.str();
}

inline TinySaveStatus deserializeTinySaveState(const std::string& text, TinySaveState& state) {
    TinySaveState parsed;
    std::istringstream in(text);

    std::string tag;
    if (!(in >> tag) ||
        (tag != "NEON_TINY_SAVE_V7" && tag != "NEON_TINY_SAVE_V8" &&
         tag != "NEON_TINY_SAVE_V9" && tag != "NEON_TINY_SAVE_V10" &&
         tag != "NEON_TINY_SAVE_V11" && tag != "NEON_TINY_SAVE_V12")) {
        return TinySaveStatus::INVALID_FORMAT;
    }
    const bool has_local_suspicion_record =
        tag == "NEON_TINY_SAVE_V8" || tag == "NEON_TINY_SAVE_V9" ||
        tag == "NEON_TINY_SAVE_V10" || tag == "NEON_TINY_SAVE_V11" ||
        tag == "NEON_TINY_SAVE_V12";
    const bool has_wage_record_field = tag == "NEON_TINY_SAVE_V9" ||
        tag == "NEON_TINY_SAVE_V10" || tag == "NEON_TINY_SAVE_V11" ||
        tag == "NEON_TINY_SAVE_V12";
    const bool has_world_phase_field = tag == "NEON_TINY_SAVE_V10" ||
        tag == "NEON_TINY_SAVE_V11" || tag == "NEON_TINY_SAVE_V12";
    const bool has_multi_worker_field = tag == "NEON_TINY_SAVE_V10" ||
        tag == "NEON_TINY_SAVE_V11" || tag == "NEON_TINY_SAVE_V12";
    const bool has_transit_field = tag == "NEON_TINY_SAVE_V11" ||
        tag == "NEON_TINY_SAVE_V12";
    const bool has_dependency_disruption_field = tag == "NEON_TINY_SAVE_V12";

    int flag = 0;
    if (!(in >> tag) || tag != "PLAYER") return TinySaveStatus::INVALID_FORMAT;
    if (!readSavedTransform(in, parsed.player)) return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> flag)) return TinySaveStatus::INVALID_FORMAT;
    parsed.player_carrying = flag != 0;

    std::string role_name;
    if (!(in >> tag) || tag != "BUILDING") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> flag >> role_name >> parsed.building_stable_id)) return TinySaveStatus::INVALID_FORMAT;
    parsed.inside_building = flag != 0;
    if (!roleFromSaveName(role_name, parsed.building_role)) return TinySaveStatus::INVALID_FORMAT;
    if (!readSavedTransform(in, parsed.exterior_position)) return TinySaveStatus::INVALID_FORMAT;
    if (!readSavedTransform(in, parsed.interior_position)) return TinySaveStatus::INVALID_FORMAT;

    if (!(in >> tag) || tag != "CARRYABLE") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> flag)) return TinySaveStatus::INVALID_FORMAT;
    parsed.has_carryable = flag != 0;
    std::string item_kind_name;
    if (!(in >> item_kind_name)) return TinySaveStatus::INVALID_FORMAT;
    if (!itemKindFromSaveName(item_kind_name, parsed.carryable_kind)) {
        return TinySaveStatus::INVALID_FORMAT;
    }
    if (!readSavedTransform(in, parsed.carryable_position)) return TinySaveStatus::INVALID_FORMAT;

    if (!(in >> tag) || tag != "SHELTER") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> parsed.shelter_supply)) return TinySaveStatus::INVALID_FORMAT;
    parsed.shelter_supply = std::clamp(parsed.shelter_supply, 0, 1);

    if (!(in >> tag) || tag != "WORKPLACE_BENCH") return TinySaveStatus::INVALID_FORMAT;
    std::string bench_state_name;
    if (!(in >> bench_state_name)) return TinySaveStatus::INVALID_FORMAT;
    if (!workplaceBenchStateFromSaveName(bench_state_name, parsed.workplace_bench_state)) {
        return TinySaveStatus::INVALID_FORMAT;
    }

    if (!(in >> tag) || tag != "BUILDING_IMPROVEMENT") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> flag)) return TinySaveStatus::INVALID_FORMAT;
    parsed.building_improved = flag != 0;

    if (has_world_phase_field) {
        std::string phase_name;
        if (!(in >> tag) || tag != "WORLD_PHASE") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> phase_name >>
              parsed.world_phase_elapsed_seconds >>
              parsed.world_phase_interval_seconds)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (!worldPhaseFromSaveName(phase_name, parsed.world_phase)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        parsed.world_phase_elapsed_seconds = std::max(0.0f, parsed.world_phase_elapsed_seconds);
        parsed.world_phase_interval_seconds = std::max(1.0f, parsed.world_phase_interval_seconds);
        parsed.world_phase_elapsed_seconds =
            std::fmod(parsed.world_phase_elapsed_seconds,
                      parsed.world_phase_interval_seconds);
    }

    if (has_transit_field) {
        int transit_flag = 0;
        int doors_flag = 0;
        if (!(in >> tag) || tag != "TRANSIT") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> transit_flag >>
              parsed.transit_origin_station_stable_id >>
              parsed.transit_destination_station_stable_id >>
              parsed.transit_elapsed_seconds >>
              parsed.transit_stop_interval_seconds >>
              doors_flag)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        parsed.in_transit = transit_flag != 0;
        parsed.transit_doors_open = doors_flag != 0;
        if (!readSavedTransform(in, parsed.transit_exterior_position)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (!readSavedTransform(in, parsed.transit_interior_position)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        parsed.transit_stop_interval_seconds = std::max(1.0f, parsed.transit_stop_interval_seconds);
        parsed.transit_elapsed_seconds = std::clamp(parsed.transit_elapsed_seconds,
                                                    0.0f,
                                                    parsed.transit_stop_interval_seconds);
        if (parsed.transit_elapsed_seconds >= parsed.transit_stop_interval_seconds) {
            parsed.transit_doors_open = true;
        }
    }

    auto read_worker = [&](SavedWorker& worker) {
        int worker_flag = 0;
        int acknowledged_flag = 0;
        int worker_carrying_flag = 0;
        if (!(in >> tag) || tag != "WORKER") return false;
        if (!(in >> worker_flag >> worker.path_from_stable_id >>
              worker.path_to_stable_id >>
              worker.route_t >>
              worker.direction >>
              acknowledged_flag >>
              worker_carrying_flag)) {
            return false;
        }
        worker.present = worker_flag != 0;
        worker.acknowledged = acknowledged_flag != 0;
        worker.carrying = worker_carrying_flag != 0;
        if (has_wage_record_field) {
            int wage_record_flag = 0;
            if (!(in >> wage_record_flag)) return false;
            worker.wage_record_spoofed = wage_record_flag != 0;
        }
        worker.route_t = std::clamp(worker.route_t, 0.0f, 1.0f);
        if (std::fabs(worker.direction) <= 0.001f) {
            worker.direction = 0.0f;
        } else {
            worker.direction = worker.direction > 0.0f ? 1.0f : -1.0f;
        }
        return true;
    };

    if (has_multi_worker_field) {
        size_t worker_count = 0;
        if (!(in >> tag) || tag != "WORKERS") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> worker_count)) return TinySaveStatus::INVALID_FORMAT;
        parsed.workers.clear();
        parsed.workers.reserve(worker_count);
        for (size_t i = 0; i < worker_count; ++i) {
            SavedWorker worker;
            if (!read_worker(worker)) return TinySaveStatus::INVALID_FORMAT;
            parsed.workers.push_back(worker);
        }
    } else {
        SavedWorker worker;
        if (!read_worker(worker)) return TinySaveStatus::INVALID_FORMAT;
        parsed.workers.push_back(worker);
    }
    for (const auto& worker : parsed.workers) {
        if (worker.present && !parsed.has_worker) {
            parsed.has_worker = true;
            parsed.worker_path_from_stable_id = worker.path_from_stable_id;
            parsed.worker_path_to_stable_id = worker.path_to_stable_id;
            parsed.worker_route_t = worker.route_t;
            parsed.worker_direction = worker.direction;
            parsed.worker_acknowledged = worker.acknowledged;
            parsed.worker_carrying = worker.carrying;
            parsed.worker_wage_record_spoofed = worker.wage_record_spoofed;
        }
    }

    size_t spoofed_count = 0;
    if (!(in >> tag) || tag != "SPOOFED_SIGNPOSTS") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> spoofed_count)) return TinySaveStatus::INVALID_FORMAT;
    parsed.spoofed_signposts.clear();
    parsed.spoofed_signposts.reserve(spoofed_count);
    for (size_t i = 0; i < spoofed_count; ++i) {
        SavedSpoofedSignpost signpost;
        if (!(in >> tag) || tag != "SPOOFED_SIGNPOST") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> signpost.path_from_stable_id >>
              signpost.path_to_stable_id >>
              signpost.endpoint_stable_id)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        parsed.spoofed_signposts.push_back(signpost);
    }

    if (has_dependency_disruption_field) {
        size_t disruption_count = 0;
        if (!(in >> tag) || tag != "DEPENDENCY_DISRUPTIONS") {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (!(in >> disruption_count)) return TinySaveStatus::INVALID_FORMAT;
        parsed.dependency_disruptions.clear();
        parsed.dependency_disruptions.reserve(disruption_count);
        for (size_t i = 0; i < disruption_count; ++i) {
            SavedDependencyDisruption disruption;
            std::string dependent_name;
            std::string provider_name;
            int disrupted_flag = 0;
            if (!(in >> tag) || tag != "DEPENDENCY_DISRUPTION") {
                return TinySaveStatus::INVALID_FORMAT;
            }
            if (!(in >> dependent_name >>
                  provider_name >>
                  disruption.district_id >>
                  disrupted_flag)) {
                return TinySaveStatus::INVALID_FORMAT;
            }
            if (!roleFromSaveName(dependent_name, disruption.dependent_role) ||
                !roleFromSaveName(provider_name, disruption.provider_role)) {
                return TinySaveStatus::INVALID_FORMAT;
            }
            disruption.disrupted = disrupted_flag != 0;
            if (disruption.disrupted) {
                parsed.dependency_disruptions.push_back(disruption);
            }
        }
    }

    if (has_local_suspicion_record && has_multi_worker_field) {
        size_t suspicion_count = 0;
        if (!(in >> tag) || tag != "LOCAL_SUSPICIONS") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> suspicion_count)) return TinySaveStatus::INVALID_FORMAT;
        parsed.local_suspicions.clear();
        parsed.local_suspicions.reserve(suspicion_count);
        for (size_t i = 0; i < suspicion_count; ++i) {
            SavedWorkerSuspicion record;
            int present_flag = 0;
            int active_flag = 0;
            int log_recovered_flag = 0;
            std::string cause_name;
            std::string resolution_name;
            std::string target_kind;
            if (!(in >> tag) || tag != "LOCAL_SUSPICION") return TinySaveStatus::INVALID_FORMAT;
            if (!(in >> record.worker_index >>
                  present_flag >>
                  active_flag >>
                  cause_name >>
                  resolution_name >>
                  record.suspicion.workplace_stable_id >>
                  target_kind >>
                  record.suspicion.target_path_from_stable_id >>
                  record.suspicion.target_path_to_stable_id >>
                  record.suspicion.target_endpoint_stable_id >>
                  record.suspicion.path_from_stable_id >>
                  record.suspicion.path_to_stable_id >>
                  record.suspicion.resolution_stable_id >>
                  log_recovered_flag)) {
                return TinySaveStatus::INVALID_FORMAT;
            }
            record.suspicion.present = present_flag != 0;
            record.suspicion.active = active_flag != 0;
            record.suspicion.institutional_log_recovered = log_recovered_flag != 0;
            if (!localSuspicionCauseFromSaveName(cause_name, record.suspicion.cause)) {
                return TinySaveStatus::INVALID_FORMAT;
            }
            if (!localSuspicionResolutionFromSaveName(resolution_name,
                                                      record.suspicion.resolution)) {
                return TinySaveStatus::INVALID_FORMAT;
            }
            if (target_kind == "CARRYABLE") {
                record.suspicion.target_is_carryable = true;
            } else if (target_kind == "SIGNPOST") {
                record.suspicion.target_is_carryable = false;
            } else {
                return TinySaveStatus::INVALID_FORMAT;
            }
            if (record.suspicion.present) {
                if (!parsed.local_suspicion.present) {
                    parsed.local_suspicion = record.suspicion;
                }
                parsed.local_suspicions.push_back(record);
            }
        }
    } else if (has_local_suspicion_record) {
        int present_flag = 0;
        int active_flag = 0;
        int log_recovered_flag = 0;
        std::string cause_name;
        std::string resolution_name;
        std::string target_kind;
        if (!(in >> tag) || tag != "LOCAL_SUSPICION") return TinySaveStatus::INVALID_FORMAT;
        if (!(in >> present_flag >>
              active_flag >>
              cause_name >>
              resolution_name >>
              parsed.local_suspicion.workplace_stable_id >>
              target_kind >>
              parsed.local_suspicion.target_path_from_stable_id >>
              parsed.local_suspicion.target_path_to_stable_id >>
              parsed.local_suspicion.target_endpoint_stable_id >>
              parsed.local_suspicion.path_from_stable_id >>
              parsed.local_suspicion.path_to_stable_id >>
              parsed.local_suspicion.resolution_stable_id >>
              log_recovered_flag)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        parsed.local_suspicion.present = present_flag != 0;
        parsed.local_suspicion.active = active_flag != 0;
        parsed.local_suspicion.institutional_log_recovered = log_recovered_flag != 0;
        if (!localSuspicionCauseFromSaveName(cause_name, parsed.local_suspicion.cause)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (!localSuspicionResolutionFromSaveName(resolution_name,
                                                  parsed.local_suspicion.resolution)) {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (target_kind == "CARRYABLE") {
            parsed.local_suspicion.target_is_carryable = true;
        } else if (target_kind == "SIGNPOST") {
            parsed.local_suspicion.target_is_carryable = false;
        } else {
            return TinySaveStatus::INVALID_FORMAT;
        }
        if (parsed.local_suspicion.present) {
            parsed.local_suspicions.push_back(
                SavedWorkerSuspicion{0, parsed.local_suspicion});
        }
    }

    if (!(in >> tag) || tag != "END") return TinySaveStatus::INVALID_FORMAT;

    state = parsed;
    return TinySaveStatus::OK;
}

inline TinySaveStatus applyTinySaveState(Registry& registry, Entity player, const TinySaveState& state) {
    if (!registry.alive(player) ||
        !registry.has<PlayerComponent>(player) ||
        !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player)) {
        return TinySaveStatus::INVALID_WORLD;
    }

    registry.get<TransformComponent>(player) = transformFromSavedTransform(state.player);
    auto& player_component = registry.get<PlayerComponent>(player);

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    interaction.inside_building = false;
    interaction.building_entity = MAX_ENTITIES;
    interaction.building_role = state.building_role;
    interaction.exterior_position = transformFromSavedTransform(state.exterior_position);
    interaction.interior_position = transformFromSavedTransform(state.interior_position);

    if (state.inside_building) {
        const Entity building = buildingByStableId(registry, state.building_stable_id);
        if (building != MAX_ENTITIES && registry.has<BuildingUseComponent>(building)) {
            interaction.inside_building = true;
            interaction.building_entity = building;
            interaction.building_role = registry.get<BuildingUseComponent>(building).role;
        }
    }

    const Entity carryable = firstCarryableObject(registry);
    player_component.carried_object = MAX_ENTITIES;
    if (state.has_carryable && carryable != MAX_ENTITIES) {
        registry.get<CarryableComponent>(carryable).kind = state.carryable_kind;
        if (state.player_carrying) {
            player_component.carried_object = carryable;
            registry.get<TransformComponent>(carryable) =
                TransformComponent{99999.0f, 99999.0f,
                                   state.carryable_position.width,
                                   state.carryable_position.height};
        } else {
            registry.get<TransformComponent>(carryable) =
                transformFromSavedTransform(state.carryable_position);
        }
    }

    const Entity shelter = firstShelterStockEntity(registry);
    if (shelter != MAX_ENTITIES) {
        auto& stock = registry.get<ShelterStockComponent>(shelter);
        stock.current_supply = std::clamp(state.shelter_supply, 0, stock.capacity);
    }

    const Entity workplace = firstWorkplaceBenchEntity(registry);
    if (workplace != MAX_ENTITIES) {
        registry.get<WorkplaceBenchComponent>(workplace).state = state.workplace_bench_state;
    }

    const Entity improved_building = firstBuildingImprovementBuilding(registry);
    if (improved_building != MAX_ENTITIES) {
        registry.get<BuildingImprovementComponent>(improved_building).improved =
            state.building_improved;
    }

    const Entity phase_entity = worldPhaseEntity(registry);
    if (phase_entity != MAX_ENTITIES) {
        auto& phase = registry.get<WorldPhaseComponent>(phase_entity);
        phase.phase = state.world_phase;
        phase.interval_seconds = std::max(1.0f, state.world_phase_interval_seconds);
        phase.elapsed_seconds =
            std::fmod(std::max(0.0f, state.world_phase_elapsed_seconds),
                      phase.interval_seconds);
    }

    if (registry.has<TransitRideComponent>(player)) {
        registry.remove<TransitRideComponent>(player);
    }
    if (state.in_transit) {
        const Entity origin =
            stationByStableId(registry, state.transit_origin_station_stable_id);
        const Entity destination =
            stationByStableId(registry, state.transit_destination_station_stable_id);
        if (origin != MAX_ENTITIES && destination != MAX_ENTITIES) {
            TransitRideComponent ride;
            ride.origin_station = origin;
            ride.destination_station = destination;
            ride.origin_district_id = registry.get<StationComponent>(origin).district_id;
            ride.destination_district_id = registry.get<StationComponent>(destination).district_id;
            ride.elapsed_seconds = std::max(0.0f, state.transit_elapsed_seconds);
            ride.stop_interval_seconds = std::max(1.0f, state.transit_stop_interval_seconds);
            ride.doors_open = state.transit_doors_open ||
                ride.elapsed_seconds >= ride.stop_interval_seconds;
            ride.exterior_position = transformFromSavedTransform(state.transit_exterior_position);
            ride.interior_position = transformFromSavedTransform(state.transit_interior_position);
            registry.assign<TransitRideComponent>(player, ride);
        }
    }

    const std::vector<Entity> workers = fixedWorkersInSaveOrder(registry);
    std::vector<SavedWorker> saved_workers = state.workers;
    if (saved_workers.empty()) {
        saved_workers.push_back(SavedWorker{
            state.has_worker,
            state.worker_path_from_stable_id,
            state.worker_path_to_stable_id,
            state.worker_route_t,
            state.worker_direction,
            state.worker_acknowledged,
            state.worker_carrying,
            state.worker_wage_record_spoofed});
    }
    for (size_t i = 0; i < workers.size() && i < saved_workers.size(); ++i) {
        const SavedWorker& saved_worker = saved_workers[i];
        if (!saved_worker.present) continue;
        const Entity worker = workers[i];
        auto& worker_component = registry.get<FixedActorComponent>(worker);
        worker_component.carried_object = MAX_ENTITIES;
        Entity path = pedestrianPathByEndpointStableIds(registry,
                                                        saved_worker.path_from_stable_id,
                                                        saved_worker.path_to_stable_id);
        if (path == MAX_ENTITIES) path = firstPedestrianPath(registry);
        if (path != MAX_ENTITIES && registry.has<TransformComponent>(path)) {
            worker_component.path_entity = path;
            worker_component.route_t = std::clamp(saved_worker.route_t, 0.0f, 1.0f);
            if (std::fabs(saved_worker.direction) <= 0.001f) {
                worker_component.direction = 0.0f;
            } else {
                worker_component.direction = saved_worker.direction > 0.0f ? 1.0f : -1.0f;
            }
            worker_component.acknowledged = saved_worker.acknowledged;
            worker_component.wage_record_spoofed = saved_worker.wage_record_spoofed;
            if (saved_worker.carrying && carryable != MAX_ENTITIES) {
                worker_component.carried_object = carryable;
                registry.get<TransformComponent>(carryable) =
                    TransformComponent{99999.0f, 99999.0f,
                                       state.carryable_position.width,
                                       state.carryable_position.height};
            }
            registry.get<TransformComponent>(worker) =
                transformOnPath(registry.get<TransformComponent>(path), worker_component.route_t);
        }
    }

    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        auto& signpost = registry.get<RouteSignpostComponent>(marker);
        signpost.spoofed = false;
        signpost.signal_recovered = false;
    }
    for (const auto& saved_signpost : state.spoofed_signposts) {
        const Entity marker = routeSignpostByStableIds(registry,
                                                       saved_signpost.path_from_stable_id,
                                                       saved_signpost.path_to_stable_id,
                                                       saved_signpost.endpoint_stable_id);
        if (marker != MAX_ENTITIES) {
            registry.get<RouteSignpostComponent>(marker).spoofed = true;
        }
    }

    auto disruptions = registry.view<DependencyDisruptionComponent>();
    for (Entity disruption : disruptions) {
        auto& component = registry.get<DependencyDisruptionComponent>(disruption);
        component.disrupted = false;
        component.recovered = false;
    }
    for (const auto& saved_disruption : state.dependency_disruptions) {
        if (!saved_disruption.disrupted) {
            continue;
        }
        DependencySpec dependency{
            saved_disruption.dependent_role,
            saved_disruption.provider_role,
            kWorkplaceDependsOnSupply.flow_label,
            kWorkplaceDependsOnSupply.required_for};
        if (!dependencyEdgeResolved(registry, dependency, saved_disruption.district_id)) {
            continue;
        }
        const Entity state_entity = dependencyDisruptionStateEntity(registry,
                                                                    dependency,
                                                                    saved_disruption.district_id,
                                                                    true);
        if (state_entity != MAX_ENTITIES) {
            auto& component = registry.get<DependencyDisruptionComponent>(state_entity);
            component.disrupted = true;
            component.recovered = false;
        }
    }

    auto clinic_ledgers = registry.view<ClinicAccessLedgerComponent>();
    for (Entity clinic : clinic_ledgers) {
        registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed = false;
    }

    auto suspicion_workers = registry.view<LocalSuspicionComponent>();
    for (Entity suspicion_worker : suspicion_workers) {
        registry.remove<LocalSuspicionComponent>(suspicion_worker);
    }
    std::vector<SavedWorkerSuspicion> saved_suspicions = state.local_suspicions;
    if (saved_suspicions.empty() && state.local_suspicion.present) {
        saved_suspicions.push_back(SavedWorkerSuspicion{0, state.local_suspicion});
    }
    for (const auto& saved_record : saved_suspicions) {
        if (!saved_record.suspicion.present ||
            saved_record.worker_index >= workers.size()) {
            continue;
        }
        const Entity worker = workers[saved_record.worker_index];
        if (worker != MAX_ENTITIES) {
            const auto& saved_suspicion = saved_record.suspicion;
            Entity target = MAX_ENTITIES;
            if (saved_suspicion.target_is_carryable) {
                target = carryable;
            } else {
                target = routeSignpostByStableIds(
                    registry,
                    saved_suspicion.target_path_from_stable_id,
                    saved_suspicion.target_path_to_stable_id,
                    saved_suspicion.target_endpoint_stable_id);
            }
            const Entity path =
                pedestrianPathByEndpointStableIds(registry,
                                                  saved_suspicion.path_from_stable_id,
                                                  saved_suspicion.path_to_stable_id);
            auto& suspicion = registry.assign<LocalSuspicionComponent>(worker);
            suspicion.active = saved_suspicion.active;
            suspicion.cause = saved_suspicion.cause;
            suspicion.resolution = saved_suspicion.resolution;
            suspicion.workplace_entity =
                buildingByStableId(registry, saved_suspicion.workplace_stable_id);
            suspicion.target_entity = target;
            suspicion.path_entity = path;
            suspicion.resolution_entity =
                buildingByStableId(registry, saved_suspicion.resolution_stable_id);
            suspicion.institutional_log_recovered =
                saved_suspicion.institutional_log_recovered;
        }
    }

    return TinySaveStatus::OK;
}

inline TinySaveStatus saveTinyStateToFile(Registry& registry, Entity player, const std::string& path) {
    std::ofstream out(path);
    if (!out) return TinySaveStatus::INVALID_WORLD;
    out << serializeTinySaveState(captureTinySaveState(registry, player));
    return out ? TinySaveStatus::OK : TinySaveStatus::INVALID_WORLD;
}

inline TinySaveStatus loadTinyStateFromFile(Registry& registry, Entity player, const std::string& path) {
    std::ifstream in(path);
    if (!in) return TinySaveStatus::MISSING_FILE;

    std::ostringstream buffer;
    buffer << in.rdbuf();

    TinySaveState state;
    const TinySaveStatus parsed = deserializeTinySaveState(buffer.str(), state);
    if (parsed != TinySaveStatus::OK) return parsed;
    return applyTinySaveState(registry, player, state);
}

inline const char* tinySaveStatusName(TinySaveStatus status) {
    switch (status) {
        case TinySaveStatus::OK: return "OK";
        case TinySaveStatus::MISSING_FILE: return "MISSING FILE";
        case TinySaveStatus::INVALID_FORMAT: return "INVALID FORMAT";
        case TinySaveStatus::INVALID_WORLD: return "INVALID WORLD";
    }
    return "INVALID";
}

inline bool tinySaveFileExists(const std::string& path) {
    std::ifstream in(path);
    return static_cast<bool>(in);
}

inline const char* tinySaveLocationName(PlayerLocationState state) {
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

inline std::string startupSaveStatusLine(bool save_file_exists) {
    return save_file_exists ? "SAVE FILE READY - F9 LOAD" : "NO SAVE FILE - F5 SAVE";
}

inline std::string saveResultStatusLine(TinySaveStatus status) {
    return std::string("SAVE ") + tinySaveStatusName(status);
}

inline std::string loadResultStatusLine(TinySaveStatus status,
                                        PlayerLocationState location,
                                        const std::string& carried_label) {
    if (status != TinySaveStatus::OK) {
        if (status == TinySaveStatus::MISSING_FILE) {
            return "LOAD MISSING FILE - F5 SAVE FIRST";
        }
        return std::string("LOAD ") + tinySaveStatusName(status);
    }

    return std::string("LOAD OK - ") +
           tinySaveLocationName(location) +
           " - CARRYING " +
           (carried_label.empty() ? "NONE" : carried_label);
}
