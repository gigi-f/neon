#pragma once
#include "ecs.h"
#include "components.h"
#include <cmath>
#include <random>

class CityPlannerSystem {
public:
    void planZone(Registry& registry, ZoneType type, float startX, float startY, float width, float height) {
        int blocksX = 1, blocksY = 1;
        int lotsX = 1, lotsY = 1;
        bool back_to_back = true;

        switch (type) {
            case ZoneType::URBAN_CORE:
                blocksX = 4; blocksY = 4;
                lotsX = 2; lotsY = 2;
                break;
            case ZoneType::RESIDENTIAL:
                blocksX = 4; blocksY = 2;
                lotsX = 6; lotsY = 3;
                break;
            case ZoneType::SLUM:
                blocksX = 5; blocksY = 3;
                lotsX = 8; lotsY = 3;
                break;
            case ZoneType::CORPORATE:
                blocksX = 2; blocksY = 2;
                lotsX = 2; lotsY = 2;
                break;
            default:
                break;
        }

        float blockWidth = width / blocksX;
        float blockHeight = height / blocksY;
        float roadWidth = 6.0f;
        float pedestrianPathWidth = 2.0f;
        
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (int bx = 0; bx < blocksX; ++bx) {
            for (int by = 0; by < blocksY; ++by) {
                float bStartX = startX + bx * blockWidth;
                float bStartY = startY + by * blockHeight;

                // Occasional pedestrian path instead of secondary road
                float currentRoadWidth = roadWidth;
                if (dist(rng) < 0.1f) { // 10% chance for pedestrian path
                    currentRoadWidth = pedestrianPathWidth;
                    Entity road = registry.create();
                    registry.assign<RoadComponent>(road, RoadType::PEDESTRIAN_PATH, 0.0f);
                    registry.assign<TransformComponent>(road, bStartX, bStartY, currentRoadWidth, blockHeight);
                } else {
                    Entity road = registry.create();
                    registry.assign<RoadComponent>(road, RoadType::SECONDARY, 0.5f);
                    registry.assign<TransformComponent>(road, bStartX, bStartY, currentRoadWidth, blockHeight);
                }

                float usableBlockWidth = blockWidth - currentRoadWidth;
                float usableBlockHeight = blockHeight - currentRoadWidth;

                float lotWidth = usableBlockWidth / lotsX;
                float lotHeight = usableBlockHeight / lotsY;

                for (int lx = 0; lotsX > 0 && lx < lotsX; ++lx) {
                    for (int ly = 0; lotsY > 0 && ly < lotsY; ++ly) {
                        float lStartX = bStartX + currentRoadWidth + lx * lotWidth;
                        float lStartY = bStartY + currentRoadWidth + ly * lotHeight;

                        // Create building entity
                        Entity building = registry.create();
                        registry.assign<TransformComponent>(building, lStartX, lStartY, lotWidth, lotHeight);
                        registry.assign<ZoningComponent>(building, type);
                    }
                }
                
                // Add mid-block alley if back_to_back
                if (back_to_back && lotsX > 1) {
                    Entity alley = registry.create();
                    registry.assign<RoadComponent>(alley, RoadType::ALLEY, 0.1f);
                    registry.assign<TransformComponent>(alley, bStartX + currentRoadWidth + (usableBlockWidth / 2.0f) - 1.0f, bStartY + currentRoadWidth, 2.0f, usableBlockHeight);
                }
            }
        }
    }
};

class CityGenerationSystem {
public:
    void generateBuildings(Registry& registry) {
        std::mt19937 rng(std::random_device{}());

        auto view = registry.view<TransformComponent, ZoningComponent>();
        for (Entity e : view) {
            if (registry.has<BuildingComponent>(e)) continue; // Already generated
            
            auto& transform = registry.get<TransformComponent>(e);
            auto& zoning = registry.get<ZoningComponent>(e);

            // Calculate distance from center (0,0 assumed URBAN_CORE center)
            float dist = std::sqrt(transform.x * transform.x + transform.y * transform.y);
            
            int minFloors = 1, maxFloors = 1;
            
            if (dist < 40.0f) {
                minFloors = 30; maxFloors = 150;
            } else if (dist >= 40.0f && dist < 80.0f) {
                minFloors = 15; maxFloors = 50;
            } else if (dist >= 80.0f && dist < 120.0f) {
                minFloors = 5; maxFloors = 20;
            } else if (dist >= 120.0f && dist < 200.0f) {
                minFloors = 3; maxFloors = 8;
            } else {
                minFloors = 1; maxFloors = 4;
            }

            // Override with zoning specifics if industrial
            if (zoning.type == ZoneType::INDUSTRIAL) {
                minFloors = 1; maxFloors = 5;
            }

            std::uniform_int_distribution<int> floorDist(minFloors, maxFloors);
            int floors = floorDist(rng);

            // Generate 64-bit stable ID based on coordinates
            uint64_t stable_id = std::hash<float>{}(transform.x) ^ (std::hash<float>{}(transform.y) << 1);

            registry.assign<BuildingComponent>(e, stable_id, floors, true);
        }
    }
};
