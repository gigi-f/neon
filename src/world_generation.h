#pragma once
#include "ecs.h"
#include "components.h"
#include <cmath>
#include <random>

inline void generateChicagoCity(Registry& registry, int numCols, int numRows) {
    float blockWidth = 400.0f;
    float blockHeight = 600.0f;
    float roadWidth = 100.0f;
    float alleyWidth = 60.0f; 
    float sidewalkWidth = 20.0f;

    // Center the city somewhat around (0,0) by applying an offset
    float offsetX = -((numCols * (blockWidth + roadWidth)) / 2.0f);
    float offsetY = -((numRows * (blockHeight + roadWidth)) / 2.0f);

    float totalCityHeight = numRows * (blockHeight + roadWidth) + roadWidth;

    // Generate Vertical Roads
    for (int col = 0; col <= numCols; ++col) {
        Entity road = registry.create();
        float cx = col * (blockWidth + roadWidth) - (roadWidth / 2.0f) + offsetX;
        float cy = (numRows * (blockHeight + roadWidth)) / 2.0f - (roadWidth / 2.0f) + offsetY;
        float width = roadWidth;
        float height = totalCityHeight;
        
        registry.assign<TransformComponent>(road, cx, cy, width, height);
        registry.assign<RoadComponent>(road, col % 2 == 0 ? RoadType::PRIMARY : RoadType::SECONDARY, 1.0f);
        registry.assign<PowerConduitComponent>(road, 10000.0f);
    }

    // Generate Horizontal Roads
    for (int row = 0; row <= numRows; ++row) {
        Entity road = registry.create();
        float cy = row * (blockHeight + roadWidth) - (roadWidth / 2.0f) + offsetY;
        float cx = (numCols * (blockWidth + roadWidth)) / 2.0f - (roadWidth / 2.0f) + offsetX;
        float width = numCols * (blockWidth + roadWidth) + roadWidth;
        float height = roadWidth;
        
        registry.assign<TransformComponent>(road, cx, cy, width, height);
        registry.assign<RoadComponent>(road, row % 2 == 0 ? RoadType::PRIMARY : RoadType::SECONDARY, 1.0f);
        registry.assign<PowerConduitComponent>(road, 10000.0f);
    }

    // Generate Continuous Transit Track (Vertical)
    int transitCol = numCols / 2;
    float buildableWidth = blockWidth - 2.0f * sidewalkWidth;
    float lotAreaWidth = (buildableWidth - alleyWidth) / 2.0f;
    float trackX = transitCol * (blockWidth + roadWidth) + offsetX + sidewalkWidth + lotAreaWidth / 2.0f;
    float trackY = offsetY + (numRows * (blockHeight + roadWidth)) / 2.0f - (roadWidth / 2.0f);

    Entity longTrack = registry.create();
    registry.assign<TransformComponent>(longTrack, trackX, trackY, 40.0f, totalCityHeight);
    registry.assign<RoadComponent>(longTrack, RoadType::MAGLIFT_TRACK, 0.0f);

    // Stations at each horizontal road crossing the maglift track (one per row boundary).
    for (int row = 0; row <= numRows; ++row) {
        float stationY = row * (blockHeight + roadWidth) - roadWidth / 2.0f + offsetY;
        Entity station = registry.create();
        registry.assign<TransformComponent>(station, trackX, stationY, 60.0f, 40.0f);
        registry.assign<StationComponent>(station, row, 4.0f);
        registry.assign<GlyphComponent>(station, std::string("[S]"),
            (uint8_t)80, (uint8_t)220, (uint8_t)210, (uint8_t)255, 1.0f, true);

        // Staircase connecting ground to the elevated platform (right side of track).
        Entity stair = registry.create();
        registry.assign<TransformComponent>(stair, trackX + 30.0f, stationY, 20.0f, 40.0f);
        registry.assign<StaircaseComponent>(stair, row);
        registry.assign<RoadComponent>(stair, RoadType::PEDESTRIAN_PATH, 0.0f);

        // Ground-level waiting area where pedestrians queue for the maglift.
        Entity waitArea = registry.create();
        registry.assign<TransformComponent>(waitArea, trackX + 75.0f, stationY, 70.0f, 50.0f);
        registry.assign<WaitingAreaComponent>(waitArea, row);
        registry.assign<RoadComponent>(waitArea, RoadType::PEDESTRIAN_PATH, 0.0f);
    }

    // Two automated transit MAGLIFT vehicles — TransitSystem drives them directly (no MovementComponent).
    // Vehicle A: starts stopped at station 0 (top terminus), heading toward station 1.
    {
        float startY = 0 * (blockHeight + roadWidth) - roadWidth / 2.0f + offsetY;
        Entity ta = registry.create();
        registry.assign<TransformComponent>(ta, trackX, startY, 80.0f, 28.0f);
        registry.assign<VehicleComponent>(ta, VehicleComponent::MAGLIFT, MAX_ENTITIES, 140.0f);
        registry.assign<GlyphComponent>(ta, std::string("M"),
            (uint8_t)255, (uint8_t)80, (uint8_t)140, (uint8_t)255, 0.5f, true, true);
        TransitVehicleComponent tvc{};
        tvc.state            = TransitVehicleComponent::STOPPED;
        tvc.current_station  = 0;
        tvc.next_station     = 1;
        tvc.direction        = 1;
        tvc.stop_timer       = 2.0f;
        tvc.speed            = 140.0f;
        tvc.track_x          = trackX;
        registry.assign<TransitVehicleComponent>(ta, tvc);
    }

    // Vehicle B: starts stopped at station 2 (midpoint), heading toward station 3.
    {
        float startY = 2 * (blockHeight + roadWidth) - roadWidth / 2.0f + offsetY;
        Entity tb = registry.create();
        registry.assign<TransformComponent>(tb, trackX, startY, 80.0f, 28.0f);
        registry.assign<VehicleComponent>(tb, VehicleComponent::MAGLIFT, MAX_ENTITIES, 140.0f);
        registry.assign<GlyphComponent>(tb, std::string("MMMM"),
            (uint8_t)255, (uint8_t)80, (uint8_t)140, (uint8_t)255, 1.0f, true);
        TransitVehicleComponent tvc{};
        tvc.state            = TransitVehicleComponent::STOPPED;
        tvc.current_station  = 2;
        tvc.next_station     = 3;
        tvc.direction        = 1;
        tvc.stop_timer       = 5.0f; // offset so vehicles aren't synchronized
        tvc.speed            = 140.0f;
        tvc.track_x          = trackX;
        registry.assign<TransitVehicleComponent>(tb, tvc);
    }

    std::mt19937 rng(std::random_device{}());

    // Market cluster counter: one market per block cluster
    int marketClusterIndex = 0;

    // Generate Blocks and lots
    for (int col = 0; col < numCols; ++col) {
        for (int row = 0; row < numRows; ++row) {
            float blockCx = col * (blockWidth + roadWidth) + blockWidth / 2.0f + offsetX;
            float blockCy = row * (blockHeight + roadWidth) + blockHeight / 2.0f + offsetY;

            // Subdivide block into lots
            // Chicago style: alley down the middle vertically
            
            // Create Sidewalk covering the entire block
            Entity sidewalk = registry.create();
            registry.assign<TransformComponent>(sidewalk, blockCx, blockCy, blockWidth, blockHeight);
            registry.assign<RoadComponent>(sidewalk, RoadType::PEDESTRIAN_PATH, 0.0f);
            
            // Create Alley road (goes through the entire block to connect to streets)
            Entity alley = registry.create();
            registry.assign<TransformComponent>(alley, blockCx, blockCy, alleyWidth, blockHeight);
            registry.assign<RoadComponent>(alley, RoadType::ALLEY, 0.0f);

            float buildableHeight = blockHeight - 2.0f * sidewalkWidth;
            
            bool isSuperBlock = (std::uniform_int_distribution<int>(0, 9)(rng) < 4); // 40% chance
            bool isTransitHub = (col == transitCol); // Transit line column
            float gapY = isSuperBlock ? 0.0f : 20.0f;
            int numLotsY = isSuperBlock ? 4 : 8;
            float lotHeight = (buildableHeight - (numLotsY - 1) * gapY) / numLotsY;

            // One market per block cluster; first eligible building gets it.
            // MarketComponent carries all three commodity types (food/water/medical).
            bool blockMarketPlaced = false;

            for (int lx = 0; lx < 2; ++lx) {
                for (int ly = 0; ly < numLotsY; ++ly) {
                    float lotWidth = lotAreaWidth;
                    float bx = blockCx - buildableWidth / 2.0f + lx * (lotAreaWidth + alleyWidth) + lotWidth / 2.0f;
                    float by = blockCy - buildableHeight / 2.0f + ly * (lotHeight + gapY) + lotHeight / 2.0f;

                    if (isTransitHub && lx == 0) {
                        // Skip building placement where the continuous track already exists
                        continue; 
                    }
                    
                    Entity building = registry.create();
                    
                    float dist = std::sqrt(bx * bx + by * by);
                    int minFloors = 1, maxFloors = 1;
                    
                    if (dist < 400.0f) {
                        minFloors = 20; maxFloors = 50;
                    } else if (dist < 800.0f) {
                        minFloors = 10; maxFloors = 25;
                    } else if (dist < 1200.0f) {
                        minFloors = 5; maxFloors = 12;
                    } else {
                        minFloors = 1; maxFloors = 4;
                    }

                    // Super-blocks are 50% taller
                    if (isSuperBlock) {
                        minFloors = static_cast<int>(minFloors * 1.5f);
                        maxFloors = static_cast<int>(maxFloors * 1.5f);
                    }
                    
                    std::uniform_int_distribution<int> floorDist(minFloors, maxFloors);
                    int floors = floorDist(rng);
                    
                    uint64_t stable_id = std::hash<float>{}(bx) ^ (std::hash<float>{}(by) << 1);
                    
                    registry.assign<TransformComponent>(building, bx, by, lotWidth, lotHeight);
                    registry.assign<SolidComponent>(building);
                    registry.assign<BuildingComponent>(building, stable_id, floors, true); // Make it enterable for atmosphere logic
                    registry.assign<BuildingAtmosphereComponent>(building, 20.0f, 100.0f);

                    ZoneType zType = ZoneType::RESIDENTIAL;
                    if (dist < 500.0f) zType = ZoneType::URBAN_CORE;
                    else if (dist < 1000.0f) zType = ZoneType::COMMERCIAL;
                    else zType = ZoneType::INDUSTRIAL;
                    registry.assign<ZoningComponent>(building, zType);

                    // Add StructuralComponent based on zone
                    StructuralComponent sc;
                    sc.integrity = 100.0f;
                    sc.is_exposed = true;
                    if (zType == ZoneType::URBAN_CORE) sc.material_type = MaterialType::STEEL;
                    else if (zType == ZoneType::INDUSTRIAL) sc.material_type = MaterialType::REINFORCED_CONCRETE;
                    else if (zType == ZoneType::COMMERCIAL) sc.material_type = MaterialType::COMPOSITE;
                    else sc.material_type = MaterialType::SCRAP;
                    registry.assign<StructuralComponent>(building, sc);

                    // ~30% of buildings are employers
                    {
                        std::uniform_real_distribution<float> chance(0.0f, 1.0f);
                        if (chance(rng) < 0.30f) {
                            std::uniform_int_distribution<int> capDist(3, 8);
                            registry.assign<EmployerComponent>(building, capDist(rng), 0);
                        }
                    }

                    float power_supply = 0.0f;
                    float power_demand = 10.0f * floors;
                    if (zType == ZoneType::INDUSTRIAL && std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) < 0.20f) {
                        power_supply = 100000.0f; 
                        power_demand = 0.0f;
                        registry.assign<GlyphComponent>(building, std::string("G"),
                            (uint8_t)255, (uint8_t)200, (uint8_t)50, (uint8_t)255, 1.0f, true, true);
                    } else {
                        registry.assign<GlyphComponent>(building, std::string("#"),
                            (uint8_t)80, (uint8_t)82, (uint8_t)90, (uint8_t)255, 1.0f, true, true);
                    }
                    registry.assign<PowerNodeComponent>(building, power_supply, power_demand, false);

                    // First building per block cluster becomes a market (all three commodities).
                    if (!blockMarketPlaced) {
                        auto& market = registry.assign<MarketComponent>(building);
                        market.greed_margin = 0.05f + 0.05f * static_cast<float>(marketClusterIndex % 4);
                        // Override glyph to amber-gold '$'
                        registry.get<GlyphComponent>(building).chars = "$";
                        registry.get<GlyphComponent>(building).r = 255;
                        registry.get<GlyphComponent>(building).g = 210;
                        registry.get<GlyphComponent>(building).b = 50;
                        blockMarketPlaced = true;
                    }

                    // Add a horizontal pedestrian path in the gap if not a super-block
                    if (!isSuperBlock && ly < numLotsY - 1) {
                        Entity pPath = registry.create();
                        float pWidth = lotAreaWidth;
                        float pHeight = gapY;
                        float px = bx;
                        float py = by + lotHeight / 2.0f + gapY / 2.0f;
                        registry.assign<TransformComponent>(pPath, px, py, pWidth, pHeight);
                        registry.assign<RoadComponent>(pPath, RoadType::PEDESTRIAN_PATH, 0.0f);
                    }
                }
            }
            
            if (blockMarketPlaced) ++marketClusterIndex;

            // Add traffic lights and intersection entity at each road crossing
            float ix = col * (blockWidth + roadWidth) - roadWidth / 2.0f + offsetX;
            float iy = row * (blockHeight + roadWidth) - roadWidth / 2.0f + offsetY;

            Entity tl1 = registry.create();
            registry.assign<TransformComponent>(tl1, ix - roadWidth / 2.0f - 10.0f, iy - roadWidth / 2.0f - 10.0f, 20.0f, 20.0f);
            registry.assign<TrafficLightComponent>(tl1, TrafficLightComponent::GREEN, 5.0f, Facing::LEFT);

            Entity tl2 = registry.create();
            registry.assign<TransformComponent>(tl2, ix + roadWidth / 2.0f + 10.0f, iy + roadWidth / 2.0f + 10.0f, 20.0f, 20.0f);
            registry.assign<TrafficLightComponent>(tl2, TrafficLightComponent::RED, 7.0f, Facing::DOWN);

            // Intersection entity — FIFO queue managed by IntersectionSystem
            Entity intersection = registry.create();
            registry.assign<TransformComponent>(intersection, ix, iy, roadWidth, roadWidth);
            registry.assign<IntersectionComponent>(intersection);
            registry.assign<StopSignComponent>(intersection);
        }
    }
}
