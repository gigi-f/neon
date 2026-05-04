#pragma once

#include <cstdint>
#include "components.h"

inline constexpr int MACRO_ZONE_COUNT_X = 1;
inline constexpr int MACRO_ZONE_COUNT_Y = 1;
inline constexpr float MACRO_ZONE_SIZE_WU = 360.0f;
inline constexpr uint32_t kDefaultWorldSeed = 0xC0FFEEu;

struct ConnectionSpec {
    MicroZoneRole from_role = MicroZoneRole::HOUSING;
    MicroZoneRole to_role = MicroZoneRole::WORKPLACE;
    PathKind path_kind = PathKind::PEDESTRIAN;
};

struct DependencySpec {
    MicroZoneRole dependent_role = MicroZoneRole::WORKPLACE;
    MicroZoneRole provider_role = MicroZoneRole::SUPPLY;
    const char* flow_label = "MATERIAL";
    const char* required_for = "BENCH STOCK";
};

inline constexpr ConnectionSpec kHousingToWorkplacePedestrianAccess{
    MicroZoneRole::HOUSING,
    MicroZoneRole::WORKPLACE,
    PathKind::PEDESTRIAN
};

inline constexpr ConnectionSpec kWorkplaceToSupplyPedestrianAccess{
    MicroZoneRole::WORKPLACE,
    MicroZoneRole::SUPPLY,
    PathKind::PEDESTRIAN
};

inline constexpr DependencySpec kWorkplaceDependsOnSupply{
    MicroZoneRole::WORKPLACE,
    MicroZoneRole::SUPPLY,
    "MATERIAL",
    "BENCH STOCK"
};

struct WorldConfig {
    int macro_count_x = MACRO_ZONE_COUNT_X;
    int macro_count_y = MACRO_ZONE_COUNT_Y;
    ZoneType macro_type = ZoneType::SLUM;
    int housing_micro_zone_count = 1;
    int housing_building_count = 1;
    int workplace_micro_zone_count = 0;
    int workplace_building_count = 0;
    int supply_micro_zone_count = 0;
    int supply_building_count = 0;
    int market_micro_zone_count = 0;
    int market_building_count = 0;
    int clinic_micro_zone_count = 0;
    int clinic_building_count = 0;
    int fixed_worker_count = 0;
    int carryable_object_count = 0;
    bool transit_enabled = false;
    float transit_ride_seconds = 4.0f;
    float transit_signal_cycle_seconds = 8.0f;
    float world_phase_interval_seconds = 240.0f;
    WorldPhase initial_world_phase = WorldPhase::DAY;
    uint32_t seed = kDefaultWorldSeed;
};

inline WorldConfig makeSandboxConfig() {
    return {};
}
