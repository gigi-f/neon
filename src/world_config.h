#pragma once

#include <cstdint>
#include "components.h"

inline constexpr int MACRO_ZONE_COUNT_X = 1;
inline constexpr int MACRO_ZONE_COUNT_Y = 1;
inline constexpr float MACRO_ZONE_SIZE_WU = 360.0f;
inline constexpr uint32_t kDefaultWorldSeed = 0xC0FFEEu;

struct WorldConfig {
    int macro_count_x = MACRO_ZONE_COUNT_X;
    int macro_count_y = MACRO_ZONE_COUNT_Y;
    ZoneType macro_type = ZoneType::SLUM;
    int housing_micro_zone_count = 1;
    int housing_building_count = 1;
    uint32_t seed = kDefaultWorldSeed;
};

inline WorldConfig makeSandboxConfig() {
    return {};
}
