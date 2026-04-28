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

    std::vector<SavedSpoofedSignpost> spoofed_signposts;
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

    const Entity worker = firstFixedWorker(registry);
    if (worker != MAX_ENTITIES) {
        const auto& worker_component = registry.get<FixedActorComponent>(worker);
        state.has_worker = true;
        state.worker_route_t = std::clamp(worker_component.route_t, 0.0f, 1.0f);
        if (std::fabs(worker_component.direction) <= 0.001f) {
            state.worker_direction = 0.0f;
        } else {
            state.worker_direction = worker_component.direction > 0.0f ? 1.0f : -1.0f;
        }
        state.worker_acknowledged = worker_component.acknowledged;
        state.worker_carrying = worker_component.carried_object != MAX_ENTITIES;
        if (registry.alive(worker_component.path_entity) &&
            registry.has<PathComponent>(worker_component.path_entity)) {
            const auto& path = registry.get<PathComponent>(worker_component.path_entity);
            state.worker_path_from_stable_id = stableIdForEntity(registry, path.from);
            state.worker_path_to_stable_id = stableIdForEntity(registry, path.to);
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
    out << "NEON_TINY_SAVE_V7\n";
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

    out << "WORKER "
        << (state.has_worker ? 1 : 0) << ' '
        << state.worker_path_from_stable_id << ' '
        << state.worker_path_to_stable_id << ' '
        << state.worker_route_t << ' '
        << state.worker_direction << ' '
        << (state.worker_acknowledged ? 1 : 0) << ' '
        << (state.worker_carrying ? 1 : 0) << "\n";
    out << "SPOOFED_SIGNPOSTS " << state.spoofed_signposts.size() << "\n";
    for (const auto& signpost : state.spoofed_signposts) {
        out << "SPOOFED_SIGNPOST "
            << signpost.path_from_stable_id << ' '
            << signpost.path_to_stable_id << ' '
            << signpost.endpoint_stable_id << "\n";
    }
    out << "END\n";
    return out.str();
}

inline TinySaveStatus deserializeTinySaveState(const std::string& text, TinySaveState& state) {
    TinySaveState parsed;
    std::istringstream in(text);

    std::string tag;
    if (!(in >> tag) || tag != "NEON_TINY_SAVE_V7") return TinySaveStatus::INVALID_FORMAT;

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

    int worker_flag = 0;
    int acknowledged_flag = 0;
    int worker_carrying_flag = 0;
    if (!(in >> tag) || tag != "WORKER") return TinySaveStatus::INVALID_FORMAT;
    if (!(in >> worker_flag >> parsed.worker_path_from_stable_id >>
          parsed.worker_path_to_stable_id >>
          parsed.worker_route_t >>
          parsed.worker_direction >>
          acknowledged_flag >>
          worker_carrying_flag)) {
        return TinySaveStatus::INVALID_FORMAT;
    }
    parsed.has_worker = worker_flag != 0;
    parsed.worker_acknowledged = acknowledged_flag != 0;
    parsed.worker_carrying = worker_carrying_flag != 0;
    parsed.worker_route_t = std::clamp(parsed.worker_route_t, 0.0f, 1.0f);
    if (std::fabs(parsed.worker_direction) <= 0.001f) {
        parsed.worker_direction = 0.0f;
    } else {
        parsed.worker_direction = parsed.worker_direction > 0.0f ? 1.0f : -1.0f;
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

    const Entity worker = firstFixedWorker(registry);
    if (state.has_worker && worker != MAX_ENTITIES && registry.has<FixedActorComponent>(worker)) {
        auto& worker_component = registry.get<FixedActorComponent>(worker);
        worker_component.carried_object = MAX_ENTITIES;
        Entity path = pedestrianPathByEndpointStableIds(registry,
                                                        state.worker_path_from_stable_id,
                                                        state.worker_path_to_stable_id);
        if (path == MAX_ENTITIES) path = firstPedestrianPath(registry);
        if (path != MAX_ENTITIES && registry.has<TransformComponent>(path)) {
            worker_component.path_entity = path;
            worker_component.route_t = std::clamp(state.worker_route_t, 0.0f, 1.0f);
            if (std::fabs(state.worker_direction) <= 0.001f) {
                worker_component.direction = 0.0f;
            } else {
                worker_component.direction = state.worker_direction > 0.0f ? 1.0f : -1.0f;
            }
            worker_component.acknowledged = state.worker_acknowledged;
            if (state.worker_carrying && carryable != MAX_ENTITIES) {
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
