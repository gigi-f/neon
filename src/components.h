#pragma once

#include <cstdint>
#include <string>
#include "ecs.h"

enum class ZoneType {
    SLUM
};

enum class MicroZoneRole {
    HOUSING,
    WORKPLACE
};

enum class PathKind {
    PEDESTRIAN
};

enum class InspectionTargetType {
    NONE,
    HOUSING,
    WORKPLACE,
    PEDESTRIAN_PATH,
    WORKER
};

enum class FixedActorKind {
    WORKER
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
    INSIDE_HOUSING,
    NEAR_WORKPLACE,
    INSIDE_WORKPLACE
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

struct BuildingInteractionComponent {
    Entity building_entity = MAX_ENTITIES;
    MicroZoneRole building_role = MicroZoneRole::HOUSING;
    bool inside_building = false;
};

struct InspectionComponent {
    Entity target_entity = MAX_ENTITIES;
    InspectionTargetType target_type = InspectionTargetType::NONE;
    bool has_result = false;
};

struct FixedActorComponent {
    FixedActorKind kind = FixedActorKind::WORKER;
    Entity path_entity = MAX_ENTITIES;
    float route_t = 0.0f;
    float direction = 1.0f;
    float speed_wu = 24.0f;
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

struct PathComponent {
    PathKind kind = PathKind::PEDESTRIAN;
    Entity from = MAX_ENTITIES;
    Entity to = MAX_ENTITIES;
};
