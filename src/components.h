#pragma once

#include <cstdint>
#include <string>
#include "ecs.h"

enum class ZoneType {
    SLUM
};

enum class MicroZoneRole {
    HOUSING,
    WORKPLACE,
    SUPPLY,
    MARKET,
    CLINIC
};

enum class PathKind {
    PEDESTRIAN
};

enum class PathState {
    LIT
};

enum class InspectionTargetType {
    NONE,
    HOUSING,
    WORKPLACE,
    SUPPLY,
    MARKET,
    CLINIC,
    PEDESTRIAN_PATH,
    ROUTE_SIGNPOST,
    WORKER,
    HOUSING_INTERIOR,
    WORKPLACE_INTERIOR,
    SUPPLY_INTERIOR,
    CARRYABLE_OBJECT
};

enum class FixedActorKind {
    WORKER
};

enum class ItemKind {
    SUPPLY,
    PART
};

inline const char* itemKindDisplayName(ItemKind kind) {
    switch (kind) {
        case ItemKind::SUPPLY: return "SUPPLY";
        case ItemKind::PART: return "PART";
    }
    return "UNKNOWN";
}

inline const char* itemKindSaveName(ItemKind kind) {
    switch (kind) {
        case ItemKind::SUPPLY: return "SUPPLY";
        case ItemKind::PART: return "PART";
    }
    return "UNKNOWN";
}

inline bool itemKindFromSaveName(const std::string& name, ItemKind& kind) {
    if (name == "SUPPLY") {
        kind = ItemKind::SUPPLY;
        return true;
    }
    if (name == "PART") {
        kind = ItemKind::PART;
        return true;
    }
    return false;
}

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
    INSIDE_WORKPLACE,
    NEAR_SUPPLY,
    INSIDE_SUPPLY
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
    Entity carried_object = MAX_ENTITIES;
};

enum class InheritedGadgetResultKind {
    DEBUGGER,
    INTERFERENCE_TORCH
};

enum class LocalSuspicionCause {
    NONE,
    MISSING_PART,
    ROUTE_TAMPERING
};

enum class LocalSuspicionResolution {
    NONE,
    RETURNED_OUTPUT,
    CORRECTED_ROUTE,
    HIDDEN_ITEM
};

struct InheritedGadgetComponent {
    bool available = true;
    std::string label = "MOTHER'S DEBUGGER";
    std::string last_result;
    InheritedGadgetResultKind last_result_kind = InheritedGadgetResultKind::DEBUGGER;
    Entity last_result_target_entity = MAX_ENTITIES;
    InspectionTargetType last_result_target_type = InspectionTargetType::NONE;
};

struct CarryableComponent {
    ItemKind kind = ItemKind::SUPPLY;
};

struct ShelterStockComponent {
    int current_supply = 0;
    int capacity = 1;
};

struct BuildingImprovementComponent {
    bool improved = false;
};

enum class WorkplaceBenchState {
    EMPTY,
    STOCKED,
    OUTPUT_READY
};

struct WorkplaceBenchComponent {
    WorkplaceBenchState state = WorkplaceBenchState::EMPTY;
};

struct BuildingInteractionComponent {
    Entity building_entity = MAX_ENTITIES;
    MicroZoneRole building_role = MicroZoneRole::HOUSING;
    bool inside_building = false;
    TransformComponent exterior_position{};
    TransformComponent interior_position{0.0f, 0.0f, 12.0f, 12.0f};
};

struct InspectionComponent {
    Entity target_entity = MAX_ENTITIES;
    InspectionTargetType target_type = InspectionTargetType::NONE;
    bool has_result = false;
};

struct FixedActorComponent {
    FixedActorKind kind = FixedActorKind::WORKER;
    Entity path_entity = MAX_ENTITIES;
    Entity carried_object = MAX_ENTITIES;
    float route_t = 0.0f;
    float direction = 1.0f;
    float speed_wu = 24.0f;
    bool acknowledged = false;
    bool wage_record_spoofed = false;
};

struct LocalSuspicionComponent {
    bool active = false;
    LocalSuspicionCause cause = LocalSuspicionCause::NONE;
    LocalSuspicionResolution resolution = LocalSuspicionResolution::NONE;
    Entity workplace_entity = MAX_ENTITIES;
    Entity target_entity = MAX_ENTITIES;
    Entity path_entity = MAX_ENTITIES;
    Entity resolution_entity = MAX_ENTITIES;
    bool institutional_log_recovered = false;
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

struct PathStateComponent {
    PathState state = PathState::LIT;
};

struct DependencyDisruptionComponent {
    MicroZoneRole dependent_role = MicroZoneRole::WORKPLACE;
    MicroZoneRole provider_role = MicroZoneRole::SUPPLY;
    bool disrupted = false;
    bool recovered = false;
};

struct RouteSignpostComponent {
    Entity path_entity = MAX_ENTITIES;
    Entity endpoint_entity = MAX_ENTITIES;
    MicroZoneRole target_role = MicroZoneRole::HOUSING;
    bool spoofed = false;
    bool signal_recovered = false;
};
