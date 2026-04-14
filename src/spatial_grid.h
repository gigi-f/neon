#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "ecs.h"
#include "components.h"

// Uniform spatial hash grid for fast AABB proximity queries.
// Rebuilt from SolidComponent entities each frame by MovementSystem.
// Cell size should match world granularity (macro_cell_size = 40 WU).
class SpatialGrid {
public:
    explicit SpatialGrid(float cell_size = 40.0f) : cell_size_(cell_size) {}

    void clear() {
        cells_.clear();
    }

    void insert(Entity e, float cx, float cy, float w, float h) {
        int x0 = to_cell(cx - w / 2.0f);
        int x1 = to_cell(cx + w / 2.0f);
        int y0 = to_cell(cy - h / 2.0f);
        int y1 = to_cell(cy + h / 2.0f);

        for (int x = x0; x <= x1; ++x) {
            for (int y = y0; y <= y1; ++y) {
                cells_[key(x, y)].push_back(e);
            }
        }
    }

    // Returns candidate entities whose cells overlap the queried AABB.
    // Caller must still do exact AABB check (may include false positives).
    std::vector<Entity> query(float cx, float cy, float w, float h) const {
        int x0 = to_cell(cx - w / 2.0f);
        int x1 = to_cell(cx + w / 2.0f);
        int y0 = to_cell(cy - h / 2.0f);
        int y1 = to_cell(cy + h / 2.0f);

        std::vector<Entity> result;
        for (int x = x0; x <= x1; ++x) {
            for (int y = y0; y <= y1; ++y) {
                auto it = cells_.find(key(x, y));
                if (it == cells_.end()) continue;
                for (Entity e : it->second) {
                    result.push_back(e);
                }
            }
        }
        return result;
    }

    void rebuild(Registry& registry) {
        clear();
        auto solids = registry.view<TransformComponent, SolidComponent>();
        for (Entity e : solids) {
            auto& t = registry.get<TransformComponent>(e);
            insert(e, t.x, t.y, t.width, t.height);
        }
    }

private:
    float cell_size_;
    std::unordered_map<uint64_t, std::vector<Entity>> cells_;

    int to_cell(float v) const {
        return static_cast<int>(std::floor(v / cell_size_));
    }

    static uint64_t key(int x, int y) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
                static_cast<uint32_t>(y);
    }
};
