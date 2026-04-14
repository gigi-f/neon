#include <SDL.h>
#include <iostream>
#include "feature_flags.h"
#include "ecs.h"
#include "components.h"
#include "simulation_systems.h"
#include "simulation_coordinator.h"
#include "world_generation.h"
#include "stb_image.h"

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image " << path << ": " << stbi_failure_reason() << std::endl;
        return nullptr;
    }
    
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        data, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        stbi_image_free(data);
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(data);
    
    return texture;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Neon Oubliette",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Set blend mode for transparent PNG
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Initialize ECS
    Registry registry;
    SimulationCoordinator coordinator;
    PlayerInputSystem playerInput;
    MovementSystem mover;
    CameraSystem cameraSystem;
    TrafficLightSystem tlSystem;
    CitizenAISystem citizenAI;
    AmbientSpawnSystem ambient;
    TrafficDensitySystem trafficDensity;
    IntersectionSystem intersectionSystem;

    // ... player and camera entity creation ...
    Entity player = registry.create();
    registry.assign<TransformComponent>(player, -50.0f, -50.0f, 16.0f, 16.0f); // Size in WU
    registry.assign<MovementComponent>(player, 0.0f, 0.0f, MovementComponent::NORMAL);
    registry.assign<PlayerComponent>(player, 100.0f); // speed in WU/s

    Entity camera = registry.create();
    registry.assign<CameraComponent>(camera, 0.0f, 0.0f, 2.0f, 800.0f, 600.0f, player);

    // ... vehicle creation ...
    Entity pVehicle = registry.create();
    registry.assign<TransformComponent>(pVehicle, -50.0f, -20.0f, 60.0f, 30.0f);
    registry.assign<MovementComponent>(pVehicle, 0.0f, 0.0f, MovementComponent::NORMAL);
    registry.assign<VehicleComponent>(pVehicle, VehicleComponent::MAGLIFT, MAX_ENTITIES, 250.0f);
    registry.assign<SolidComponent>(pVehicle); // Vehicles are solid
    registry.assign<OwnershipComponent>(pVehicle, player);
    registry.assign<HomeLocationComponent>(pVehicle, -50.0f, -20.0f);

    Entity worldConfig = registry.create();
    registry.assign<WorldConfigComponent>(worldConfig);

    generateChicagoCity(registry, 4, 4);

    char* basePath = SDL_GetBasePath();
    std::string base(basePath ? basePath : "");
    if (basePath) SDL_free(basePath);

    SDL_Texture* texDown = loadTexture(renderer, (base + "../assets/sprites/girl/girl_f.png").c_str());
    SDL_Texture* texUp = loadTexture(renderer, (base + "../assets/sprites/girl/girl_b.png").c_str());
    SDL_Texture* texLeft = loadTexture(renderer, (base + "../assets/sprites/girl/girl_l.png").c_str());
    SDL_Texture* texRight = loadTexture(renderer, (base + "../assets/sprites/girl/girl_r.png").c_str());

    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float ambientTimer = 0.0f;

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        bool interactPressed = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_E || event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    interactPressed = true;
                }
            }
        }

        // Extract camera position for this frame's spawn/despawn decisions
        float frameCamX = 0.0f, frameCamY = 0.0f;
        {
            auto cv = registry.view<CameraComponent>();
            for (Entity ce : cv) {
                auto& cam = registry.get<CameraComponent>(ce);
                frameCamX = cam.x; frameCamY = cam.y;
            }
        }

        // Ambient Spawning (camera-radius gated)
        ambientTimer -= dt;
        if (ambientTimer <= 0.0f) {
            ambient.spawnVehicles(registry, frameCamX, frameCamY, 800.0f);
            ambient.spawnCitizens(registry, frameCamX, frameCamY, 600.0f);
            ambientTimer = 2.0f;
        }

        coordinator.advance();

        const Uint8* state = SDL_GetKeyboardState(NULL);
        playerInput.handleInput(registry, state, interactPressed);

        // L0: every frame
        mover.update(registry, dt, registry.get<WorldConfigComponent>(worldConfig));
        cameraSystem.update(registry, dt, state);

        // L0: traffic lights (low overhead, every frame is fine)
        tlSystem.update(registry, dt);

        // L1: traffic density propagation + intersection FIFO at ~6 Hz
        if (coordinator.tick_l1()) {
            trafficDensity.update(registry);
            intersectionSystem.update(registry);
        }

        // L2: citizen AI runs at ~2 Hz
        if (coordinator.tick_l2()) {
            citizenAI.update(registry, dt);
        }

        // Debug HUD via window title (no font dep required)
        if (coordinator.tick_l3()) {
            float fps = (dt > 0.0f) ? 1.0f / dt : 0.0f;
            float camX = 0.0f, camY = 0.0f;
            auto camView = registry.view<CameraComponent>();
            for (Entity ce : camView) {
                auto& cam = registry.get<CameraComponent>(ce);
                camX = cam.x; camY = cam.y;
            }
            char title[128];
            std::snprintf(title, sizeof(title),
                "Neon Oubliette | FPS: %.0f | Entities: %zu | Cam: (%.0f, %.0f) | Frame: %llu",
                fps, registry.entity_count(), camX, camY,
                static_cast<unsigned long long>(coordinator.frame));
            SDL_SetWindowTitle(window, title);
        }

        // Get Camera
        float camX = 0.0f, camY = 0.0f, scale = 2.0f;
        float screenW = 800.0f, screenH = 600.0f;
        auto camView = registry.view<CameraComponent>();
        for (Entity e : camView) {
            auto& cam = registry.get<CameraComponent>(e);
            camX = cam.x;
            camY = cam.y;
            scale = cam.scale;
            screenW = cam.screenWidth;
            screenH = cam.screenHeight;
            ambient.despawnEntities(registry, cam.x, cam.y, 2000.0f);
        }
        
        float centerX = screenW / 2.0f;
        float centerY = screenH / 2.0f;

        // Render Background
        SDL_SetRenderDrawColor(renderer, 40, 42, 45, 255); // Dark Slate
        SDL_RenderClear(renderer);

        // Render Grid (Playable Area Visualizer)
        SDL_SetRenderDrawColor(renderer, 60, 62, 65, 255);
        int gridWU = 20;
        
        int startX = std::floor((camX - (centerX / scale)) / gridWU) * gridWU;
        int endX = std::ceil((camX + (centerX / scale)) / gridWU) * gridWU;
        
        int startY = std::floor((camY - (centerY / scale)) / gridWU) * gridWU;
        int endY = std::ceil((camY + (centerY / scale)) / gridWU) * gridWU;
        
        for (int x = startX; x <= endX; x += gridWU) {
            int screenX = static_cast<int>(centerX + (x - camX) * scale);
            SDL_RenderDrawLine(renderer, screenX, 0, screenX, static_cast<int>(screenH));
        }
        for (int y = startY; y <= endY; y += gridWU) {
            int screenY = static_cast<int>(centerY + (y - camY) * scale);
            SDL_RenderDrawLine(renderer, 0, screenY, static_cast<int>(screenW), screenY);
        }

        // Define screen bounds for culling
        float viewLeft = camX - (centerX / scale);
        float viewRight = camX + (centerX / scale);
        float viewTop = camY - (centerY / scale);
        float viewBottom = camY + (centerY / scale);

        // Render Roads (Pass 1: Ground-level roads, alleys, sidewalks)
        auto roadView = registry.view<TransformComponent, RoadComponent>();
        for (Entity e : roadView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            auto& r = registry.get<RoadComponent>(e);
            if (r.type == RoadType::MAGLIFT_TRACK) continue; // Render in Pass 2

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            if (r.type == RoadType::PRIMARY) {
                SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
                SDL_RenderFillRect(renderer, &destRect);
            } else if (r.type == RoadType::SECONDARY) {
                SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255);
                SDL_RenderFillRect(renderer, &destRect);
            } else if (r.type == RoadType::PEDESTRIAN_PATH) {
                SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
                int cornerRadius = static_cast<int>(5.0f * scale);
                if (cornerRadius * 2 > destRect.w || cornerRadius * 2 > destRect.h) {
                    SDL_RenderFillRect(renderer, &destRect);
                } else {
                    SDL_Rect r1 = { destRect.x + cornerRadius, destRect.y, destRect.w - 2 * cornerRadius, destRect.h };
                    SDL_Rect r2 = { destRect.x, destRect.y + cornerRadius, destRect.w, destRect.h - 2 * cornerRadius };
                    SDL_RenderFillRect(renderer, &r1);
                    SDL_RenderFillRect(renderer, &r2);
                    auto drawCircleFill = [&](int cx, int cy, int radius) {
                        for (int w = 0; w < radius * 2; w++) {
                            for (int h = 0; h < radius * 2; h++) {
                                int dx = radius - w;
                                int dy = radius - h;
                                if ((dx*dx + dy*dy) <= (radius * radius)) {
                                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                                }
                            }
                        }
                    };
                    drawCircleFill(destRect.x + cornerRadius, destRect.y + cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + destRect.w - cornerRadius, destRect.y + cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + cornerRadius, destRect.y + destRect.h - cornerRadius, cornerRadius);
                    drawCircleFill(destRect.x + destRect.w - cornerRadius, destRect.y + destRect.h - cornerRadius, cornerRadius);
                }
            } else if (r.type == RoadType::ALLEY) {
                SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
                SDL_RenderFillRect(renderer, &destRect);
            }
        }

        // Render Obstacles
        auto solidView = registry.view<TransformComponent, SolidComponent>();
        for (Entity e : solidView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 90, 95, 100, 255); // Solid block color
            SDL_RenderFillRect(renderer, &destRect);
            
            // Draw a border for clarity
            SDL_SetRenderDrawColor(renderer, 150, 155, 160, 255);
            SDL_RenderDrawRect(renderer, &destRect);
        }

        // Render Ground Vehicles
        auto vehicleView = registry.view<TransformComponent, VehicleComponent>();
        for (Entity e : vehicleView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }
            auto& v = registry.get<VehicleComponent>(e);
            
            if (v.type == VehicleComponent::MAGLIFT) continue; // Render in Pass 2

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_RenderFillRect(renderer, &destRect);
            
            // Draw driver indicator (white dot) if someone is driving
            if (v.driver != MAX_ENTITIES) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect driverRect = {
                    static_cast<int>(centerX + (t.x - camX) * scale - (4.0f * scale) / 2.0f),
                    static_cast<int>(centerY + (t.y - camY) * scale - (4.0f * scale) / 2.0f),
                    static_cast<int>(4.0f * scale),
                    static_cast<int>(4.0f * scale)
                };
                SDL_RenderFillRect(renderer, &driverRect);
            }
        }

        // Render Citizens
        auto citizenView = registry.view<TransformComponent, CitizenComponent>();
        for (Entity e : citizenView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255); // Light blue
            SDL_RenderFillRect(renderer, &destRect);
        }

        // Render Player
        auto playerView = registry.view<TransformComponent, PlayerComponent>();
        for (Entity e : playerView) {
            if (registry.has<PassengerComponent>(e) && registry.get<PassengerComponent>(e).vehicle != MAX_ENTITIES) {
                continue; // Don't render player if inside a vehicle
            }
            auto& t = registry.get<TransformComponent>(e);
            
            // Player is usually near center, but cull anyway for consistency
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            auto& p = registry.get<PlayerComponent>(e);
            
            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };

            SDL_Texture* activeTex = texDown;
            if (p.facing == Facing::UP) activeTex = texUp;
            else if (p.facing == Facing::LEFT) activeTex = texLeft;
            else if (p.facing == Facing::RIGHT) activeTex = texRight;

            if (activeTex) {
                SDL_RenderCopy(renderer, activeTex, NULL, &destRect);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
                SDL_RenderFillRect(renderer, &destRect);
            }
        }

        // Render Infrastructure (Pass 2: Maglift tracks, etc.)
        for (Entity e : roadView) {
            auto& t = registry.get<TransformComponent>(e);
            auto& r = registry.get<RoadComponent>(e);
            if (r.type != RoadType::MAGLIFT_TRACK) continue;

            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255); // Cyan track
            SDL_RenderFillRect(renderer, &destRect);
            // Draw some rails
            SDL_SetRenderDrawColor(renderer, 200, 240, 255, 255);
            SDL_Rect rail1 = { destRect.x + destRect.w / 4, destRect.y, 2, destRect.h };
            SDL_Rect rail2 = { destRect.x + 3 * destRect.w / 4, destRect.y, 2, destRect.h };
            SDL_RenderFillRect(renderer, &rail1);
            SDL_RenderFillRect(renderer, &rail2);
        }

        // Render Elevated Vehicles (Maglift)
        for (Entity e : vehicleView) {
            auto& t = registry.get<TransformComponent>(e);
            auto& v = registry.get<VehicleComponent>(e);
            
            if (v.type != VehicleComponent::MAGLIFT) continue;

            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Pinkish red

            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            SDL_RenderFillRect(renderer, &destRect);
            
            // Draw driver indicator (white dot) if someone is driving
            if (v.driver != MAX_ENTITIES) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect driverRect = {
                    static_cast<int>(centerX + (t.x - camX) * scale - (4.0f * scale) / 2.0f),
                    static_cast<int>(centerY + (t.y - camY) * scale - (4.0f * scale) / 2.0f),
                    static_cast<int>(4.0f * scale),
                    static_cast<int>(4.0f * scale)
                };
                SDL_RenderFillRect(renderer, &driverRect);
            }
        }

        // Render Traffic Lights (Elevated)
        auto tlView = registry.view<TransformComponent, TrafficLightComponent>();
        for (Entity e : tlView) {
            auto& t = registry.get<TransformComponent>(e);
            
            // Frustum Culling
            if (t.x + t.width/2.0f < viewLeft || t.x - t.width/2.0f > viewRight ||
                t.y + t.height/2.0f < viewTop || t.y - t.height/2.0f > viewBottom) {
                continue;
            }

            auto& tl = registry.get<TrafficLightComponent>(e);
            SDL_Rect destRect = {
                static_cast<int>(centerX + (t.x - camX) * scale - (t.width * scale) / 2.0f),
                static_cast<int>(centerY + (t.y - camY) * scale - (t.height * scale) / 2.0f),
                static_cast<int>(t.width * scale),
                static_cast<int>(t.height * scale)
            };
            if (tl.state == TrafficLightComponent::GREEN) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            } else if (tl.state == TrafficLightComponent::YELLOW) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            SDL_RenderFillRect(renderer, &destRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &destRect);

            // Draw facing indicator
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            int indicatorSize = static_cast<int>(4.0f * scale);
            SDL_Rect indicatorRect = destRect;
            if (tl.facing == Facing::UP) {
                indicatorRect.h = indicatorSize;
            } else if (tl.facing == Facing::DOWN) {
                indicatorRect.y = destRect.y + destRect.h - indicatorSize;
                indicatorRect.h = indicatorSize;
            } else if (tl.facing == Facing::LEFT) {
                indicatorRect.w = indicatorSize;
            } else if (tl.facing == Facing::RIGHT) {
                indicatorRect.x = destRect.x + destRect.w - indicatorSize;
                indicatorRect.w = indicatorSize;
            }
            SDL_RenderFillRect(renderer, &indicatorRect);
        }

        SDL_RenderPresent(renderer);
    }

    if (texDown) SDL_DestroyTexture(texDown);
    if (texUp) SDL_DestroyTexture(texUp);
    if (texLeft) SDL_DestroyTexture(texLeft);
    if (texRight) SDL_DestroyTexture(texRight);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}