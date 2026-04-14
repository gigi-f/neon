#pragma once
#include <string>
#include <array>

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
    PEDESTRIAN_PATH,
    MAGLIFT_TRACK
};

struct WorldConfigComponent {
    int macro_cell_size = 40;      // World Units (WU)
    int chunk_size = 80;           // World Units (WU)
    float world_min = -1000.0f;    // World boundary minimum (WU)
    float world_max =  1000.0f;    // World boundary maximum (WU)
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

enum class Facing { UP, DOWN, LEFT, RIGHT };

struct TrafficLightComponent {
    enum State { RED, YELLOW, GREEN } state;
    float timer = 0.0f;
    Facing facing = Facing::DOWN;
};

struct StopSignComponent {
    // FIFO logic is handled by intersection management system
};

// Represents a road intersection; holds a FIFO queue of up to 8 waiting vehicles.
// The front-of-queue vehicle gets the right-of-way; it must dequeue itself once clear.
struct IntersectionComponent {
    static constexpr int QUEUE_CAP = 8;
    std::array<Entity, QUEUE_CAP> queue{};
    int head = 0; // index of the front waiting vehicle
    int size = 0; // number of vehicles in queue

    // Returns MAX_ENTITIES if queue empty, else the entity at the front.
    Entity front() const { return (size > 0) ? queue[head % QUEUE_CAP] : MAX_ENTITIES; }

    bool enqueue(Entity e) {
        if (size >= QUEUE_CAP) return false;
        queue[(head + size) % QUEUE_CAP] = e;
        ++size;
        return true;
    }

    void dequeue() {
        if (size > 0) { ++head; --size; }
    }

    bool contains(Entity e) const {
        for (int i = 0; i < size; ++i)
            if (queue[(head + i) % QUEUE_CAP] == e) return true;
        return false;
    }
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

struct CitizenComponent {
    Facing facing = Facing::DOWN;
    float speed = 30.0f;
    float wander_timer = 0.0f;
};

struct MovementComponent {
    float vx = 0.0f;
    float vy = 0.0f;
    enum SpeedState { NORMAL, HURRY } speed_state = NORMAL;
};
