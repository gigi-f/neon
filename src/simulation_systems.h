#pragma once
#include "ecs.h"
#include "components.h"
#include <random>
#include <cmath>

class AmbientSpawnSystem {
public:
    void spawnVehicles(Registry& registry) {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        auto view = registry.view<TransformComponent, RoadComponent>();
        for (Entity e : view) {
            auto& road = registry.get<RoadComponent>(e);
            auto& transform = registry.get<TransformComponent>(e);

            if (road.traffic_density > 0.0f && dist(rng) < 0.01f * road.traffic_density) {
                // Spawn a vehicle
                Entity vehicle = registry.create();
                
                // Determine vehicle type
                VehicleComponent::Type vType = VehicleComponent::EMMV;
                float length = 6.0f; // WU
                if (dist(rng) < 0.3f) {
                    vType = VehicleComponent::MAGLIFT;
                    length = 12.0f;
                }
                
                registry.assign<VehicleComponent>(vehicle, vType);
                registry.assign<TransformComponent>(vehicle, transform.x, transform.y + 1.0f, length, 3.0f);
                registry.assign<MovementComponent>(vehicle, 0.5f, 0.0f, MovementComponent::NORMAL);
            }
        }
    }

    void despawnVehicles(Registry& registry, float cameraX, float cameraY, float maxRadius) {
        auto view = registry.view<TransformComponent, VehicleComponent>();
        for (Entity e : view) {
            auto& t = registry.get<TransformComponent>(e);
            float dist = std::sqrt(std::pow(t.x - cameraX, 2) + std::pow(t.y - cameraY, 2));
            if (dist > maxRadius) {
                // In a full ECS we would destroy the entity here.
                // For simplicity in this prototype, we just move them out of bounds 
                // or disable them, or let them drift. Since our minimal ECS lacks a 
                // robust delete(), we could flag them as despawned.
            }
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
    bool checkCollision(float nextX, float nextY, float width, float height, Registry& registry, Entity self) {
        auto solidView = registry.view<TransformComponent, SolidComponent>();
        for (Entity e : solidView) {
            if (e == self) continue;
            auto& otherT = registry.get<TransformComponent>(e);
            
            // AABB Collision (assuming x, y are center points)
            float leftA = nextX - width / 2.0f;
            float rightA = nextX + width / 2.0f;
            float topA = nextY - height / 2.0f;
            float bottomA = nextY + height / 2.0f;

            float leftB = otherT.x - otherT.width / 2.0f;
            float rightB = otherT.x + otherT.width / 2.0f;
            float topB = otherT.y - otherT.height / 2.0f;
            float bottomB = otherT.y + otherT.height / 2.0f;

            if (leftA < rightB && rightA > leftB && topA < bottomB && bottomA > topB) {
                return true; // Collision detected
            }
        }
        return false;
    }

    void update(Registry& registry, float dt) {
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

            // Screen boundary check (World bounds)
            if (t.x < -1000.0f) t.x = -1000.0f;
            if (t.x > 1000.0f) t.x = 1000.0f;
            if (t.y < -1000.0f) t.y = -1000.0f;
            if (t.y > 1000.0f) t.y = 1000.0f;
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
                    tl.timer = 5.0f; // 5 seconds red
                } else if (tl.state == TrafficLightComponent::RED) {
                    tl.state = TrafficLightComponent::GREEN;
                    tl.timer = 5.0f; // 5 seconds green
                }
            }
        }
    }
};