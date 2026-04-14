#pragma once
#include <string>

enum class ZoneType {
    URBAN_CORE,
    CORPORATE,
    COMMERCIAL,
    RESIDENTIAL,
    SLUM,
    INDUSTRIAL
};

enum class RoadType {
    PRIMARY,
    SECONDARY,
    ALLEY,
    PEDESTRIAN_PATH
};

struct WorldConfigComponent {
    int macro_cell_size = 40; // World Units (WU)
    int chunk_size = 80;      // World Units (WU)
};

struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct ZoningComponent {
    ZoneType type;
};

struct BuildingComponent {
    uint64_t stable_id;
    int floors;
    bool is_enterable = true;
};

struct RoadComponent {
    RoadType type;
    float traffic_density = 0.0f;
};

struct TrafficLightComponent {
    enum State { RED, YELLOW, GREEN } state;
    float timer = 0.0f;
};

struct StopSignComponent {
    // FIFO logic is handled by intersection management system
};

struct VehicleComponent {
    enum Type { EMMV, MAGLIFT, TRANSPORT } type;
    Entity driver = MAX_ENTITIES; // MAX_ENTITIES means empty
    float max_speed = 150.0f; // Speed when driven
};

struct HomeLocationComponent {
    float x;
    float y;
};

struct OwnershipComponent {
    Entity owner = MAX_ENTITIES;
};

struct PassengerComponent {
    Entity vehicle = MAX_ENTITIES; // Which vehicle they are in
};

struct CameraComponent {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 2.0f;
    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    Entity target_entity; // Entity to follow
};

enum class Facing { UP, DOWN, LEFT, RIGHT };

struct PlayerComponent {
    float speed = 50.0f; // WU per second
    Facing facing = Facing::DOWN;
};

struct GoalComponent {
    Entity target_entity;
};

struct SolidComponent {
    bool is_solid = true;
};

struct MovementComponent {
    float vx = 0.0f;
    float vy = 0.0f;
    enum SpeedState { NORMAL, HURRY } speed_state = NORMAL;
};
