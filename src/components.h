#pragma once
#include <string>
#include <array>
#include <vector>
#include <cstdint>
#include "ecs.h"

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
    float anim_timer = 0.0f; // countdown to next frame
    int   anim_frame = 0;    // current frame index (0-based)
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

// Ground-level area where pedestrians wait before ascending to a maglift station.
// Also carries RoadComponent(PEDESTRIAN_PATH) so citizens can navigate to it.
struct WaitingAreaComponent {
    int station_index = 0;
};

// Staircase connecting the ground waiting area to the elevated maglift platform.
// Also carries RoadComponent(PEDESTRIAN_PATH) so it is walkable.
struct StaircaseComponent {
    int station_index = 0;
};

// Marks an entity as a maglift station stop.
struct StationComponent {
    int station_index = 0;
    float stop_duration = 4.0f; // seconds transit vehicle dwells here
};

// Drives a maglift transit vehicle along the track (pingpong between terminal stations).
// TransitSystem updates TransformComponent.y directly — no MovementComponent needed.
struct TransitVehicleComponent {
    enum State { MOVING, STOPPED } state = MOVING;
    int current_station = 0;  // station index just departed (or currently at)
    int next_station    = 1;  // station index heading toward
    int direction       = 1;  // +1 = increasing index, -1 = decreasing
    float stop_timer    = 0.0f;
    float speed         = 140.0f; // WU/s along track
    float track_x       = 0.0f;  // fixed X of the track; snapped to each frame
};

struct TemperatureGridComponent {
    int cols = 0;
    int rows = 0;
    std::vector<float> grid;     // current temperature values
    std::vector<float> back;     // back-buffer for diffusion
};

struct AtmosphereGridComponent {
    int cols = 0;
    int rows = 0;
    std::vector<float> grid;     // air quality values (0.0 - 100.0)
    std::vector<float> back;
};

struct InteriorComponent {
    Entity building_entity = MAX_ENTITIES;
};

struct BuildingAtmosphereComponent {
    float temperature = 20.0f;
    float air_quality = 100.0f;
};

enum class TimeOfDay { DAWN, DAY, DUSK, NIGHT };
enum class WeatherState { CLEAR, OVERCAST, ACID_RAIN, SMOG };

// Singleton component on the worldConfig entity.
// time_scale: game-minutes per real second (2.0 → full day in ~12 real minutes).
struct TimeOfDayComponent {
    float game_hour       = 8.0f;            // 0.0–24.0
    float time_scale      = 2.0f;            // game-minutes per real second
    TimeOfDay phase       = TimeOfDay::DAY;
    WeatherState weather  = WeatherState::CLEAR;
    float ambient_target  = 20.0f;           // target Celsius for equilibrium
    float spawn_multiplier = 1.0f;
    uint8_t tint_r = 40, tint_g = 42, tint_b = 45; // background clear color
    bool is_flooded       = false;           // placeholder hazard flag
};

// ── L0 Structural Decay ──────────────────────────────────────────────────────

enum class MaterialType { STEEL, REINFORCED_CONCRETE, COMPOSITE, SCRAP };

struct StructuralComponent {
    float integrity = 100.0f;
    MaterialType material_type = MaterialType::REINFORCED_CONCRETE;
    uint64_t last_maintenance_tick = 0;
    bool is_exposed = true;
};

struct CollapsedComponent {
    bool is_collapsed = true;
};

// ── L2 Cognitive ─────────────────────────────────────────────────────────────

enum class SocialRank { VAGRANT = 0, SLUM_DWELLER = 1, WORKING_POOR = 2, MIDDLE_CLASS = 3, CORPORATE_ELITE = 4 };

// Social tier derived from economic state; drives dominance PAD axis and movement speed.
// prestige: smooth [-1, +1] signal within the tier, shaped by proximity to higher/lower ranks.
struct SocialRankComponent {
    SocialRank rank     = SocialRank::WORKING_POOR;
    float      prestige = 0.0f;
};

enum class MemoryEventType { SAW_FOOD, ATE_FOOD, DRANK_WATER, SAW_VIOLENCE, HEARD_RUMOR, BECAME_ILL };

struct MemoryRecord {
    MemoryEventType event     = MemoryEventType::SAW_FOOD;
    float           timestamp = 0.0f;   // game_hour when recorded
    float           intensity = 0.0f;   // 0.0–1.0
    Entity          source    = MAX_ENTITIES;
};

// PAD emotional model + fixed-capacity episodic memory ring.
// pleasure/arousal/dominance: -1.0 (negative pole) to +1.0 (positive pole).
struct CognitiveComponent {
    float pleasure  = 0.0f;
    float arousal   = 0.0f;
    float dominance = 0.0f;

    static constexpr int MEM_CAP = 16;
    std::array<MemoryRecord, MEM_CAP> memory{};
    int mem_head = 0;
    int mem_size = 0;

    void record(MemoryRecord r) {
        memory[mem_head] = r;
        mem_head = (mem_head + 1) % MEM_CAP;
        if (mem_size < MEM_CAP) ++mem_size;
    }
};

// ── L2 Goal-Desire-Intention ──────────────────────────────────────────────────

// What a citizen wants to do, ranked by urgency.
// NONE means no pressing need — fall back to wandering.
enum class DesireType { NONE, SATISFY_HUNGER, SATISFY_THIRST, FIND_TRANSIT, BUY_FOOD, BUY_WATER };

// Tracks why a citizen's current GoalComponent target was chosen.
// commitment counts down each L2 tick; at 0 the GDISystem re-evaluates.
struct IntentionComponent {
    DesireType active_desire = DesireType::NONE;
    float      commitment    = 0.0f;  // L2-ticks remaining before reconsidering
};

// ── L2 Schedule ──────────────────────────────────────────────────────────────

enum class ScheduleState { SLEEPING, WORKING, LEISURE, TRANSIT };

// Daily rhythm component — drives sleep/work/leisure/transit cycles.
// work_start/work_end and sleep_start/sleep_end are game-hour floats (0–24).
// Sleep window may cross midnight (e.g., sleep_start=22, sleep_end=6).
struct ScheduleComponent {
    ScheduleState state      = ScheduleState::LEISURE;
    float work_start         = 8.0f;
    float work_end           = 18.0f;
    float sleep_start        = 22.0f;
    float sleep_end          = 6.0f;
};

// ── L2 Relationship Graph ─────────────────────────────────────────────────────

struct RelationshipEntry {
    Entity other    = MAX_ENTITIES; // MAX_ENTITIES = vacant slot
    float  affinity = 0.0f;         // [-1.0 hostile → +1.0 bonded]
};

// Fixed-capacity relationship table carried by each citizen.
// Up to 8 concurrent non-stranger relationships. Tier is derived from affinity at
// query time. Slots are compacted in-place when entries decay to near-zero.
struct RelationshipComponent {
    static constexpr int CAP = 8;
    std::array<RelationshipEntry, CAP> entries{};
    int count = 0; // number of active slots

    // Returns index of existing slot for `other`, or -1.
    int find(Entity other) const {
        for (int i = 0; i < count; ++i)
            if (entries[i].other == other) return i;
        return -1;
    }

    // Returns affinity for `other`, or 0.0 if not tracked.
    float get_affinity(Entity other) const {
        int i = find(other);
        return (i >= 0) ? entries[i].affinity : 0.0f;
    }
};

struct ConversationComponent {
    float cooldown = 0.0f;
    Entity last_partner = MAX_ENTITIES;
};

struct SpeechBubbleComponent {
    std::string text;
    float ttl = 0.0f;
};

// ── L1 Biology ────────────────────────────────────────────────────────────────

enum ItemFlag : uint32_t {
    ITEM_FLAG_LEGAL            = 1u << 0,
    ITEM_FLAG_ILLEGAL          = 1u << 1,
    ITEM_FLAG_UNIQUE           = 1u << 2,
    ITEM_FLAG_HIGH_VALUE       = 1u << 3,
    ITEM_FLAG_FACTION_RELEVANT = 1u << 4,
    ITEM_FLAG_QUEST            = 1u << 5
};

struct ItemProvenance {
    bool tracked = false;
    bool stolen = false;
    Entity owner = MAX_ENTITIES;
    Entity source = MAX_ENTITIES;
};

// Collectible item that restores a biological stat when consumed.
struct ItemComponent {
    enum Type {
        FOOD,
        WATER,
        MEDICAL,
        SURFACE_SCAN_TOOL,
        BIOLOGY_AUDIT_TOOL,
        COGNITIVE_PROFILE_TOOL,
        FINANCIAL_FORENSICS_TOOL,
        STRUCTURAL_ANALYSIS_TOOL
    } type = FOOD;
    float restore_value = 40.0f;
    uint32_t flags = ITEM_FLAG_LEGAL;
    ItemProvenance provenance;
};

// Player-carried survival supplies. Full inventory management remains future work;
// this is the L1 survival counter UI for common consumables.
struct SurvivalInventoryComponent {
    int food_count    = 0;
    int water_count   = 0;
    int medical_count = 0;
};

struct CarriedItem {
    bool occupied = false;
    ItemComponent::Type type = ItemComponent::FOOD;
    float restore_value = 40.0f;
    uint32_t flags = ITEM_FLAG_LEGAL;
    Entity source = MAX_ENTITIES;
    ItemProvenance provenance;
};

struct DiscreteInventoryComponent {
    static constexpr size_t CAPACITY = 12;
    std::array<CarriedItem, CAPACITY> slots{};
    size_t selected = 0;
};

// Emitted by BiologySystem when a citizen dies; consumed by MemoryFormationSystem.
struct ViolenceEvent {
    float x, y;
};

// Granular health tracking for critical systems.
struct OrganHealth {
    float heart   = 100.0f;
    float lungs   = 100.0f;
    float brain   = 100.0f;
    float liver   = 100.0f;
    float kidneys = 100.0f;
};

// Real-time vital signs derived from organ and metabolic state.
struct VitalSigns {
    float blood_pressure[2] = { 120.0f, 80.0f }; // Systolic, Diastolic
    float heart_rate       = 80.0f;              // bpm
    float oxygen_sat       = 1.0f;               // 0.0 - 1.0 (100%)
};

// Active pathogen exposure/infection. Carried only by exposed or infected entities.
struct PathogenComponent {
    int   strain_id        = 1;
    float infection_load   = 35.0f; // 0 = cleared; >20 can progress after incubation
    float infectiousness   = 0.35f; // exposure strength per L1 update
    float severity         = 0.0f;  // 0.0 - 1.0 symptomatic intensity
    float incubation_timer = 30.0f; // game-minutes until symptoms can worsen
    float immune_response  = 0.08f; // load cleared per scaled second
    int   alert_tier       = 0;     // highest infection alert tier already emitted
    bool  trauma_recorded  = false; // one-shot L2 memory marker for severe symptoms
};

// Metabolic survival stats. Decays over time; health cascades when starving/dehydrated.
struct BiologyComponent {
    float health  = 80.0f;  // 0–100; reaches 0 → entity dies
    float hunger  = 80.0f;  // 0 = starving
    float thirst  = 80.0f;  // 0 = dehydrated
    float fatigue = 80.0f;  // 0 = exhausted

    OrganHealth organs;
    VitalSigns vitals;
};

// Active injury types. NONE is used as the empty-slot sentinel.
enum class InjuryType { NONE, LACERATION, INTERNAL_BLEEDING, BROKEN_LIMB, CONCUSSION, BURN };

// One injury slot: type + severity in [0,1]. severity == 0 means the slot is free.
struct InjurySlot {
    InjuryType type     = InjuryType::NONE;
    float      severity = 0.0f;
};

// Up to 4 concurrent injuries per entity. Slots are packed; cleared slots have type NONE.
struct InjuryComponent {
    static constexpr int MAX_SLOTS = 4;
    std::array<InjurySlot, MAX_SLOTS> slots{};
};

// ── L3 Economic ───────────────────────────────────────────────────────────────

// Wallet + employment record carried by each citizen.
// credits:     current balance [0, 9999]
// employer:    building entity that pays this citizen; MAX_ENTITIES = unemployed
// daily_wage:  credits earned per game-day; 0 if unemployed
// market_reputation: broad standing with local traders [-1 hostile, +1 trusted].
struct EconomicComponent {
    float  credits           = 100.0f;
    Entity employer          = MAX_ENTITIES;
    float  daily_wage        = 0.0f;
    float  market_reputation = 0.0f;
};

// Applied to building entities that act as employers.
// capacity:       max number of employees
// employee_count: current number of assigned employees
struct EmployerComponent {
    int capacity       = 4;
    int employee_count = 0;
};

// District-level market building. Citizens within 60 WU may spend credits on food/water/medical.
// Price per unit = base_price / clamp(stock / max_stock, 0.1f, 2.0f) — scarce raises, abundant lowers.
// greed_margin adjusts player-facing terms above scarcity (0.2 = 20% greed pressure).
// All three commodity categories live on each market; citizens buy whichever they need.
// restock_rate: units per real second (each stock field restocks independently at this rate).
struct MarketComponent {
    float food_stock    = 20.0f;
    float water_stock   = 20.0f;
    float medical_stock = 20.0f;
    float max_stock     = 20.0f;
    float base_price    = 5.0f;
    float restock_rate  = 0.5f;
    float greed_margin  = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────

// Bitmap-font glyph representation for any entity.
// chars: one or more ASCII characters rendered centered on the entity origin.
// r,g,b,a: glyph color (white = fully tintable via SDL_SetTextureColorMod).
// scale: extra multiplier applied on top of camera scale at render time.
// centered: if true, center the string on the entity's world position.
// tiled: if true, fill the entity's world-space rect with repeated chars[0]
//        (scale still controls tile size; centered is ignored in this mode).
struct GlyphComponent {
    std::string chars;
    uint8_t r = 255, g = 255, b = 255, a = 255;
    float   scale    = 1.0f;
    bool    centered = true;
    bool    tiled    = false;
};

// Power Grid Nodes (Generators, Substations, Consumers)
struct PowerNodeComponent {
    float supply  = 0.0f;
    float demand  = 0.0f;
    bool  powered = false;
};

// Power Grid Conduits (Transmission lines)
struct PowerConduitComponent {
    float capacity = 100.0f;
};
