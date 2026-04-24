#pragma once

#include <cstdint>
#include <string>
#include "ecs.h"

enum class ZoneType {
    SLUM
};

enum class MicroZoneRole {
    HOUSING
};

enum class Facing {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class PlayerLocationState {
    OUTSIDE,
    NEAR_HOUSING,
    INSIDE_HOUSING
};

struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct CameraComponent {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 2.0f;
    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    Entity target_entity = MAX_ENTITIES;
};

struct PlayerComponent {
    float speed = 90.0f;
    Facing facing = Facing::DOWN;
};

struct HousingInteractionComponent {
    Entity housing_entity = MAX_ENTITIES;
    bool inside_housing = false;
};

struct SolidComponent {
    bool is_solid = true;
};

struct GlyphComponent {
    std::string chars;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    float scale = 1.0f;
    bool centered = true;
    bool tiled = false;
};

struct MacroZoneComponent {
    uint32_t macro_id = 0;
    ZoneType type = ZoneType::SLUM;
    int grid_x = 0;
    int grid_y = 0;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
};

struct MicroZoneComponent {
    Entity parent_macro = MAX_ENTITIES;
    MicroZoneRole role = MicroZoneRole::HOUSING;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
};

struct BuildingComponent {
    uint64_t stable_id = 0;
    int floors = 1;
    bool is_enterable = true;
};

struct BuildingUseComponent {
    MicroZoneRole role = MicroZoneRole::HOUSING;
};
