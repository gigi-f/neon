#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include "ecs.h"
#include "components.h"
#include "world_generation.h"

int main() {
    Registry registry;
    generateSandboxMicrocity(registry);

    assert(validateSandboxMicrocity(registry));

    bool has_housing = false;
    auto buildings = registry.view<BuildingComponent, ZoningComponent>();
    for (Entity e : buildings) {
        const auto& zone = registry.get<ZoningComponent>(e);
        if (zone.type == ZoneType::RESIDENTIAL) has_housing = true;
    }

    assert(has_housing);

    std::array<bool, 2> station_seen = {false, false};
    auto stations = registry.view<StationComponent>();
    assert(stations.size() >= 2);
    for (Entity e : stations) {
        int idx = registry.get<StationComponent>(e).station_index;
        if (idx >= 0 && idx < static_cast<int>(station_seen.size())) {
            station_seen[static_cast<size_t>(idx)] = true;
        }
    }
    for (bool seen : station_seen) {
        assert(seen);
    }

    auto transit_vehicles = registry.view<TransitVehicleComponent, VehicleComponent>();
    assert(transit_vehicles.size() >= 1);

    std::array<Entity, 64> roads{};
    size_t road_count = 0;
    auto road_view = registry.view<RoadComponent, TransformComponent>();
    for (Entity road : road_view) {
        if (road_count < roads.size()) roads[road_count++] = road;
    }

    auto citizens = registry.view<CitizenComponent>();
    assert(citizens.size() == kSandboxCitizenCount);
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();
    for (Entity e : citizens) {
        assert(intersectsAnyRoad(registry, e, roads, road_count));
        const auto& t = registry.get<TransformComponent>(e);
        assert(!overlapsAnyNonRoad(registry, aabbFromTransform(t), e));
        min_x = std::min(min_x, t.x);
        max_x = std::max(max_x, t.x);
        min_y = std::min(min_y, t.y);
        max_y = std::max(max_y, t.y);
    }
    assert((max_x - min_x) > 300.0f);
    assert((max_y - min_y) > 300.0f);

    for (Entity e : buildings) {
        assert(!intersectsAnyRoad(registry, e, roads, road_count));
    }

    for (Entity e : stations) {
        assert(!intersectsAnyRoad(registry, e, roads, road_count));
    }

    auto stairs = registry.view<StaircaseComponent>();
    for (Entity e : stairs) {
        assert(!intersectsAnyRoad(registry, e, roads, road_count));
    }

    auto waits = registry.view<WaitingAreaComponent>();
    for (Entity e : waits) {
        assert(!intersectsAnyRoad(registry, e, roads, road_count));
    }

    std::cout << "world_generation_tests passed\n";
    return 0;
}
