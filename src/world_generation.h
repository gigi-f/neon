#pragma once
#include "ecs.h"
#include "components.h"
#include <array>

constexpr size_t kSandboxCitizenCount = 24;

struct AabbRect {
    float left;
    float right;
    float top;
    float bottom;
};

inline AabbRect aabbFromTransform(const TransformComponent& t) {
    return {t.x - t.width * 0.5f, t.x + t.width * 0.5f,
            t.y - t.height * 0.5f, t.y + t.height * 0.5f};
}

inline bool aabbOverlap(const AabbRect& a, const AabbRect& b) {
    return !(a.right <= b.left || a.left >= b.right || a.bottom <= b.top || a.top >= b.bottom);
}

inline uint32_t deterministicStep(uint32_t value) {
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return value;
}

inline bool overlapsAnyNonRoad(Registry& registry, const AabbRect& candidate, Entity ignore = MAX_ENTITIES) {
    auto occupied = registry.view<TransformComponent>();
    for (Entity e : occupied) {
        if (e == ignore) continue;
        if (registry.has<RoadComponent>(e)) continue;
        const AabbRect existing = aabbFromTransform(registry.get<TransformComponent>(e));
        if (aabbOverlap(candidate, existing)) return true;
    }
    return false;
}

inline bool intersectsAnyRoad(Registry& registry, Entity entity,
                              const std::array<Entity, 64>& road_entities,
                              size_t road_count) {
    if (!registry.has<TransformComponent>(entity)) return false;
    const AabbRect entity_rect = aabbFromTransform(registry.get<TransformComponent>(entity));
    for (size_t i = 0; i < road_count; ++i) {
        Entity road = road_entities[i];
        if (!registry.alive(road) || !registry.has<TransformComponent>(road)) continue;
        const AabbRect road_rect = aabbFromTransform(registry.get<TransformComponent>(road));
        if (aabbOverlap(entity_rect, road_rect)) return true;
    }
    return false;
}

inline Entity spawnHousingBuilding(Registry& registry, float x, float y,
                                   float width, float height, int floors) {
    Entity building = registry.create();
    uint64_t stable_id =
        (static_cast<uint64_t>(static_cast<int>(x) + 4096) << 32) ^
        static_cast<uint64_t>(static_cast<int>(y) + 4096);

    registry.assign<TransformComponent>(building, x, y, width, height);
    registry.assign<SolidComponent>(building);
    registry.assign<BuildingComponent>(building, stable_id, floors, true);
    registry.assign<BuildingAtmosphereComponent>(building, 20.0f, 100.0f);
    registry.assign<ZoningComponent>(building, ZoneType::RESIDENTIAL);

    StructuralComponent structure;
    structure.integrity = 100.0f;
    structure.is_exposed = true;
    structure.material_type = MaterialType::COMPOSITE;
    registry.assign<StructuralComponent>(building, structure);

    registry.assign<PowerNodeComponent>(building, 0.0f, 12.0f * static_cast<float>(floors), false);
    registry.assign<GlyphComponent>(building, std::string("H"),
        (uint8_t)100, (uint8_t)170, (uint8_t)255, (uint8_t)255, 1.0f, true, true);

    return building;
}

inline void seedSandboxCitizens(Registry& registry, size_t count) {
    std::array<Entity, 64> spawn_roads{};
    size_t spawn_road_count = 0;
    auto roads = registry.view<RoadComponent, TransformComponent>();
    for (Entity road : roads) {
        auto& road_component = registry.get<RoadComponent>(road);
        if (road_component.type == RoadType::MAGLIFT_TRACK) continue;
        if (spawn_road_count < spawn_roads.size()) {
            spawn_roads[spawn_road_count++] = road;
        }
    }

    if (spawn_road_count == 0) return;

    for (size_t i = 0; i < count; ++i) {
        float px = 0.0f;
        float py = 0.0f;
        bool placed = false;

        for (size_t attempt = 0; attempt < spawn_road_count * 16; ++attempt) {
            uint32_t seed = deterministicStep(static_cast<uint32_t>(i * 2654435761u + attempt * 374761393u));
            size_t road_index = static_cast<size_t>((seed + static_cast<uint32_t>(attempt)) % spawn_road_count);
            Entity road = spawn_roads[road_index];
            const auto& road_transform = registry.get<TransformComponent>(road);
            const AabbRect road_rect = aabbFromTransform(road_transform);

            uint32_t sx = deterministicStep(seed ^ 0x9e3779b9u);
            uint32_t sy = deterministicStep(sx ^ 0x7f4a7c15u);
            float nx = (static_cast<float>(sx & 0xffffu) / 65535.0f) - 0.5f;
            float ny = (static_cast<float>(sy & 0xffffu) / 65535.0f) - 0.5f;

            float half_w = 4.0f;
            float half_h = 4.0f;
            float usable_w = std::max(half_w, road_transform.width * 0.5f - half_w);
            float usable_h = std::max(half_h, road_transform.height * 0.5f - half_h);
            float candidate_x = road_transform.x + nx * usable_w * 2.0f;
            float candidate_y = road_transform.y + ny * usable_h * 2.0f;
            AabbRect candidate = {candidate_x - half_w, candidate_x + half_w,
                                  candidate_y - half_h, candidate_y + half_h};

            if (!aabbOverlap(candidate, road_rect)) continue;
            if (overlapsAnyNonRoad(registry, candidate)) continue;

            px = candidate_x;
            py = candidate_y;
            placed = true;
            break;
        }

        if (!placed) continue;

        Entity citizen = registry.create();

        registry.assign<CitizenComponent>(citizen);
        registry.assign<TransformComponent>(citizen, px, py, 8.0f, 8.0f);
        registry.assign<MovementComponent>(citizen, 0.0f, 0.0f, MovementComponent::NORMAL);
        registry.assign<GlyphComponent>(citizen, std::string("i"),
            (uint8_t)150, (uint8_t)200, (uint8_t)255, (uint8_t)255, 0.5f, true);

        BiologyComponent bio;
        bio.health = 90.0f;
        bio.hunger = 85.0f;
        bio.thirst = 85.0f;
        bio.fatigue = 75.0f;
        registry.assign<BiologyComponent>(citizen, bio);
        registry.assign<CognitiveComponent>(citizen);
        registry.assign<RelationshipComponent>(citizen);
        registry.assign<ConversationComponent>(citizen);

        ScheduleComponent schedule;
        if ((i % 5) == 0) {
            schedule.work_start = 20.0f;
            schedule.work_end = 5.0f;
            schedule.sleep_start = 9.0f;
            schedule.sleep_end = 17.0f;
        }
        registry.assign<ScheduleComponent>(citizen, schedule);

        EconomicComponent eco;
        eco.credits = 80.0f + static_cast<float>((i % 7) * 8);
        eco.employer = MAX_ENTITIES;
        eco.daily_wage = 0.0f;
        registry.assign<EconomicComponent>(citizen, eco);
        registry.assign<SocialRankComponent>(citizen, SocialRank::WORKING_POOR, 0.0f);
    }
}

inline void generateSandboxMicrocity(Registry& registry) {
    constexpr float track_x = -320.0f;
    constexpr float station_x = -220.0f;
    constexpr float stair_x = -120.0f;
    constexpr float wait_x = -20.0f;

    constexpr float housing_y = -260.0f;
    constexpr float depot_y = 260.0f;

    constexpr float housing_center_x = 500.0f;
    constexpr float housing_center_y = -410.0f;

    std::array<Entity, 64> roads{};
    size_t road_count = 0;

    Entity track = registry.create();
    registry.assign<TransformComponent>(track, track_x, 0.0f, 40.0f, 920.0f);
    registry.assign<RoadComponent>(track, RoadType::MAGLIFT_TRACK, 0.0f);
    roads[road_count++] = track;

    Entity service_vertical = registry.create();
    registry.assign<TransformComponent>(service_vertical, 120.0f, 0.0f, 80.0f, 1200.0f);
    registry.assign<RoadComponent>(service_vertical, RoadType::PRIMARY, 1.0f);
    registry.assign<PowerConduitComponent>(service_vertical, 9000.0f);
    roads[road_count++] = service_vertical;

    Entity service_horizontal = registry.create();
    registry.assign<TransformComponent>(service_horizontal, 120.0f, 60.0f, 1180.0f, 80.0f);
    registry.assign<RoadComponent>(service_horizontal, RoadType::SECONDARY, 0.8f);
    registry.assign<PowerConduitComponent>(service_horizontal, 7000.0f);
    roads[road_count++] = service_horizontal;

    Entity housing_connector = registry.create();
    registry.assign<TransformComponent>(housing_connector, 300.0f, housing_y, 380.0f, 34.0f);
    registry.assign<RoadComponent>(housing_connector, RoadType::PEDESTRIAN_PATH, 0.0f);
    roads[road_count++] = housing_connector;

    const std::array<float, 2> station_y = {housing_y, depot_y};
    for (size_t i = 0; i < station_y.size(); ++i) {
        float y = station_y[i];

        Entity station = registry.create();
        registry.assign<TransformComponent>(station, station_x, y, 84.0f, 44.0f);
        registry.assign<StationComponent>(station, static_cast<int>(i), 4.0f);
        registry.assign<GlyphComponent>(station, std::string("[S]"),
            (uint8_t)80, (uint8_t)220, (uint8_t)210, (uint8_t)255, 1.0f, true);

        Entity stair = registry.create();
        registry.assign<TransformComponent>(stair, stair_x, y, 24.0f, 44.0f);
        registry.assign<StaircaseComponent>(stair, static_cast<int>(i));
        registry.assign<GlyphComponent>(stair, std::string("||"),
            (uint8_t)210, (uint8_t)165, (uint8_t)85, (uint8_t)255, 0.75f, true);

        Entity wait = registry.create();
        registry.assign<TransformComponent>(wait, wait_x, y, 96.0f, 64.0f);
        registry.assign<WaitingAreaComponent>(wait, static_cast<int>(i));
        registry.assign<GlyphComponent>(wait, std::string("W"),
            (uint8_t)110, (uint8_t)220, (uint8_t)220, (uint8_t)255, 0.9f, true);
    }

    spawnHousingBuilding(registry, housing_center_x, housing_center_y, 220.0f, 170.0f, 10);
    spawnHousingBuilding(registry, housing_center_x + 230.0f, housing_center_y + 20.0f, 170.0f, 130.0f, 6);
    seedSandboxCitizens(registry, kSandboxCitizenCount);

    {
        Entity vehicle = registry.create();
        registry.assign<TransformComponent>(vehicle, track_x, housing_y, 88.0f, 28.0f);
        registry.assign<VehicleComponent>(vehicle, VehicleComponent::MAGLIFT, MAX_ENTITIES, 140.0f);
        registry.assign<GlyphComponent>(vehicle, std::string("M"),
            (uint8_t)255, (uint8_t)80, (uint8_t)140, (uint8_t)255, 0.5f, true, true);
        TransitVehicleComponent tvc{};
        tvc.state = TransitVehicleComponent::STOPPED;
        tvc.current_station = 0;
        tvc.next_station = 1;
        tvc.direction = 1;
        tvc.stop_timer = 2.0f;
        tvc.speed = 140.0f;
        tvc.track_x = track_x;
        registry.assign<TransitVehicleComponent>(vehicle, tvc);
    }
}

inline bool validateSandboxMicrocity(Registry& registry) {
    bool has_housing = false;
    bool station0 = false;
    bool station1 = false;
    bool has_housing_connector = false;
    bool building_road_overlap = false;
    bool station_support_road_overlap = false;
    bool has_fixed_population = false;
    bool citizens_validly_placed = true;

    std::array<Entity, 64> road_entities{};
    size_t road_count = 0;
    auto roads = registry.view<RoadComponent, TransformComponent>();
    for (Entity road : roads) {
        if (road_count < road_entities.size()) {
            road_entities[road_count++] = road;
        }
        const auto& road_comp = registry.get<RoadComponent>(road);
        const auto& t = registry.get<TransformComponent>(road);
        if (road_comp.type == RoadType::PEDESTRIAN_PATH && t.width >= 300.0f && t.y < 0.0f) {
            has_housing_connector = true;
        }
    }

    auto buildings = registry.view<BuildingComponent, ZoningComponent>();
    for (Entity e : buildings) {
        if (registry.get<ZoningComponent>(e).type == ZoneType::RESIDENTIAL) {
            has_housing = true;
        }
        if (intersectsAnyRoad(registry, e, road_entities, road_count)) {
            building_road_overlap = true;
        }
    }

    auto stations = registry.view<StationComponent>();
    for (Entity e : stations) {
        int idx = registry.get<StationComponent>(e).station_index;
        if (idx == 0) station0 = true;
        if (idx == 1) station1 = true;
        if (intersectsAnyRoad(registry, e, road_entities, road_count)) {
            station_support_road_overlap = true;
        }
    }

    auto stairs = registry.view<StaircaseComponent>();
    for (Entity e : stairs)
        if (intersectsAnyRoad(registry, e, road_entities, road_count)) station_support_road_overlap = true;

    auto waits = registry.view<WaitingAreaComponent>();
    for (Entity e : waits)
        if (intersectsAnyRoad(registry, e, road_entities, road_count)) station_support_road_overlap = true;

    auto citizens = registry.view<CitizenComponent>();
    has_fixed_population = (citizens.size() == kSandboxCitizenCount);
    for (Entity e : citizens) {
        if (!intersectsAnyRoad(registry, e, road_entities, road_count)) {
            citizens_validly_placed = false;
            break;
        }
        const auto& t = registry.get<TransformComponent>(e);
        if (overlapsAnyNonRoad(registry, aabbFromTransform(t), e)) {
            citizens_validly_placed = false;
            break;
        }
    }

    return has_housing && station0 && station1 && has_housing_connector &&
           !building_road_overlap && !station_support_road_overlap &&
           has_fixed_population && citizens_validly_placed;
}
