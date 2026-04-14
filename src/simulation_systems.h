#pragma once
#include "ecs.h"
#include "components.h"
#include "spatial_grid.h"
#include <random>
#include <cmath>

class AmbientSpawnSystem {
private:
    std::vector<Entity> cachedRoads;
    bool initialized = false;

    void initialize(Registry& registry) {
        if (initialized) return;
        cachedRoads = registry.view<TransformComponent, RoadComponent>();
        initialized = true;
    }

public:
    void spawnVehicles(Registry& registry, float camX, float camY, float spawnRadius) {
        initialize(registry);
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (Entity e : cachedRoads) {
            auto& road = registry.get<RoadComponent>(e);
            auto& transform = registry.get<TransformComponent>(e);

            // Only spawn near the camera
            float dx = transform.x - camX;
            float dy = transform.y - camY;
            if (dx * dx + dy * dy > spawnRadius * spawnRadius) continue;

            if (road.traffic_density > 0.0f && dist(rng) < 0.05f * road.traffic_density) {
                Entity vehicle = registry.create();

                // Determine vehicle type — TRANSPORT is a heavy hauler
                VehicleComponent::Type vType = VehicleComponent::EMMV;
                float length = 18.0f;
                float breadth = 9.0f;
                if (dist(rng) < 0.2f) {
                    vType = VehicleComponent::TRANSPORT;
                    length = 48.0f;
                    breadth = 14.0f;
                }

                registry.assign<VehicleComponent>(vehicle, vType);
                registry.assign<TransformComponent>(vehicle, transform.x, transform.y + 1.0f, length, breadth);
                registry.assign<MovementComponent>(vehicle, 0.5f, 0.0f, MovementComponent::NORMAL);
                registry.assign<SolidComponent>(vehicle); // vehicles block each other
            }
        }
    }

    void spawnCitizens(Registry& registry, float camX, float camY, float spawnRadius) {
        initialize(registry);
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (Entity e : cachedRoads) {
            auto& road = registry.get<RoadComponent>(e);
            auto& transform = registry.get<TransformComponent>(e);

            // Only spawn near the camera
            float dx = transform.x - camX;
            float dy = transform.y - camY;
            if (dx * dx + dy * dy > spawnRadius * spawnRadius) continue;

            if (road.type == RoadType::PEDESTRIAN_PATH && dist(rng) < 0.2f) {
                Entity citizen = registry.create();

                std::uniform_real_distribution<float> xDist(transform.x - transform.width / 2.0f, transform.x + transform.width / 2.0f);
                std::uniform_real_distribution<float> yDist(transform.y - transform.height / 2.0f, transform.y + transform.height / 2.0f);

                registry.assign<CitizenComponent>(citizen);
                registry.assign<TransformComponent>(citizen, xDist(rng), yDist(rng), 8.0f, 8.0f);
                registry.assign<MovementComponent>(citizen, 0.0f, 0.0f, MovementComponent::NORMAL);
            }
        }
    }

    void despawnEntities(Registry& registry, float cameraX, float cameraY, float maxRadius) {
        std::vector<Entity> toDestroy;

        auto vehicleView = registry.view<TransformComponent, VehicleComponent>();
        for (Entity e : vehicleView) {
            // Don't despawn vehicles with owners or drivers if they are near player
            auto& v = registry.get<VehicleComponent>(e);
            if (v.driver != MAX_ENTITIES) continue;

            auto& t = registry.get<TransformComponent>(e);
            float d = std::sqrt(std::pow(t.x - cameraX, 2) + std::pow(t.y - cameraY, 2));
            if (d > maxRadius) {
                toDestroy.push_back(e);
            }
        }

        auto citizenView = registry.view<TransformComponent, CitizenComponent>();
        for (Entity e : citizenView) {
            auto& t = registry.get<TransformComponent>(e);
            float d = std::sqrt(std::pow(t.x - cameraX, 2) + std::pow(t.y - cameraY, 2));
            if (d > maxRadius) {
                toDestroy.push_back(e);
            }
        }

        for (Entity e : toDestroy) {
            registry.destroy(e);
        }
    }
};

class CitizenAISystem {
public:
    void update(Registry& registry, float dt) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        auto view = registry.view<CitizenComponent, MovementComponent, TransformComponent>();
        for (Entity e : view) {
            auto& c = registry.get<CitizenComponent>(e);
            auto& m = registry.get<MovementComponent>(e);
            auto& t = registry.get<TransformComponent>(e);

            c.wander_timer -= dt;
            if (c.wander_timer <= 0.0f) {
                // Choose a new direction or stay idle
                c.wander_timer = 2.0f + dist(rng) * 3.0f;
                
                if (dist(rng) < 0.7f) {
                    // Start moving
                    float angle = dist(rng) * 2.0f * 3.14159f;
                    m.vx = std::cos(angle) * c.speed;
                    m.vy = std::sin(angle) * c.speed;
                    
                    if (std::abs(m.vx) > std::abs(m.vy)) {
                        c.facing = (m.vx > 0) ? Facing::RIGHT : Facing::LEFT;
                    } else {
                        c.facing = (m.vy > 0) ? Facing::DOWN : Facing::UP;
                    }
                } else {
                    // Stay idle
                    m.vx = 0.0f;
                    m.vy = 0.0f;
                }
            }
            
            // Basic collision avoidance or stay on sidewalks could be added here
            // For now, they just wander freely.
        }
    }
};

class CameraSystem {
public:
    void update(Registry& registry, float dt, const Uint8* keyboardState) {
        auto view = registry.view<CameraComponent>();
        for (Entity e : view) {
            auto& cam = registry.get<CameraComponent>(e);
            if (registry.has<TransformComponent>(cam.target_entity)) {
                auto& targetT = registry.get<TransformComponent>(cam.target_entity);
                // Simple follow
                cam.x = targetT.x;
                cam.y = targetT.y;
            }
            
            if (keyboardState[SDL_SCANCODE_EQUALS] || keyboardState[SDL_SCANCODE_KP_PLUS]) {
                cam.scale += 2.0f * dt;
                if (cam.scale > 10.0f) cam.scale = 10.0f;
            }
            if (keyboardState[SDL_SCANCODE_MINUS] || keyboardState[SDL_SCANCODE_KP_MINUS]) {
                cam.scale -= 2.0f * dt;
                if (cam.scale < 0.5f) cam.scale = 0.5f;
            }
        }
    }
};

class PlayerInputSystem {
public:
    void handleInput(Registry& registry, const Uint8* keyboardState, bool interactPressed) {
        auto view = registry.view<PlayerComponent, MovementComponent, TransformComponent>();
        for (Entity e : view) {
            auto& p = registry.get<PlayerComponent>(e);
            auto& m = registry.get<MovementComponent>(e);
            auto& t = registry.get<TransformComponent>(e);
            
            bool inVehicle = registry.has<PassengerComponent>(e) && registry.get<PassengerComponent>(e).vehicle != MAX_ENTITIES;
            
            // Check interaction
            if (interactPressed) {
                if (inVehicle) {
                    // Exit vehicle
                    auto& pass = registry.get<PassengerComponent>(e);
                    if (registry.has<VehicleComponent>(pass.vehicle)) {
                        auto& v = registry.get<VehicleComponent>(pass.vehicle);
                        if (v.driver == e) v.driver = MAX_ENTITIES;
                    }
                    
                    // Remove passenger component so they are walking again
                    pass.vehicle = MAX_ENTITIES;
                    
                    // Offset player enough so they don't get stuck inside the vehicle AABB
                    t.y += 20.0f;
                } else {
                    // Enter vehicle
                    auto vehicles = registry.view<VehicleComponent, TransformComponent>();
                    float closestDist = 40.0f; // Interaction radius
                    Entity closestVehicle = MAX_ENTITIES;
                    
                    for (Entity vEnt : vehicles) {
                        auto& vt = registry.get<TransformComponent>(vEnt);
                        auto& veh = registry.get<VehicleComponent>(vEnt);
                        
                        float dist = std::sqrt(std::pow(t.x - vt.x, 2) + std::pow(t.y - vt.y, 2));
                        if (dist < closestDist && veh.driver == MAX_ENTITIES) {
                            closestDist = dist;
                            closestVehicle = vEnt;
                        }
                    }
                    
                    if (closestVehicle != MAX_ENTITIES) {
                        if (!registry.has<PassengerComponent>(e)) {
                            registry.assign<PassengerComponent>(e, closestVehicle);
                        } else {
                            registry.get<PassengerComponent>(e).vehicle = closestVehicle;
                        }
                        
                        registry.get<VehicleComponent>(closestVehicle).driver = e;
                    }
                }
            }
            
            // Determine active speed and movement
            float activeSpeed = p.speed;
            inVehicle = registry.has<PassengerComponent>(e) && registry.get<PassengerComponent>(e).vehicle != MAX_ENTITIES;
            
            if (inVehicle) {
                Entity vEnt = registry.get<PassengerComponent>(e).vehicle;
                auto& veh = registry.get<VehicleComponent>(vEnt);
                activeSpeed = veh.max_speed;
                
                // We actually apply velocity to the vehicle, not the player!
                auto& vm = registry.get<MovementComponent>(vEnt);
                vm.vx = 0.0f;
                vm.vy = 0.0f;
                
                if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP]) vm.vy = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN]) vm.vy = activeSpeed;
                if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT]) vm.vx = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) vm.vx = activeSpeed;
                
                if (vm.vx != 0.0f && vm.vy != 0.0f) {
                    float length = std::sqrt(vm.vx * vm.vx + vm.vy * vm.vy);
                    vm.vx = (vm.vx / length) * activeSpeed;
                    vm.vy = (vm.vy / length) * activeSpeed;
                }
                
                // Zero out player's personal velocity
                m.vx = 0.0f;
                m.vy = 0.0f;
            } else {
                m.vx = 0.0f;
                m.vy = 0.0f;
                
                if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP]) m.vy = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN]) m.vy = activeSpeed;
                if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT]) m.vx = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) m.vx = activeSpeed;
                
                if (m.vx != 0.0f && m.vy != 0.0f) {
                    float length = std::sqrt(m.vx * m.vx + m.vy * m.vy);
                    m.vx = (m.vx / length) * activeSpeed;
                    m.vy = (m.vy / length) * activeSpeed;
                }
                
                if (m.vx != 0.0f || m.vy != 0.0f) {
                    if (std::abs(m.vx) > std::abs(m.vy)) {
                        p.facing = (m.vx > 0) ? Facing::RIGHT : Facing::LEFT;
                    } else {
                        p.facing = (m.vy > 0) ? Facing::DOWN : Facing::UP;
                    }
                }
            }
        }
    }
};

class MovementSystem {
public:
    // Rebuild the spatial grid from solid entities, then process movement.
    void update(Registry& registry, float dt, const WorldConfigComponent& world) {
        grid_.rebuild(registry);

        auto view = registry.view<TransformComponent, MovementComponent>();
        for (Entity e : view) {
            auto& t = registry.get<TransformComponent>(e);
            auto& m = registry.get<MovementComponent>(e);

            bool inVehicle = registry.has<PassengerComponent>(e) && registry.get<PassengerComponent>(e).vehicle != MAX_ENTITIES;

            if (inVehicle) {
                // Snap to vehicle
                Entity vEnt = registry.get<PassengerComponent>(e).vehicle;
                if (registry.has<TransformComponent>(vEnt)) {
                    auto& vt = registry.get<TransformComponent>(vEnt);
                    t.x = vt.x;
                    t.y = vt.y;
                }
                continue; // Skip collision and movement for passenger
            }

            float nextX = t.x + m.vx * dt;
            float nextY = t.y + m.vy * dt;

            // X-axis check
            if (!checkCollision(nextX, t.y, t.width, t.height, registry, e)) {
                t.x = nextX;
            }
            // Y-axis check
            if (!checkCollision(t.x, nextY, t.width, t.height, registry, e)) {
                t.y = nextY;
            }

            // World bounds
            if (t.x < world.world_min) t.x = world.world_min;
            if (t.x > world.world_max) t.x = world.world_max;
            if (t.y < world.world_min) t.y = world.world_min;
            if (t.y > world.world_max) t.y = world.world_max;
        }
    }

private:
    SpatialGrid grid_;

    // Query grid for nearby solids, then do exact AABB check on candidates only.
    // Reduces from O(n_solid) to O(candidates_in_cells) per moving entity.
    bool checkCollision(float cx, float cy, float w, float h, Registry& registry, Entity self) {
        for (Entity e : grid_.query(cx, cy, w, h)) {
            if (e == self) continue;
            if (!registry.has<TransformComponent>(e)) continue;
            auto& o = registry.get<TransformComponent>(e);
            if ((cx - w/2 < o.x + o.width/2) && (cx + w/2 > o.x - o.width/2) &&
                (cy - h/2 < o.y + o.height/2) && (cy + h/2 > o.y - o.height/2)) {
                return true;
            }
        }
        return false;
    }
};

// Manages FIFO right-of-way at intersections (runs at L1, ~6 Hz).
// Vehicles inside an intersection AABB are enqueued; only the front-of-queue
// vehicle may proceed (its vx/vy are left alone). All others are halted.
class IntersectionSystem {
public:
    void update(Registry& registry) {
        auto intersections = registry.view<TransformComponent, IntersectionComponent>();
        auto vehicles = registry.view<TransformComponent, VehicleComponent, MovementComponent>();

        for (Entity iEnt : intersections) {
            auto& it = registry.get<TransformComponent>(iEnt);
            auto& ic = registry.get<IntersectionComponent>(iEnt);

            float left  = it.x - it.width  / 2.0f;
            float right = it.x + it.width  / 2.0f;
            float top   = it.y - it.height / 2.0f;
            float bot   = it.y + it.height / 2.0f;

            // Purge vehicles that have left the intersection
            for (int i = 0; i < ic.size; ) {
                Entity v = ic.queue[(ic.head + i) % IntersectionComponent::QUEUE_CAP];
                bool still_inside = false;
                if (registry.has<TransformComponent>(v)) {
                    auto& vt = registry.get<TransformComponent>(v);
                    still_inside = vt.x > left && vt.x < right && vt.y > top && vt.y < bot;
                }
                if (!still_inside) {
                    // Compact: shift this slot out
                    for (int j = i; j < ic.size - 1; ++j)
                        ic.queue[(ic.head + j) % IntersectionComponent::QUEUE_CAP] =
                            ic.queue[(ic.head + j + 1) % IntersectionComponent::QUEUE_CAP];
                    --ic.size;
                    // Don't increment i — recheck the slot
                } else {
                    ++i;
                }
            }

            // Enqueue newly-arrived vehicles
            for (Entity v : vehicles) {
                if (ic.contains(v)) continue;
                auto& vt = registry.get<TransformComponent>(v);
                if (vt.x > left && vt.x < right && vt.y > top && vt.y < bot) {
                    ic.enqueue(v);
                }
            }

            // Halt all queued vehicles except the front-of-queue
            Entity leader = ic.front();
            for (int i = 0; i < ic.size; ++i) {
                Entity v = ic.queue[(ic.head + i) % IntersectionComponent::QUEUE_CAP];
                if (v == leader) continue; // leader keeps its velocity
                if (registry.has<MovementComponent>(v)) {
                    auto& vm = registry.get<MovementComponent>(v);
                    vm.vx = 0.0f;
                    vm.vy = 0.0f;
                }
            }
        }
    }
};

// Updates road traffic_density by counting nearby vehicles (runs at L1, ~6 Hz)
class TrafficDensitySystem {
public:
    void update(Registry& registry) {
        // Gather vehicle positions once
        auto vehicleView = registry.view<TransformComponent, VehicleComponent>();
        std::vector<std::pair<float, float>> vehiclePos;
        for (Entity v : vehicleView) {
            auto& t = registry.get<TransformComponent>(v);
            vehiclePos.emplace_back(t.x, t.y);
        }

        // For each road, count vehicles within 1.5x its half-extents
        auto roadView = registry.view<TransformComponent, RoadComponent>();
        for (Entity r : roadView) {
            auto& rt = registry.get<TransformComponent>(r);
            auto& road = registry.get<RoadComponent>(r);
            if (road.type == RoadType::PEDESTRIAN_PATH || road.type == RoadType::MAGLIFT_TRACK) continue;

            float hw = rt.width * 1.5f;
            float hh = rt.height * 1.5f;
            int count = 0;
            for (auto& [vx, vy] : vehiclePos) {
                if (std::abs(vx - rt.x) < hw && std::abs(vy - rt.y) < hh) ++count;
            }
            // Normalize: 0 vehicles → 0.0, 5+ vehicles → 1.0
            float density = std::min(1.0f, count / 5.0f);
            // Smooth toward new value
            road.traffic_density = road.traffic_density * 0.7f + density * 0.3f;
        }
    }
};

class TrafficLightSystem {
public:
    void update(Registry& registry, float dt) {
        auto view = registry.view<TrafficLightComponent>();
        for (Entity e : view) {
            auto& tl = registry.get<TrafficLightComponent>(e);
            tl.timer -= dt;
            if (tl.timer <= 0.0f) {
                if (tl.state == TrafficLightComponent::GREEN) {
                    tl.state = TrafficLightComponent::YELLOW;
                    tl.timer = 2.0f; // 2 seconds yellow
                } else if (tl.state == TrafficLightComponent::YELLOW) {
                    tl.state = TrafficLightComponent::RED;
                    tl.timer = 7.0f; // 7 seconds red
                } else if (tl.state == TrafficLightComponent::RED) {
                    tl.state = TrafficLightComponent::GREEN;
                    tl.timer = 5.0f; // 5 seconds green
                }
            }
        }
    }
};