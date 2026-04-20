#pragma once
#include <SDL.h>
#include <algorithm>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>
#include "ecs.h"
#include "components.h"
#include "inventory.h"
#include "spatial_grid.h"
#include <random>
#include <cmath>

// ── Social rank helpers (used by both AmbientSpawnSystem and SocialHierarchySystem) ──
inline SocialRank rankFromEco(const EconomicComponent& eco) {
    if (eco.employer == MAX_ENTITIES && eco.credits < 30.0f) return SocialRank::VAGRANT;
    if (eco.employer == MAX_ENTITIES || eco.daily_wage < 8.0f)  return SocialRank::SLUM_DWELLER;
    if (eco.daily_wage < 15.0f) return SocialRank::WORKING_POOR;
    if (eco.daily_wage < 25.0f) return SocialRank::MIDDLE_CLASS;
    return SocialRank::CORPORATE_ELITE;
}
inline float speedForRank(SocialRank r) {
    switch (r) {
        case SocialRank::VAGRANT:         return 18.0f;
        case SocialRank::SLUM_DWELLER:    return 24.0f;
        case SocialRank::WORKING_POOR:    return 30.0f;
        case SocialRank::MIDDLE_CLASS:    return 36.0f;
        case SocialRank::CORPORATE_ELITE: return 44.0f;
    }
    return 30.0f;
}
inline std::string glyphForRank(SocialRank r) {
    if (r == SocialRank::VAGRANT)      return ".";
    if (r >= SocialRank::MIDDLE_CLASS) return "I";
    return "i";
}

enum class SimulationAlertSeverity { INFO, WARNING, DANGER };
enum class SimulationAlertCategory { WEATHER, FLOOD, INFECTION, STRUCTURE };

struct SimulationAlert {
    SimulationAlertSeverity severity = SimulationAlertSeverity::INFO;
    SimulationAlertCategory category = SimulationAlertCategory::WEATHER;
    std::string message;
    float game_hour = 0.0f;
    float ttl = 6.0f;
};

class SimulationAlertStack {
    std::deque<SimulationAlert> alerts;
    size_t capacity = 8;

public:
    explicit SimulationAlertStack(size_t max_alerts = 8) : capacity(std::max<size_t>(1, max_alerts)) {}

    void push(SimulationAlertSeverity severity, SimulationAlertCategory category,
              std::string message, float game_hour, float ttl = 6.0f) {
        alerts.push_back({severity, category, std::move(message), game_hour, ttl});
        while (alerts.size() > capacity) alerts.pop_front();
    }

    void update(float dt) {
        for (auto& alert : alerts) {
            alert.ttl = std::max(0.0f, alert.ttl - dt);
        }
    }

    std::vector<SimulationAlert> visible(size_t max_visible = 4) const {
        std::vector<SimulationAlert> out;
        for (auto it = alerts.rbegin(); it != alerts.rend() && out.size() < max_visible; ++it) {
            if (it->ttl > 0.0f) out.push_back(*it);
        }
        return out;
    }

    size_t size() const { return alerts.size(); }
};

inline const char* weatherStateName(WeatherState weather) {
    switch (weather) {
        case WeatherState::CLEAR: return "CLEAR";
        case WeatherState::OVERCAST: return "OVERCAST";
        case WeatherState::ACID_RAIN: return "ACID_RAIN";
        case WeatherState::SMOG: return "SMOG";
    }
    return "UNKNOWN";
}

inline void emitWeatherAlertIfChanged(SimulationAlertStack& alerts,
                                      WeatherState previous,
                                      WeatherState current,
                                      float game_hour) {
    if (previous == current) return;
    SimulationAlertSeverity severity =
        (current == WeatherState::ACID_RAIN || current == WeatherState::SMOG)
            ? SimulationAlertSeverity::WARNING
            : SimulationAlertSeverity::INFO;
    alerts.push(severity, SimulationAlertCategory::WEATHER,
                std::string("WEATHER: ") + weatherStateName(current), game_hour);
}

inline void emitFloodAlert(SimulationAlertStack& alerts, bool is_flooded, float game_hour) {
    alerts.push(is_flooded ? SimulationAlertSeverity::WARNING : SimulationAlertSeverity::INFO,
                SimulationAlertCategory::FLOOD,
                is_flooded ? "FLOODING ACTIVE" : "FLOODING CLEARED",
                game_hour);
}

inline int infectionAlertTier(float severity) {
    if (severity >= 0.65f) return 2;
    if (severity >= 0.35f) return 1;
    return 0;
}

inline void emitInfectionAlertIfWorse(SimulationAlertStack& alerts,
                                      PathogenComponent& pathogen,
                                      Entity entity,
                                      float game_hour) {
    const int tier = infectionAlertTier(pathogen.severity);
    if (tier <= pathogen.alert_tier) return;
    pathogen.alert_tier = tier;

    char msg[96];
    std::snprintf(msg, sizeof(msg), "INFECTION WORSENED E%u %.0f%%",
                  static_cast<unsigned>(entity), pathogen.severity * 100.0f);
    alerts.push(tier >= 2 ? SimulationAlertSeverity::DANGER : SimulationAlertSeverity::WARNING,
                SimulationAlertCategory::INFECTION, msg, game_hour);
}

enum class FeedbackCue { PICKUP, CONSUME, SCAN, WARNING, DENIED, IMPACT, HAZARD_DAMAGE };

struct FeedbackEvent {
    FeedbackCue cue = FeedbackCue::SCAN;
    float ttl = 1.5f;
};

class FeedbackState {
    std::deque<FeedbackEvent> cues;
    size_t capacity = 12;
    float shake_remaining = 0.0f;
    float shake_duration = 0.0f;
    float shake_magnitude = 0.0f;
    float flash_remaining = 0.0f;
    float flash_duration = 0.0f;
    SDL_Color flash_color{255, 255, 255, 0};
    uint8_t flash_max_alpha = 0;

    void triggerShake(float duration, float magnitude) {
        if (magnitude >= shake_magnitude || shake_remaining <= 0.0f) {
            shake_duration = std::max(0.01f, duration);
            shake_remaining = shake_duration;
            shake_magnitude = magnitude;
        }
    }

    void triggerFlash(SDL_Color color, uint8_t alpha, float duration) {
        if (alpha >= flash_max_alpha || flash_remaining <= 0.0f) {
            flash_color = color;
            flash_duration = std::max(0.01f, duration);
            flash_remaining = flash_duration;
            flash_max_alpha = alpha;
        }
    }

public:
    explicit FeedbackState(size_t max_cues = 12) : capacity(std::max<size_t>(1, max_cues)) {}

    void push(FeedbackCue cue, float ttl = 1.5f) {
        cues.push_back({cue, ttl});
        while (cues.size() > capacity) cues.pop_front();

        switch (cue) {
            case FeedbackCue::PICKUP:
                triggerFlash({110, 235, 190, 255}, 45, 0.12f);
                break;
            case FeedbackCue::CONSUME:
                triggerFlash({120, 230, 120, 255}, 45, 0.12f);
                break;
            case FeedbackCue::SCAN:
                triggerFlash({100, 220, 255, 255}, 38, 0.10f);
                break;
            case FeedbackCue::WARNING:
                triggerFlash({245, 200, 70, 255}, 75, 0.18f);
                break;
            case FeedbackCue::DENIED:
                triggerFlash({230, 70, 70, 255}, 70, 0.14f);
                break;
            case FeedbackCue::IMPACT:
                triggerShake(0.12f, 4.0f);
                triggerFlash({255, 255, 255, 255}, 32, 0.08f);
                break;
            case FeedbackCue::HAZARD_DAMAGE:
                triggerShake(0.22f, 7.0f);
                triggerFlash({230, 55, 45, 255}, 105, 0.22f);
                break;
        }
    }

    void update(float dt) {
        for (auto& cue : cues) cue.ttl = std::max(0.0f, cue.ttl - dt);
        shake_remaining = std::max(0.0f, shake_remaining - dt);
        flash_remaining = std::max(0.0f, flash_remaining - dt);
        if (shake_remaining <= 0.0f) shake_magnitude = 0.0f;
        if (flash_remaining <= 0.0f) flash_max_alpha = 0;
    }

    std::vector<FeedbackEvent> recent(size_t max_visible = 8) const {
        std::vector<FeedbackEvent> out;
        for (auto it = cues.rbegin(); it != cues.rend() && out.size() < max_visible; ++it) {
            if (it->ttl > 0.0f) out.push_back(*it);
        }
        return out;
    }

    bool hasRecent(FeedbackCue cue) const {
        for (const auto& event : cues) {
            if (event.cue == cue && event.ttl > 0.0f) return true;
        }
        return false;
    }

    size_t size() const { return cues.size(); }
    bool shakeActive() const { return shake_remaining > 0.0f; }
    bool flashActive() const { return flash_remaining > 0.0f && flash_max_alpha > 0; }

    int shakeOffsetX() const {
        if (!shakeActive()) return 0;
        float progress = 1.0f - (shake_remaining / shake_duration);
        float envelope = shake_remaining / shake_duration;
        return static_cast<int>(std::sin(progress * 37.0f) * shake_magnitude * envelope);
    }

    int shakeOffsetY() const {
        if (!shakeActive()) return 0;
        float progress = 1.0f - (shake_remaining / shake_duration);
        float envelope = shake_remaining / shake_duration;
        return static_cast<int>(std::cos(progress * 31.0f) * shake_magnitude * envelope);
    }

    SDL_Color currentFlashColor() const {
        SDL_Color color = flash_color;
        if (!flashActive()) {
            color.a = 0;
            return color;
        }
        color.a = static_cast<uint8_t>(flash_max_alpha * (flash_remaining / flash_duration));
        return color;
    }
};

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
    void spawnVehicles(Registry& registry, float camX, float camY, float spawnRadius, float spawnMultiplier = 1.0f) {
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

            if (road.traffic_density > 0.0f && dist(rng) < 0.05f * road.traffic_density * spawnMultiplier) {
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
                if (vType == VehicleComponent::TRANSPORT) {
                    registry.assign<GlyphComponent>(vehicle, std::string("T"),
                        (uint8_t)200, (uint8_t)110, (uint8_t)200, (uint8_t)255, 0.5f, true, true);
                } else {
                    registry.assign<GlyphComponent>(vehicle, std::string("o"),
                        (uint8_t)255, (uint8_t)90, (uint8_t)90, (uint8_t)255, 0.5f, true, true);
                }
            }
        }
    }

    void spawnCitizens(Registry& registry, float camX, float camY, float spawnRadius, float spawnMultiplier = 1.0f) {
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

            if (road.type == RoadType::PEDESTRIAN_PATH && dist(rng) < 0.2f * spawnMultiplier) {
                Entity citizen = registry.create();

                std::uniform_real_distribution<float> xDist(transform.x - transform.width / 2.0f, transform.x + transform.width / 2.0f);
                std::uniform_real_distribution<float> yDist(transform.y - transform.height / 2.0f, transform.y + transform.height / 2.0f);

                registry.assign<CitizenComponent>(citizen);
                registry.assign<TransformComponent>(citizen, xDist(rng), yDist(rng), 8.0f, 8.0f);
                registry.assign<MovementComponent>(citizen, 0.0f, 0.0f, MovementComponent::NORMAL);
                registry.assign<GlyphComponent>(citizen, std::string("i"),
                    (uint8_t)150, (uint8_t)200, (uint8_t)255, (uint8_t)255, 0.5f, true);
                registry.assign<BiologyComponent>(citizen);
                registry.assign<CognitiveComponent>(citizen);
                registry.assign<RelationshipComponent>(citizen);
                if (dist(rng) < 0.02f) {
                    registry.assign<PathogenComponent>(citizen);
                }

                // Assign schedule — ~15% night workers, ~15% early birds, rest default
                {
                    ScheduleComponent sc;
                    float roll = dist(rng);
                    if (roll < 0.15f) {         // night worker
                        sc.work_start = 20.0f; sc.work_end = 5.0f;
                        sc.sleep_start = 9.0f; sc.sleep_end = 17.0f;
                    } else if (roll < 0.30f) {  // early bird
                        sc.work_start = 6.0f;  sc.work_end = 14.0f;
                    }
                    registry.assign<ScheduleComponent>(citizen, sc);
                }

                // Economic assignment: find nearest employer within 400 WU
                {
                    std::uniform_real_distribution<float> wageDist(5.0f, 25.0f);
                    std::uniform_real_distribution<float> creditsDist(50.0f, 150.0f);
                    EconomicComponent eco;
                    eco.credits = creditsDist(rng);

                    Entity nearestEmployer = MAX_ENTITIES;
                    float  bestEmpDist     = 400.0f * 400.0f;
                    auto employers = registry.view<EmployerComponent, TransformComponent>();
                    for (Entity emp : employers) {
                        auto& ec = registry.get<EmployerComponent>(emp);
                        if (ec.employee_count >= ec.capacity) continue;
                        auto& et = registry.get<TransformComponent>(emp);
                        float edx = et.x - transform.x, edy = et.y - transform.y;
                        float ed2 = edx*edx + edy*edy;
                        if (ed2 < bestEmpDist) { bestEmpDist = ed2; nearestEmployer = emp; }
                    }
                    if (nearestEmployer != MAX_ENTITIES) {
                        eco.employer   = nearestEmployer;
                        eco.daily_wage = wageDist(rng);
                        registry.get<EmployerComponent>(nearestEmployer).employee_count++;
                    }
                    registry.assign<EconomicComponent>(citizen, eco);

                    // Social rank from economic state
                    SocialRank sr = rankFromEco(eco);
                    registry.assign<SocialRankComponent>(citizen, sr, 0.0f);
                    registry.get<CitizenComponent>(citizen).speed = speedForRank(sr);
                    registry.get<GlyphComponent>(citizen).chars   = glyphForRank(sr);
                }

                // Goal Assignment: 50% chance to target a transit station
                if (dist(rng) < 0.5f) {
                    auto stations = registry.view<TransformComponent, StationComponent>();
                    Entity closestStation = MAX_ENTITIES;
                    float minDist = 1e9f;
                    
                    for (Entity sEnt : stations) {
                        auto& st = registry.get<TransformComponent>(sEnt);
                        float dx = st.x - transform.x;
                        float dy = st.y - transform.y;
                        float d = dx*dx + dy*dy;
                        if (d < minDist) {
                            minDist = d;
                            closestStation = sEnt;
                        }
                    }
                    
                    if (closestStation != MAX_ENTITIES) {
                        registry.assign<GoalComponent>(citizen, closestStation);
                    }
                }
            }
        }
    }

    void spawnItems(Registry& registry, float camX, float camY, float spawnRadius) {
        auto existingItems = registry.view<ItemComponent>();
        if (static_cast<int>(existingItems.size()) >= 60) return;

        initialize(registry);
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (Entity e : cachedRoads) {
            auto& road = registry.get<RoadComponent>(e);
            if (road.type != RoadType::PEDESTRIAN_PATH) continue;

            auto& transform = registry.get<TransformComponent>(e);
            float dx = transform.x - camX, dy = transform.y - camY;
            if (dx*dx + dy*dy > spawnRadius*spawnRadius) continue;

            if (dist(rng) >= 0.01f) continue;

            float hw = transform.width  * 0.3f;
            float hh = transform.height * 0.3f;
            std::uniform_real_distribution<float> ox(-hw, hw);
            std::uniform_real_distribution<float> oy(-hh, hh);

            bool isFood = dist(rng) < 0.5f;
            Entity item = registry.create();
            registry.assign<TransformComponent>(item, transform.x + ox(rng), transform.y + oy(rng), 8.0f, 8.0f);
            if (isFood) {
                registry.assign<ItemComponent>(item, ItemComponent::FOOD, 40.0f);
                registry.assign<GlyphComponent>(item, std::string("f"),
                    (uint8_t)220, (uint8_t)160, (uint8_t)50, (uint8_t)255, 0.5f, true);
            } else {
                registry.assign<ItemComponent>(item, ItemComponent::WATER, 40.0f);
                registry.assign<GlyphComponent>(item, std::string("w"),
                    (uint8_t)60, (uint8_t)180, (uint8_t)255, (uint8_t)255, 0.5f, true);
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

// ── L2 Schedule System ───────────────────────────────────────────────────────
// Runs at L2 (2 Hz). Transitions citizens between SLEEPING/WORKING/LEISURE/TRANSIT
// based on game_hour and their individual schedule windows.
// Updates GlyphComponent color on state transitions.
class ScheduleSystem {
    static bool inWindow(float hour, float start, float end) {
        // Handles midnight-crossing windows (e.g., sleep_start=22, sleep_end=6)
        return (start <= end) ? (hour >= start && hour < end)
                              : (hour >= start || hour < end);
    }
public:
    void update(Registry& registry, float game_hour) {
        auto view = registry.view<ScheduleComponent>();
        for (Entity e : view) {
            auto& sc = registry.get<ScheduleComponent>(e);
            ScheduleState prev = sc.state;

            if (inWindow(game_hour, sc.sleep_start, sc.sleep_end)) {
                sc.state = ScheduleState::SLEEPING;
            } else {
                // Within ±1.5 game-hours of work start or end = commuting
                float near_s = std::fmod(std::abs(game_hour - sc.work_start), 24.0f);
                float near_e = std::fmod(std::abs(game_hour - sc.work_end),   24.0f);
                // wrap the complement (e.g., abs(6-22)=16 vs 24-16=8)
                near_s = std::min(near_s, 24.0f - near_s);
                near_e = std::min(near_e, 24.0f - near_e);
                bool commuting = near_s < 1.5f || near_e < 1.5f;

                if (commuting)
                    sc.state = ScheduleState::TRANSIT;
                else if (inWindow(game_hour, sc.work_start, sc.work_end))
                    sc.state = ScheduleState::WORKING;
                else
                    sc.state = ScheduleState::LEISURE;
            }

            // Update glyph color on state change
            if (sc.state != prev && registry.has<GlyphComponent>(e)) {
                auto& g = registry.get<GlyphComponent>(e);
                switch (sc.state) {
                    case ScheduleState::SLEEPING: g.r=80;  g.g=90;  g.b=120; break;
                    case ScheduleState::WORKING:  g.r=220; g.g=220; g.b=170; break;
                    case ScheduleState::TRANSIT:  g.r=255; g.g=170; g.b=60;  break;
                    case ScheduleState::LEISURE:  g.r=150; g.g=200; g.b=255; break;
                }
            }
        }
    }
};

// ── L2 Goal-Desire-Intention System ──────────────────────────────────────────
// Runs at L2 (2 Hz). For each citizen with Biology + Cognitive components,
// scores competing desires and commits to the highest-urgency one.
// Writes (or clears) GoalComponent based on the active intention.
// Owns all citizen goal assignment; ConsumableSystem only handles consumption.
class GDISystem {
public:
    void update(Registry& registry, float dt, float time_scale) {
        auto citizens = registry.view<CitizenComponent, TransformComponent,
                                      BiologyComponent, CognitiveComponent>();
        for (Entity e : citizens) {
            auto& bio = registry.get<BiologyComponent>(e);
            auto& cog = registry.get<CognitiveComponent>(e);
            auto& t   = registry.get<TransformComponent>(e);

            // ── Schedule gate ─────────────────────────────────────────────
            float transit_boost = 0.0f;
            bool  no_transit    = false;
            if (registry.has<ScheduleComponent>(e)) {
                auto& sc = registry.get<ScheduleComponent>(e);
                if (sc.state == ScheduleState::SLEEPING) {
                    if (!registry.has<IntentionComponent>(e))
                        registry.assign<IntentionComponent>(e);
                    auto& intent = registry.get<IntentionComponent>(e);
                    intent.active_desire = DesireType::NONE;
                    intent.commitment    = 1.0f;
                    continue;
                }
                if (sc.state == ScheduleState::TRANSIT) transit_boost = 0.5f;
                if (sc.state == ScheduleState::WORKING)  no_transit    = true;
            }

            // ── Step 1: score desires ─────────────────────────────────────
            float hunger_u  = (100.0f - bio.hunger) / 100.0f
                              + std::max(0.0f, cog.arousal * 0.2f);
            float thirst_u  = (100.0f - bio.thirst) / 100.0f
                              + std::max(0.0f, cog.arousal * 0.3f);
            float transit_u = no_transit ? 0.0f
                              : 0.3f + std::max(0.0f, cog.dominance * 0.1f) + transit_boost;

            // Market purchases are handled opportunistically by MarketSystem (proximity-based,
            // no goal-setting needed). GDI only scores biology/transit desires.
            DesireType winner = DesireType::NONE;
            float      best   = 0.15f; // minimum urgency to act
            if (hunger_u  > best) { best = hunger_u;  winner = DesireType::SATISFY_HUNGER; }
            if (thirst_u  > best) { best = thirst_u;  winner = DesireType::SATISFY_THIRST; }
            if (transit_u > best) { best = transit_u; winner = DesireType::FIND_TRANSIT;   }

            // ── Step 2: check/create IntentionComponent ───────────────────
            if (!registry.has<IntentionComponent>(e))
                registry.assign<IntentionComponent>(e);
            auto& intent = registry.get<IntentionComponent>(e);

            intent.commitment -= 1.0f;

            bool desireChanged = (winner != intent.active_desire);
            bool goalDead = registry.has<GoalComponent>(e) &&
                            (!registry.alive(registry.get<GoalComponent>(e).target_entity) ||
                              registry.get<GoalComponent>(e).target_entity == MAX_ENTITIES);
            bool replan = (intent.commitment <= 0.0f) || desireChanged || goalDead;
            if (!replan) continue;

            // ── Step 3: translate desire → goal ───────────────────────────
            intent.active_desire = winner;

            if (winner == DesireType::NONE) {
                if (registry.has<GoalComponent>(e))
                    registry.get<GoalComponent>(e).target_entity = MAX_ENTITIES;
                intent.commitment = 6.0f;
                continue;
            }

            Entity target = MAX_ENTITIES;
            float  bestDist = 1e9f;

            if (winner == DesireType::SATISFY_HUNGER || winner == DesireType::SATISFY_THIRST) {
                ItemComponent::Type wanted = (winner == DesireType::SATISFY_HUNGER)
                                             ? ItemComponent::FOOD : ItemComponent::WATER;
                auto items = registry.view<ItemComponent, TransformComponent>();
                for (Entity item : items) {
                    if (registry.get<ItemComponent>(item).type != wanted) continue;
                    auto& it = registry.get<TransformComponent>(item);
                    float dx = it.x - t.x, dy = it.y - t.y;
                    float d = dx*dx + dy*dy;
                    if (d < bestDist) { bestDist = d; target = item; }
                }
            } else { // FIND_TRANSIT
                auto stations = registry.view<StationComponent, TransformComponent>();
                for (Entity st : stations) {
                    auto& st_t = registry.get<TransformComponent>(st);
                    float dx = st_t.x - t.x, dy = st_t.y - t.y;
                    float d = dx*dx + dy*dy;
                    if (d < bestDist) { bestDist = d; target = st; }
                }
            }

            if (target == MAX_ENTITIES) {
                intent.commitment = 3.0f; // retry sooner if nothing found
                continue;
            }

            if (registry.has<GoalComponent>(e))
                registry.get<GoalComponent>(e).target_entity = target;
            else
                registry.assign<GoalComponent>(e, target);

            intent.commitment = 10.0f; // ~5 s at 2 Hz
        }
    }
};
// ─────────────────────────────────────────────────────────────────────────────

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

            // Sleeping citizens stand still
            if (registry.has<ScheduleComponent>(e) &&
                registry.get<ScheduleComponent>(e).state == ScheduleState::SLEEPING) {
                m.vx = 0.0f; m.vy = 0.0f;
                continue;
            }

            // 1. Goal-Based Movement & Clustering
            bool skip_wander = false;
            if (registry.has<GoalComponent>(e)) {
                auto& goal = registry.get<GoalComponent>(e);
                if (registry.has<TransformComponent>(goal.target_entity)) {
                    auto& gt = registry.get<TransformComponent>(goal.target_entity);
                    float dx = gt.x - t.x;
                    float dy = gt.y - t.y;
                    float distSq = dx*dx + dy*dy;
                    float waitRadius = 20.0f;

                    if (distSq < waitRadius * waitRadius) {
                        // Reached goal: Cluster and idle
                        m.vx = 0.0f;
                        m.vy = 0.0f;
                    } else {
                        // Steer toward goal
                        float d = std::sqrt(distSq);
                        float activeSpeed = c.speed;

                        // 2. Transit Arrival Hurrying
                        m.speed_state = MovementComponent::NORMAL;
                        if (registry.has<StationComponent>(goal.target_entity)) {
                            auto& targetStation = registry.get<StationComponent>(goal.target_entity);
                            auto transitVehicles = registry.view<TransitVehicleComponent, TransformComponent>();
                            for (Entity vEnt : transitVehicles) {
                                auto& tv = registry.get<TransitVehicleComponent>(vEnt);
                                auto& vt = registry.get<TransformComponent>(vEnt);

                                // Hurry if a transit vehicle is at or approaching our target station
                                if (tv.next_station == targetStation.station_index ||
                                   (tv.state == TransitVehicleComponent::STOPPED && tv.current_station == targetStation.station_index)) {
                                    float vDistSq = std::pow(vt.x - gt.x, 2) + std::pow(vt.y - gt.y, 2);
                                    if (vDistSq < 40000.0f) { // Hurry if train is within ~200 WU of station
                                        m.speed_state = MovementComponent::HURRY;
                                        activeSpeed *= 1.8f;
                                        break;
                                    }
                                }
                            }
                        }

                        m.vx = (dx / d) * activeSpeed;
                        m.vy = (dy / d) * activeSpeed;

                        if (std::abs(m.vx) > std::abs(m.vy)) {
                            c.facing = (m.vx > 0) ? Facing::RIGHT : Facing::LEFT;
                        } else {
                            c.facing = (m.vy > 0) ? Facing::DOWN : Facing::UP;
                        }
                    }
                    skip_wander = true;
                }
            }

            // 3. Wandering Logic (for those without active goals)
            if (!skip_wander) {
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
            }

            // 4. Social Steering — friend attraction, enemy repulsion
            if (registry.has<RelationshipComponent>(e)) {
                auto& rel = registry.get<RelationshipComponent>(e);
                float sx = 0.0f, sy = 0.0f;
                for (int s = 0; s < rel.count; ++s) {
                    const auto& entry = rel.entries[s];
                    if (!registry.alive(entry.other)) continue;
                    if (!registry.has<TransformComponent>(entry.other)) continue;
                    auto& ot = registry.get<TransformComponent>(entry.other);
                    float dx = ot.x - t.x, dy = ot.y - t.y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < 1.0f) continue;
                    float d = std::sqrt(d2);
                    if (entry.affinity > 0.4f && d > 40.0f) {
                        // Attract toward friend when separated
                        sx += (dx / d) * 0.3f * entry.affinity;
                        sy += (dy / d) * 0.3f * entry.affinity;
                    } else if (entry.affinity < -0.4f) {
                        // Repel from enemy
                        sx -= (dx / d) * 0.5f * std::abs(entry.affinity);
                        sy -= (dy / d) * 0.5f * std::abs(entry.affinity);
                    }
                }
                if (sx != 0.0f || sy != 0.0f) {
                    float len = std::sqrt(sx*sx + sy*sy);
                    m.vx += (sx / len) * c.speed * 0.4f;
                    m.vy += (sy / len) * c.speed * 0.4f;
                }
            }
        }
    }
};

// ── L2 Relationship System ────────────────────────────────────────────────────
// Two-pass update at 2 Hz:
//   Pass 1 — proximity: reinforce relationships for citizens within 30 WU.
//   Pass 2 — decay: all affinities drift toward 0; forget near-zero slots.
// N^2 pair loop over ~100 citizens at 2 Hz is comfortably within budget.
class RelationshipSystem {
    static void reinforce(RelationshipComponent& rc, Entity other, float delta) {
        int idx = rc.find(other);
        if (idx >= 0) {
            rc.entries[idx].affinity = std::min(1.0f, rc.entries[idx].affinity + delta);
            return;
        }
        if (rc.count < RelationshipComponent::CAP) {
            rc.entries[rc.count++] = { other, delta };
            return;
        }
        // Table full — evict the weakest positive slot (never evict enemy slots).
        int weakest = -1;
        float wv = delta; // only evict if it's weaker than the newcomer
        for (int i = 0; i < rc.count; ++i) {
            if (rc.entries[i].affinity > 0.0f && rc.entries[i].affinity < wv) {
                wv = rc.entries[i].affinity;
                weakest = i;
            }
        }
        if (weakest >= 0)
            rc.entries[weakest] = { other, delta };
    }

public:
    void update(Registry& registry) {
        constexpr float PROX_SQ   = 30.0f * 30.0f;
        constexpr float REINFORCE = 0.02f;
        constexpr float DECAY     = 0.003f;
        constexpr float FORGET    = 0.01f;

        auto view = registry.view<CitizenComponent, TransformComponent, RelationshipComponent>();
        std::vector<Entity> list;
        for (Entity e : view) list.push_back(e);
        const int n = static_cast<int>(list.size());

        // Pass 1: proximity formation / reinforcement (each pair processed once)
        for (int i = 0; i < n; ++i) {
            auto& ta = registry.get<TransformComponent>(list[i]);
            auto& ra = registry.get<RelationshipComponent>(list[i]);
            for (int j = i + 1; j < n; ++j) {
                auto& tb = registry.get<TransformComponent>(list[j]);
                float dx = ta.x - tb.x, dy = ta.y - tb.y;
                if (dx*dx + dy*dy > PROX_SQ) continue;
                reinforce(ra, list[j], REINFORCE);
                reinforce(registry.get<RelationshipComponent>(list[j]), list[i], REINFORCE);
            }
        }

        // Pass 2: passive decay toward zero; compact-out forgotten slots
        for (int i = 0; i < n; ++i) {
            auto& rc = registry.get<RelationshipComponent>(list[i]);
            int dst = 0;
            for (int s = 0; s < rc.count; ++s) {
                auto& entry = rc.entries[s];
                entry.affinity += (entry.affinity > 0.0f) ? -DECAY : DECAY;
                entry.affinity = std::max(-1.0f, std::min(1.0f, entry.affinity));
                if (std::abs(entry.affinity) < FORGET) continue; // forget
                rc.entries[dst++] = entry;
            }
            rc.count = dst;
        }
    }
};

// ── L2 Social Hierarchy System ────────────────────────────────────────────────
// Runs at L2 (2 Hz). Two passes:
//   Pass 1 — derives SocialRank from EconomicComponent, nudges PAD dominance axis.
//   Pass 2 — prestige decay + N² proximity exchange (higher rank dominates local space).
class SocialHierarchySystem {
public:
    void update(Registry& registry) {
        // Pass 1: rank refresh + dominance nudge
        auto pass1 = registry.view<SocialRankComponent, EconomicComponent,
                                   CognitiveComponent, CitizenComponent>();
        for (Entity e : pass1) {
            auto& sr  = registry.get<SocialRankComponent>(e);
            auto& eco = registry.get<EconomicComponent>(e);
            auto& cog = registry.get<CognitiveComponent>(e);
            auto& cit = registry.get<CitizenComponent>(e);

            SocialRank newRank = rankFromEco(eco);
            if (newRank != sr.rank) {
                sr.rank    = newRank;
                cit.speed  = speedForRank(newRank);
            }

            float dominance_target = (static_cast<int>(sr.rank) / 4.0f) * 2.0f - 1.0f;
            cog.dominance += (dominance_target - cog.dominance) * 0.05f;
        }

        // Pass 2: prestige decay + proximity exchange
        constexpr float PROX_SQ      = 50.0f * 50.0f;
        constexpr float GAIN_HIGH    = 0.01f;
        constexpr float LOSS_LOW     = 0.005f;
        constexpr float DECAY_FACTOR = 0.98f;

        std::vector<Entity> list;
        {
            auto pass2view = registry.view<SocialRankComponent, TransformComponent>();
            for (Entity e : pass2view) list.push_back(e);
        }
        const int n = static_cast<int>(list.size());

        // Decay prestige toward 0
        for (int i = 0; i < n; ++i) {
            registry.get<SocialRankComponent>(list[i]).prestige *= DECAY_FACTOR;
        }

        // Proximity prestige exchange
        for (int i = 0; i < n; ++i) {
            auto& ta = registry.get<TransformComponent>(list[i]);
            auto& sa = registry.get<SocialRankComponent>(list[i]);
            for (int j = i + 1; j < n; ++j) {
                auto& tb = registry.get<TransformComponent>(list[j]);
                float dx = ta.x - tb.x, dy = ta.y - tb.y;
                if (dx*dx + dy*dy > PROX_SQ) continue;
                auto& sb = registry.get<SocialRankComponent>(list[j]);
                int delta = static_cast<int>(sa.rank) - static_cast<int>(sb.rank);
                if (delta > 0) {
                    sa.prestige = std::min(1.0f, sa.prestige + GAIN_HIGH);
                    sb.prestige = std::max(-1.0f, sb.prestige - LOSS_LOW);
                } else if (delta < 0) {
                    sb.prestige = std::min(1.0f, sb.prestige + GAIN_HIGH);
                    sa.prestige = std::max(-1.0f, sa.prestige - LOSS_LOW);
                }
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
                    // Exit vehicle — transit vehicles only allow exit when STOPPED at a station
                    auto& pass = registry.get<PassengerComponent>(e);
                    bool canExit = true;
                    if (registry.has<TransitVehicleComponent>(pass.vehicle)) {
                        auto& tv = registry.get<TransitVehicleComponent>(pass.vehicle);
                        canExit = (tv.state == TransitVehicleComponent::STOPPED);
                    }
                    if (canExit) {
                        if (registry.has<VehicleComponent>(pass.vehicle)) {
                            auto& v = registry.get<VehicleComponent>(pass.vehicle);
                            if (v.driver == e) v.driver = MAX_ENTITIES;
                        }
                        pass.vehicle = MAX_ENTITIES;
                        t.y += 20.0f;
                    }
                } else {
                    // Enter vehicle — transit vehicles only boardable when STOPPED
                    auto vehicles = registry.view<VehicleComponent, TransformComponent>();
                    float closestDist = 40.0f;
                    Entity closestVehicle = MAX_ENTITIES;

                    for (Entity vEnt : vehicles) {
                        auto& vt = registry.get<TransformComponent>(vEnt);
                        auto& veh = registry.get<VehicleComponent>(vEnt);

                        // Skip transit vehicles that are in motion
                        if (registry.has<TransitVehicleComponent>(vEnt)) {
                            auto& tv = registry.get<TransitVehicleComponent>(vEnt);
                            if (tv.state != TransitVehicleComponent::STOPPED) continue;
                        }

                        float dist = std::sqrt(std::pow(t.x - vt.x, 2) + std::pow(t.y - vt.y, 2));
                        if (dist < closestDist && veh.driver == MAX_ENTITIES) {
                            closestDist = dist;
                            closestVehicle = vEnt;
                        }
                    }

                    if (closestVehicle != MAX_ENTITIES) {
                        // Transit vehicles: player is passenger, not driver
                        bool isTransit = registry.has<TransitVehicleComponent>(closestVehicle);
                        if (!registry.has<PassengerComponent>(e)) {
                            registry.assign<PassengerComponent>(e, closestVehicle);
                        } else {
                            registry.get<PassengerComponent>(e).vehicle = closestVehicle;
                        }
                        if (!isTransit) {
                            registry.get<VehicleComponent>(closestVehicle).driver = e;
                        }
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

                // Transit vehicles are automated — player is a passive rider, no input applied.
                bool isTransit = registry.has<TransitVehicleComponent>(vEnt);
                if (!isTransit && registry.has<MovementComponent>(vEnt)) {
                    // Apply velocity to the driven vehicle, not the player
                    auto& vm = registry.get<MovementComponent>(vEnt);
                    vm.vx = 0.0f;
                    vm.vy = 0.0f;

                    if (keyboardState[SDL_SCANCODE_W]) vm.vy = -activeSpeed;
                    if (keyboardState[SDL_SCANCODE_S]) vm.vy = activeSpeed;
                    if (keyboardState[SDL_SCANCODE_A]) vm.vx = -activeSpeed;
                    if (keyboardState[SDL_SCANCODE_D]) vm.vx = activeSpeed;

                    if (vm.vx != 0.0f && vm.vy != 0.0f) {
                        float length = std::sqrt(vm.vx * vm.vx + vm.vy * vm.vy);
                        vm.vx = (vm.vx / length) * activeSpeed;
                        vm.vy = (vm.vy / length) * activeSpeed;
                    }
                }

                // Zero out player's personal velocity
                m.vx = 0.0f;
                m.vy = 0.0f;
            } else {
                m.vx = 0.0f;
                m.vy = 0.0f;
                
                if (keyboardState[SDL_SCANCODE_W]) m.vy = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_S]) m.vy = activeSpeed;
                if (keyboardState[SDL_SCANCODE_A]) m.vx = -activeSpeed;
                if (keyboardState[SDL_SCANCODE_D]) m.vx = activeSpeed;
                
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

        auto todView = registry.view<TimeOfDayComponent>();
        bool is_flooded = !todView.empty() && registry.get<TimeOfDayComponent>(todView[0]).is_flooded;

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

            float flood_multiplier = 1.0f;
            if (is_flooded && registry.has<CitizenComponent>(e)) {
                flood_multiplier = 0.5f;
            }

            float nextX = t.x + m.vx * flood_multiplier * dt;
            float nextY = t.y + m.vy * flood_multiplier * dt;

            // Separation (Boids avoidance) to prevent perfect overlap
            if (registry.has<CitizenComponent>(e)) {
                float sepX = 0.0f;
                float sepY = 0.0f;
                float sepRadius = 12.0f;
                int count = 0;
                
                for (Entity neighbor : grid_.query(t.x, t.y, sepRadius*2, sepRadius*2)) {
                    if (neighbor == e) continue;
                    if (!registry.has<CitizenComponent>(neighbor)) continue;
                    auto& nt = registry.get<TransformComponent>(neighbor);
                    float dx = t.x - nt.x;
                    float dy = t.y - nt.y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 > 0 && d2 < sepRadius * sepRadius) {
                        float d = std::sqrt(d2);
                        sepX += (dx / d) * (sepRadius - d);
                        sepY += (dy / d) * (sepRadius - d);
                        count++;
                    }
                }
                
                if (count > 0) {
                    nextX += (sepX / count) * 0.5f;
                    nextY += (sepY / count) * 0.5f;
                }
            }

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

// Drives automated maglift transit vehicles along their track (gate at L0 — every frame).
// Updates TransformComponent.y directly; no MovementComponent interaction.
// Pingpong: vehicle reverses direction at each terminal station.
class TransitSystem {
private:
    struct StationInfo { int index; float y; float stop_duration; };
    std::vector<StationInfo> stations_;
    bool initialized_ = false;

    void buildStationList(Registry& registry) {
        stations_.clear();
        auto view = registry.view<TransformComponent, StationComponent>();
        for (Entity e : view) {
            auto& t = registry.get<TransformComponent>(e);
            auto& s = registry.get<StationComponent>(e);
            stations_.push_back({s.station_index, t.y, s.stop_duration});
        }
        std::sort(stations_.begin(), stations_.end(),
            [](const StationInfo& a, const StationInfo& b){ return a.index < b.index; });
        initialized_ = true;
    }

public:
    void update(Registry& registry, float dt) {
        if (!initialized_) buildStationList(registry);
        int n = static_cast<int>(stations_.size());
        if (n < 2) return;

        auto view = registry.view<TransformComponent, TransitVehicleComponent>();
        for (Entity e : view) {
            auto& t  = registry.get<TransformComponent>(e);
            auto& tv = registry.get<TransitVehicleComponent>(e);

            t.x = tv.track_x; // snap to track — no lateral drift

            if (tv.state == TransitVehicleComponent::STOPPED) {
                tv.stop_timer -= dt;
                if (tv.stop_timer <= 0.0f) {
                    // Depart toward next station; bounce at terminals
                    int nxt = tv.current_station + tv.direction;
                    if (nxt < 0)  { tv.direction =  1; nxt = tv.current_station + 1; }
                    if (nxt >= n) { tv.direction = -1; nxt = tv.current_station - 1; }
                    tv.next_station = nxt;
                    tv.state = TransitVehicleComponent::MOVING;
                }
            } else { // MOVING
                if (tv.next_station < 0 || tv.next_station >= n) continue;
                float targetY = stations_[tv.next_station].y;
                float dy      = targetY - t.y;
                if (std::abs(dy) < tv.speed * dt + 2.0f) {
                    // Arrived — snap and stop
                    t.y               = targetY;
                    tv.current_station = tv.next_station;
                    tv.state          = TransitVehicleComponent::STOPPED;
                    tv.stop_timer     = stations_[tv.next_station].stop_duration;
                } else {
                    t.y += (dy > 0.0f ? 1.0f : -1.0f) * tv.speed * dt;
                }
            }
        }
    }
};

// Advances the in-game clock and updates phase tint + spawn multiplier (gate at L1).
// Phase boundaries (game hours):
//   DAWN  05:00–07:30  (spawn ×0.4, purple-blue tint)
//   DAY   07:30–18:00  (spawn ×1.0, dark slate)
//   DUSK  18:00–21:00  (spawn ×0.6, warm orange)
//   NIGHT 21:00–05:00  (spawn ×0.15, deep blue-black)
class TimeOfDaySystem {
public:
    void update(Registry& registry, float dt) {
        auto view = registry.view<TimeOfDayComponent>();
        for (Entity e : view) {
            auto& tod = registry.get<TimeOfDayComponent>(e);
            // Advance clock: time_scale is game-minutes per real second
            tod.game_hour += (tod.time_scale / 60.0f) * dt;
            if (tod.game_hour >= 24.0f) tod.game_hour -= 24.0f;

            float h = tod.game_hour;
            TimeOfDay newPhase;
            if (h >= 5.0f && h < 7.5f) {
                newPhase = TimeOfDay::DAWN;
                tod.spawn_multiplier = 0.4f;
                tod.ambient_target = 16.0f;
                tod.tint_r = 45; tod.tint_g = 40; tod.tint_b = 58;
            } else if (h >= 7.5f && h < 18.0f) {
                newPhase = TimeOfDay::DAY;
                tod.spawn_multiplier = 1.0f;
                tod.ambient_target = 22.0f;
                tod.tint_r = 40; tod.tint_g = 42; tod.tint_b = 45;
            } else if (h >= 18.0f && h < 21.0f) {
                newPhase = TimeOfDay::DUSK;
                tod.spawn_multiplier = 0.6f;
                tod.ambient_target = 18.0f;
                tod.tint_r = 55; tod.tint_g = 38; tod.tint_b = 30;
            } else {
                newPhase = TimeOfDay::NIGHT;
                tod.spawn_multiplier = 0.15f;
                tod.ambient_target = 12.0f;
                tod.tint_r = 12; tod.tint_g = 15; tod.tint_b = 22;
            }

            if (newPhase != tod.phase) {
                int roll = std::rand() % 100;
                if (roll < 70) tod.weather = WeatherState::CLEAR;
                else if (roll < 85) tod.weather = WeatherState::OVERCAST;
                else if (roll < 95) tod.weather = WeatherState::ACID_RAIN;
                else tod.weather = WeatherState::SMOG;
            }

            tod.phase = newPhase;
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
// Inflict an injury on entity e. Emplaces InjuryComponent if absent.
// Finds the first free slot (type NONE) and fills it. Silent no-op if all 4 slots occupied.
inline void inflictInjury(Registry& registry, Entity e, InjuryType type, float severity) {
    if (!registry.has<InjuryComponent>(e))
        registry.assign<InjuryComponent>(e);
    auto& inj = registry.get<InjuryComponent>(e);
    for (auto& slot : inj.slots) {
        if (slot.type == InjuryType::NONE) {
            slot.type     = type;
            slot.severity = std::clamp(severity, 0.0f, 1.0f);
            return;
        }
    }
    // All slots occupied — silent drop
}

class TemperatureSystem {
public:
    void update(Registry& registry, float dt, float time_scale) {
        auto configView = registry.view<WorldConfigComponent, TemperatureGridComponent>();
        if (configView.empty()) return;

        Entity configEntity = configView[0];
        auto& config = registry.get<WorldConfigComponent>(configEntity);
        auto& tempGrid = registry.get<TemperatureGridComponent>(configEntity);

        if (tempGrid.grid.empty()) return;

        auto& tod = registry.get<TimeOfDayComponent>(configEntity);

        // 1. Weather Evolution
        // Very simple probabilistic state machine for weather
        if (rand() % 1000 == 0) { // Check every L2 tick occasionally
            if (tod.weather == WeatherState::CLEAR) {
                tod.weather = WeatherState::OVERCAST;
            } else if (tod.weather == WeatherState::OVERCAST) {
                // If it's cool and overcast, high chance of Acid Rain
                if (tod.ambient_target < 25.0f && (rand() % 2 == 0)) {
                    tod.weather = WeatherState::ACID_RAIN;
                } else {
                    tod.weather = (rand() % 2 == 0) ? WeatherState::CLEAR : WeatherState::SMOG;
                }
            } else {
                tod.weather = WeatherState::OVERCAST;
            }
        }

        int cols = tempGrid.cols;
        int rows = tempGrid.rows;

        const float diffusion = 0.1f;
        const float equilibrium_rate = 0.05f;

        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int idx = y * cols + x;
                float current = tempGrid.grid[idx];

                float sum = 0.0f;
                int neighbors = 0;
                if (x > 0)        { sum += tempGrid.grid[idx - 1];    ++neighbors; }
                if (x < cols - 1) { sum += tempGrid.grid[idx + 1];    ++neighbors; }
                if (y > 0)        { sum += tempGrid.grid[idx - cols]; ++neighbors; }
                if (y < rows - 1) { sum += tempGrid.grid[idx + cols]; ++neighbors; }

                float avg = (neighbors > 0) ? (sum / neighbors) : current;

                float next = current + diffusion * (avg - current);

                next += equilibrium_rate * (tod.ambient_target - next) * time_scale * dt;

                tempGrid.back[idx] = next;
            }
        }

        auto zoneView = registry.view<ZoningComponent, TransformComponent>();
        for (Entity e : zoneView) {
            auto& z = registry.get<ZoningComponent>(e);
            auto& t = registry.get<TransformComponent>(e);

            int cx = static_cast<int>((t.x - config.world_min) / config.macro_cell_size);
            int cy = static_cast<int>((t.y - config.world_min) / config.macro_cell_size);

            if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
                float injection = 0.0f;
                if (z.type == ZoneType::URBAN_CORE) injection = 0.5f;
                else if (z.type == ZoneType::INDUSTRIAL) injection = 0.8f;

                if (injection > 0.0f) {
                    tempGrid.back[cy * cols + cx] += injection * time_scale * dt;
                }
            }
        }

        tempGrid.grid = tempGrid.back;

        // 3. Interior Environmental Buffering
        auto buildingView = registry.view<BuildingAtmosphereComponent, TransformComponent>();
        for (Entity e : buildingView) {
            auto& bAtmo = registry.get<BuildingAtmosphereComponent>(e);
            auto& t = registry.get<TransformComponent>(e);

            float extTemp = getTempAt(config, tempGrid, t.x, t.y);
            const float insulation_factor = 0.01f;
            bAtmo.temperature += insulation_factor * (extTemp - bAtmo.temperature) * time_scale * dt;
        }
    }

    static float getTempAt(const WorldConfigComponent& config, const TemperatureGridComponent& tempGrid, float x, float y) {
        if (tempGrid.grid.empty()) return 20.0f;
        int cx = static_cast<int>((x - config.world_min) / config.macro_cell_size);
        int cy = static_cast<int>((y - config.world_min) / config.macro_cell_size);
        if (cx >= 0 && cx < tempGrid.cols && cy >= 0 && cy < tempGrid.rows) {
            return tempGrid.grid[cy * tempGrid.cols + cx];
        }
        return 20.0f;
    }
};

class AtmosphereSystem {
public:
    void update(Registry& registry, float dt, float time_scale) {
        auto configView = registry.view<WorldConfigComponent, AtmosphereGridComponent, TimeOfDayComponent>();
        if (configView.empty()) return;

        Entity configEntity = configView[0];
        auto& config = registry.get<WorldConfigComponent>(configEntity);
        auto& atmoGrid = registry.get<AtmosphereGridComponent>(configEntity);
        auto& tod = registry.get<TimeOfDayComponent>(configEntity);

        if (atmoGrid.grid.empty()) return;

        int cols = atmoGrid.cols;
        int rows = atmoGrid.rows;

        const float diffusion = 0.1f;
        const float equilibrium_rate = 0.01f;
        const float ambient_target = (tod.weather == WeatherState::SMOG) ? 50.0f : 100.0f;

        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int idx = y * cols + x;
                float current = atmoGrid.grid[idx];

                float sum = 0.0f;
                int neighbors = 0;
                if (x > 0)        { sum += atmoGrid.grid[idx - 1];    ++neighbors; }
                if (x < cols - 1) { sum += atmoGrid.grid[idx + 1];    ++neighbors; }
                if (y > 0)        { sum += atmoGrid.grid[idx - cols]; ++neighbors; }
                if (y < rows - 1) { sum += atmoGrid.grid[idx + cols]; ++neighbors; }

                float avg = (neighbors > 0) ? (sum / neighbors) : current;

                float next = current + diffusion * (avg - current);

                next += equilibrium_rate * (ambient_target - next) * time_scale * dt;

                atmoGrid.back[idx] = next;
            }
        }

        auto zoneView = registry.view<ZoningComponent, TransformComponent>();
        for (Entity e : zoneView) {
            auto& z = registry.get<ZoningComponent>(e);
            auto& t = registry.get<TransformComponent>(e);

            int cx = static_cast<int>((t.x - config.world_min) / config.macro_cell_size);
            int cy = static_cast<int>((t.y - config.world_min) / config.macro_cell_size);

            if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
                float injection = 0.0f;
                if (z.type == ZoneType::INDUSTRIAL) injection = -1.0f; // smog reduces quality
                else if (z.type == ZoneType::SLUM) injection = -0.2f;

                if (injection < 0.0f) {
                    atmoGrid.back[cy * cols + cx] += injection * time_scale * dt;
                    if (atmoGrid.back[cy * cols + cx] < 0.0f) atmoGrid.back[cy * cols + cx] = 0.0f;
                }
            }
        }

        atmoGrid.grid = atmoGrid.back;

        // Interior buffering
        auto buildingView = registry.view<BuildingAtmosphereComponent, TransformComponent>();
        for (Entity e : buildingView) {
            auto& bAtmo = registry.get<BuildingAtmosphereComponent>(e);
            auto& t = registry.get<TransformComponent>(e);

            float extAtmo = getAtmoAt(config, atmoGrid, t.x, t.y);
            const float ventilation_factor = 0.05f;
            bAtmo.air_quality += ventilation_factor * (extAtmo - bAtmo.air_quality) * time_scale * dt;
        }
    }

    static float getAtmoAt(const WorldConfigComponent& config, const AtmosphereGridComponent& atmoGrid, float x, float y) {
        if (atmoGrid.grid.empty()) return 100.0f;
        int cx = static_cast<int>((x - config.world_min) / config.macro_cell_size);
        int cy = static_cast<int>((y - config.world_min) / config.macro_cell_size);
        if (cx >= 0 && cx < atmoGrid.cols && cy >= 0 && cy < atmoGrid.rows) {
            return atmoGrid.grid[cy * atmoGrid.cols + cx];
        }
        return 100.0f;
    }
};

class StructuralDecaySystem {
public:
    void update(Registry& registry, float dt, float time_scale,
                SimulationAlertStack* alerts = nullptr) {
        auto configView = registry.view<WorldConfigComponent, TimeOfDayComponent>();
        if (configView.empty()) return;

        auto& tod = registry.get<TimeOfDayComponent>(configView[0]);
        bool is_acid_rain = (tod.weather == WeatherState::ACID_RAIN);

        auto view = registry.view<StructuralComponent>();
        for (Entity e : view) {
            auto& s = registry.get<StructuralComponent>(e);

            if (registry.has<CollapsedComponent>(e)) continue;

            float base_decay = 0.0f;
            switch (s.material_type) {
                case MaterialType::STEEL:               base_decay = 0.0001f; break;
                case MaterialType::REINFORCED_CONCRETE: base_decay = 0.0002f; break;
                case MaterialType::COMPOSITE:           base_decay = 0.00005f; break;
                case MaterialType::SCRAP:               base_decay = 0.001f; break;
            }

            float multiplier = 1.0f;
            if (is_acid_rain && s.is_exposed) {
                multiplier = 50.0f;
            }

            s.integrity -= base_decay * multiplier * time_scale * dt;

            if (s.integrity <= 0.0f) {
                s.integrity = 0.0f;
                registry.assign<CollapsedComponent>(e);
                if (alerts) {
                    alerts->push(SimulationAlertSeverity::DANGER,
                                 SimulationAlertCategory::STRUCTURE,
                                 "STRUCTURE COLLAPSE DETECTED",
                                 tod.game_hour);
                }
            }
        }
    }
};

class BiologySystem {
public:
    // Runs at L1 tick gate. time_scale = game-minutes per real-second (from TimeOfDayComponent).
    // Pushes a ViolenceEvent for each citizen death so MemoryFormationSystem can broadcast it.
    void update(Registry& registry, float dt, float time_scale,
                const WorldConfigComponent& config, const TemperatureGridComponent& tempGrid,
                const TimeOfDayComponent& tod,
                std::vector<ViolenceEvent>& outEvents) {
        std::vector<Entity> dead;

        auto view = registry.view<BiologyComponent>();
        for (Entity e : view) {
            auto& b = registry.get<BiologyComponent>(e);

            // Decay scaled to game-time so biology tracks the day cycle.
            b.hunger  -= 0.5f * time_scale * dt;
            b.thirst  -= 0.8f * time_scale * dt;
            b.fatigue -= 0.2f * time_scale * dt;

            if (b.hunger  < 0.0f) b.hunger  = 0.0f;
            if (b.thirst  < 0.0f) b.thirst  = 0.0f;
            if (b.fatigue < 0.0f) b.fatigue = 0.0f;

            // Health cascade when starving or dehydrated.
            if (b.hunger < 10.0f || b.thirst < 10.0f) {
                b.health         -= 1.0f * time_scale * dt;
                b.organs.liver   -= 0.1f * time_scale * dt;
                b.organs.kidneys -= 0.1f * time_scale * dt;
            }

            // Temperature health drain
            if (registry.has<TransformComponent>(e)) {
                float temp = 20.0f;
                if (registry.has<InteriorComponent>(e)) {
                    Entity building = registry.get<InteriorComponent>(e).building_entity;
                    if (registry.alive(building) && registry.has<BuildingAtmosphereComponent>(building)) {
                        temp = registry.get<BuildingAtmosphereComponent>(building).temperature;
                    } else {
                        auto& t = registry.get<TransformComponent>(e);
                        temp = TemperatureSystem::getTempAt(config, tempGrid, t.x, t.y);
                    }
                } else {
                    auto& t = registry.get<TransformComponent>(e);
                    temp = TemperatureSystem::getTempAt(config, tempGrid, t.x, t.y);
                }

                if (temp > 45.0f || temp < -5.0f) {
                    b.health -= 2.0f * time_scale * dt;
                }

                // Acid Rain health drain
                if (tod.weather == WeatherState::ACID_RAIN && !registry.has<InteriorComponent>(e)) {
                    b.health -= 0.5f * time_scale * dt;
                }
            }

            if (b.organs.heart < 0.0f)   b.organs.heart = 0.0f;
            if (b.organs.lungs < 0.0f)   b.organs.lungs = 0.0f;
            if (b.organs.brain < 0.0f)   b.organs.brain = 0.0f;
            if (b.organs.liver < 0.0f)   b.organs.liver = 0.0f;
            if (b.organs.kidneys < 0.0f) b.organs.kidneys = 0.0f;

            if (b.health < 0.0f) b.health = 0.0f;
            if (b.health >= 100.0f) b.health = 100.0f;

            // Mark citizens for death; player death handled by gameplay layer later.
            // Critical organ failure (Brain/Heart) or zero overall health triggers death.
            bool is_dead = (b.health <= 0.0f || b.organs.brain <= 0.0f || b.organs.heart <= 0.0f);
            if (is_dead && registry.has<CitizenComponent>(e)) {
                if (registry.has<TransformComponent>(e)) {
                    auto& t = registry.get<TransformComponent>(e);
                    outEvents.push_back({t.x, t.y});
                }
                dead.push_back(e);
            }
        }

        for (Entity e : dead) registry.destroy(e);
    }
};

class PathogenSystem {
    static bool sameSharedInterior(Registry& registry, Entity a, Entity b) {
        if (!registry.has<InteriorComponent>(a) || !registry.has<InteriorComponent>(b)) return false;
        Entity ba = registry.get<InteriorComponent>(a).building_entity;
        Entity bb = registry.get<InteriorComponent>(b).building_entity;
        return ba != MAX_ENTITIES && ba == bb;
    }

    static bool bothOutdoors(Registry& registry, Entity a, Entity b) {
        return !registry.has<InteriorComponent>(a) && !registry.has<InteriorComponent>(b);
    }

    static void applyExposure(Registry& registry, Entity target, const PathogenComponent& source, float dose) {
        if (dose <= 0.0f) return;

        if (!registry.has<PathogenComponent>(target)) {
            PathogenComponent seeded;
            seeded.strain_id = source.strain_id;
            seeded.infection_load = std::min(100.0f, dose);
            seeded.infectiousness = std::max(0.05f, source.infectiousness * 0.75f);
            seeded.incubation_timer = 45.0f;
            registry.assign<PathogenComponent>(target, seeded);
            return;
        }

        auto& targetPathogen = registry.get<PathogenComponent>(target);
        if (targetPathogen.strain_id != source.strain_id) return;
        targetPathogen.infection_load = std::min(100.0f, targetPathogen.infection_load + dose);
    }

public:
    static size_t infectedCount(Registry& registry) {
        return registry.view<PathogenComponent>().size();
    }

    void update(Registry& registry, float dt, float time_scale, bool run_l2 = false,
                float game_hour = 0.0f, SimulationAlertStack* alerts = nullptr) {
        float scaled_dt = time_scale * dt;
        auto bioEntities = registry.view<BiologyComponent, TransformComponent>();

        for (Entity source : bioEntities) {
            if (!registry.has<PathogenComponent>(source)) continue;

            auto& sp = registry.get<PathogenComponent>(source);
            if (sp.infection_load < 20.0f || sp.infectiousness <= 0.0f) continue;

            auto& st = registry.get<TransformComponent>(source);
            for (Entity target : bioEntities) {
                if (target == source) continue;

                float dose_multiplier = 0.0f;
                if (sameSharedInterior(registry, source, target)) {
                    dose_multiplier = 0.35f;
                } else if (bothOutdoors(registry, source, target)) {
                    auto& tt = registry.get<TransformComponent>(target);
                    float dx = st.x - tt.x;
                    float dy = st.y - tt.y;
                    if (dx * dx + dy * dy <= 24.0f * 24.0f) {
                        dose_multiplier = 1.0f;
                    }
                }

                if (dose_multiplier <= 0.0f) continue;
                float dose = sp.infectiousness * dose_multiplier * scaled_dt;
                applyExposure(registry, target, sp, dose);
            }
        }

        std::vector<Entity> cleared;
        auto infected = registry.view<PathogenComponent, BiologyComponent>();
        for (Entity e : infected) {
            auto& p = registry.get<PathogenComponent>(e);
            auto& b = registry.get<BiologyComponent>(e);

            p.incubation_timer = std::max(0.0f, p.incubation_timer - scaled_dt);
            p.infection_load = std::max(0.0f, p.infection_load - p.immune_response * scaled_dt);

            if (p.incubation_timer <= 0.0f && p.infection_load > 20.0f) {
                float severity_gain = (p.infection_load / 100.0f) * 0.02f * scaled_dt;
                p.severity = std::min(1.0f, p.severity + severity_gain);
            } else {
                p.severity = std::max(0.0f, p.severity - 0.01f * scaled_dt);
            }

            if (p.severity > 0.0f) {
                b.health -= p.severity * 0.25f * scaled_dt;
                b.organs.lungs = std::max(0.0f, b.organs.lungs - p.severity * 0.04f * scaled_dt);
                b.vitals.oxygen_sat = std::max(0.70f, b.vitals.oxygen_sat - p.severity * 0.002f * scaled_dt);
                b.vitals.heart_rate = std::min(150.0f, b.vitals.heart_rate + p.severity * 0.20f * scaled_dt);
                if (alerts) {
                    emitInfectionAlertIfWorse(*alerts, p, e, game_hour);
                }
            } else {
                b.vitals.oxygen_sat = std::min(1.0f, b.vitals.oxygen_sat + 0.001f * scaled_dt);
                b.vitals.heart_rate += (80.0f - b.vitals.heart_rate) * std::min(1.0f, 0.02f * scaled_dt);
            }

            if (p.infection_load <= 0.1f && p.severity <= 0.0f && p.incubation_timer <= 0.0f) {
                cleared.push_back(e);
            }
        }

        if (run_l2) {
            auto cognitiveInfections = registry.view<PathogenComponent, CognitiveComponent>();
            for (Entity e : cognitiveInfections) {
                auto& p = registry.get<PathogenComponent>(e);
                if (p.severity < 0.15f) continue;

                auto& cog = registry.get<CognitiveComponent>(e);
                cog.pleasure = std::max(-1.0f, cog.pleasure - 0.03f * p.severity);
                cog.arousal  = std::min( 1.0f, cog.arousal  + 0.02f * p.severity);

                if (p.severity >= 0.35f && !p.trauma_recorded) {
                    cog.record({MemoryEventType::BECAME_ILL, game_hour, p.severity, e});
                    p.trauma_recorded = true;
                }
            }
        }

        for (Entity e : cleared) {
            registry.remove<PathogenComponent>(e);
        }
    }
};

// Runs at L1 (every frame). Evolves injury severity per type, drains health, and (when
// run_l2 == true) cascades cumulative injury pain to CognitiveComponent.pleasure.
class InjurySystem {
public:
    void update(Registry& registry, float dt, float time_scale, bool run_l2) {
        // Pass 1 — severity evolution + health drain
        auto view = registry.view<InjuryComponent>();
        for (Entity e : view) {
            auto& inj = registry.get<InjuryComponent>(e);
            bool has_bio = registry.has<BiologyComponent>(e);
            BiologyComponent* bio = has_bio ? &registry.get<BiologyComponent>(e) : nullptr;

            for (auto& slot : inj.slots) {
                if (slot.type == InjuryType::NONE) continue;

                // Per-type severity delta per real second (NOT scaled by time_scale)
                float delta = 0.0f;
                switch (slot.type) {
                    case InjuryType::LACERATION:         delta = -0.002f; break;
                    case InjuryType::INTERNAL_BLEEDING:  delta = +0.005f; break;
                    case InjuryType::BROKEN_LIMB:        delta = -0.0005f; break;
                    case InjuryType::CONCUSSION:         delta = -0.001f; break;
                    case InjuryType::BURN:               delta = -0.001f; break;
                    default: break;
                }
                slot.severity = std::clamp(slot.severity + delta * dt, 0.0f, 1.0f);

                if (slot.severity == 0.0f) {
                    slot.type = InjuryType::NONE;
                    continue;
                }

                // Health drain proportional to severity (time_scale scaled)
                if (bio)
                    bio->health -= slot.severity * 0.5f * time_scale * dt;
            }

            if (bio)
                bio->health = std::max(0.0f, bio->health);
        }

        // Pass 2 — PAD pleasure cascade (L2 cadence only)
        if (!run_l2) return;
        auto cogview = registry.view<InjuryComponent, CognitiveComponent>();
        for (Entity e : cogview) {
            auto& inj = registry.get<InjuryComponent>(e);
            float total = 0.0f;
            for (const auto& slot : inj.slots) {
                if (slot.type != InjuryType::NONE)
                    total += slot.severity;
            }
            if (total > 0.5f) {
                auto& cog = registry.get<CognitiveComponent>(e);
                cog.pleasure = std::max(-1.0f, cog.pleasure - 0.1f);
            }
        }
    }
};

// Broadcasts violence events to nearby citizen memories (runs at L1, after BiologySystem).
// For each death position, citizens within 80 WU record SAW_VIOLENCE and shift PAD.
class MemoryFormationSystem {
public:
    static constexpr float WITNESS_RADIUS_SQ = 80.0f * 80.0f; // 6400 WU²

    void update(Registry& registry, const std::vector<ViolenceEvent>& events, float game_hour) {
        if (events.empty()) return;

        auto witnesses = registry.view<TransformComponent, CognitiveComponent>();
        for (Entity e : witnesses) {
            auto& t   = registry.get<TransformComponent>(e);
            auto& cog = registry.get<CognitiveComponent>(e);

            for (const ViolenceEvent& ev : events) {
                float dx = ev.x - t.x, dy = ev.y - t.y;
                if (dx*dx + dy*dy > WITNESS_RADIUS_SQ) continue;

                cog.record({ MemoryEventType::SAW_VIOLENCE, game_hour, 0.8f, MAX_ENTITIES });
                cog.pleasure = std::max(-1.0f, cog.pleasure - 0.3f);
                cog.arousal  = std::min( 1.0f, cog.arousal  + 0.4f);
            }
        }
    }
};

// ── L2 Rumor System ───────────────────────────────────────────────────────────
// Runs at L2, AFTER RelationshipSystem. Direct witnesses (SAW_VIOLENCE, intensity ≥ 0.5,
// within last 2 game-hours) tell nearby friends (affinity ≥ 0.2, ≤ 30 WU) about the event.
// Friends record HEARD_RUMOR with intensity 0.4 and receive a mild PAD shift.
// Single-hop only: HEARD_RUMOR recipients do NOT re-propagate.
class RumorSystem {
    // True if cog has SAW_VIOLENCE (intensity ≥ 0.5) recorded within `window` game-hours.
    static bool hasDirectWitness(const CognitiveComponent& cog, float game_hour, float window) {
        for (int i = 0; i < cog.mem_size; ++i) {
            const auto& r = cog.memory[i];
            if (r.event != MemoryEventType::SAW_VIOLENCE || r.intensity < 0.5f) continue;
            float diff = std::abs(game_hour - r.timestamp);
            if (diff > 12.0f) diff = 24.0f - diff; // midnight wrap
            if (diff <= window) return true;
        }
        return false;
    }

    // True if cog has SAW_VIOLENCE or HEARD_RUMOR within `window` game-hours (dedup guard).
    static bool hasRecentViolence(const CognitiveComponent& cog, float game_hour, float window) {
        for (int i = 0; i < cog.mem_size; ++i) {
            const auto& r = cog.memory[i];
            if (r.event != MemoryEventType::SAW_VIOLENCE &&
                r.event != MemoryEventType::HEARD_RUMOR) continue;
            float diff = std::abs(game_hour - r.timestamp);
            if (diff > 12.0f) diff = 24.0f - diff;
            if (diff <= window) return true;
        }
        return false;
    }

public:
    void update(Registry& registry, float game_hour) {
        constexpr float WINDOW  = 2.0f;          // game-hours
        constexpr float PROX_SQ = 30.0f * 30.0f; // same radius as RelationshipSystem

        auto view = registry.view<CitizenComponent, TransformComponent,
                                   CognitiveComponent, RelationshipComponent>();
        for (Entity teller : view) {
            auto& cog_t = registry.get<CognitiveComponent>(teller);
            if (!hasDirectWitness(cog_t, game_hour, WINDOW)) continue;

            auto& ta  = registry.get<TransformComponent>(teller);
            auto& rel = registry.get<RelationshipComponent>(teller);

            for (int s = 0; s < rel.count; ++s) {
                if (rel.entries[s].affinity < 0.2f) continue;
                Entity friend_e = rel.entries[s].other;
                if (!registry.alive(friend_e)) continue;
                if (!registry.has<CognitiveComponent>(friend_e)) continue;
                if (!registry.has<TransformComponent>(friend_e)) continue;

                auto& tb = registry.get<TransformComponent>(friend_e);
                float dx = ta.x - tb.x, dy = ta.y - tb.y;
                if (dx*dx + dy*dy > PROX_SQ) continue;

                auto& cog_f = registry.get<CognitiveComponent>(friend_e);
                if (hasRecentViolence(cog_f, game_hour, WINDOW)) continue;

                cog_f.record({ MemoryEventType::HEARD_RUMOR, game_hour, 0.4f, teller });
                cog_f.pleasure = std::max(-1.0f, cog_f.pleasure - 0.15f);
                cog_f.arousal  = std::min( 1.0f, cog_f.arousal  + 0.20f);
            }
        }
    }
};

class ConsumableSystem {
    // Injury-aware MEDICAL item application. Clears/reduces worst injury slot if any;
    // falls back to direct health restore when no active injuries are present.
    static void applyMedical(Registry& registry, Entity e, BiologyComponent& bio, float restore_value) {
        if (registry.has<InjuryComponent>(e)) {
            auto& inj = registry.get<InjuryComponent>(e);
            // Find worst active slot
            int worst = -1;
            for (int i = 0; i < InjuryComponent::MAX_SLOTS; ++i) {
                if (inj.slots[i].type == InjuryType::NONE) continue;
                if (worst < 0 || inj.slots[i].severity > inj.slots[worst].severity)
                    worst = i;
            }
            if (worst >= 0) {
                inj.slots[worst].severity -= 0.5f;
                if (inj.slots[worst].severity <= 0.0f) {
                    inj.slots[worst].severity = 0.0f;
                    inj.slots[worst].type     = InjuryType::NONE;
                }
                return; // consumed on injury, don't also restore health
            }
        }
        // No injuries — restore health as before
        bio.health = std::min(100.0f, bio.health + restore_value);
    }

public:
    // Runs at L2 tick. Goal assignment is owned by GDISystem; this handles consumption only.
    void update(Registry& registry, float game_hour) {
        // --- Consume items on arrival (within 20 WU) ---
        std::vector<Entity> toDestroy;
        auto goalers = registry.view<CitizenComponent, GoalComponent, TransformComponent, BiologyComponent>();
        for (Entity e : goalers) {
            Entity item = registry.get<GoalComponent>(e).target_entity;
            if (item == MAX_ENTITIES || !registry.alive(item) || !registry.has<ItemComponent>(item)) {
                registry.get<GoalComponent>(e).target_entity = MAX_ENTITIES;
                continue;
            }
            auto& t  = registry.get<TransformComponent>(e);
            auto& it = registry.get<TransformComponent>(item);
            float dx = it.x - t.x, dy = it.y - t.y;
            if (dx*dx + dy*dy > 400.0f) continue; // 20 WU

            auto& bio = registry.get<BiologyComponent>(e);
            auto& ic  = registry.get<ItemComponent>(item);
            if      (ic.type == ItemComponent::FOOD)    bio.hunger = std::min(100.0f, bio.hunger + ic.restore_value);
            else if (ic.type == ItemComponent::WATER)   bio.thirst = std::min(100.0f, bio.thirst + ic.restore_value);
            else                                         applyMedical(registry, e, bio, ic.restore_value);

            // Record consumption memory and apply pleasure boost
            if (registry.has<CognitiveComponent>(e)) {
                auto& cog = registry.get<CognitiveComponent>(e);
                MemoryEventType evt = (ic.type == ItemComponent::FOOD)
                                      ? MemoryEventType::ATE_FOOD
                                      : MemoryEventType::DRANK_WATER;
                cog.record({ evt, game_hour, ic.restore_value / 100.0f, item });
                cog.pleasure = std::min(1.0f, cog.pleasure + 0.2f);
            }

            registry.get<GoalComponent>(e).target_entity = MAX_ENTITIES;
            toDestroy.push_back(item);
        }
        for (Entity e : toDestroy)
            if (registry.alive(e)) registry.destroy(e);
    }

    // Called on interact press. Returns true if an item was consumed (suppresses vehicle boarding).
    bool playerPickup(Registry& registry, Entity player) {
        if (!registry.has<TransformComponent>(player) || !registry.has<BiologyComponent>(player))
            return false;

        auto& pt  = registry.get<TransformComponent>(player);
        auto& bio = registry.get<BiologyComponent>(player);

        Entity best = MAX_ENTITIES;
        float bestDist = 900.0f; // 30 WU radius²

        auto items = registry.view<ItemComponent, TransformComponent>();
        for (Entity item : items) {
            auto& it = registry.get<TransformComponent>(item);
            float dx = it.x - pt.x, dy = it.y - pt.y;
            float d = dx*dx + dy*dy;
            if (d < bestDist) { bestDist = d; best = item; }
        }
        if (best == MAX_ENTITIES) return false;

        auto& ic = registry.get<ItemComponent>(best);
        if      (ic.type == ItemComponent::FOOD)    bio.hunger = std::min(100.0f, bio.hunger + ic.restore_value);
        else if (ic.type == ItemComponent::WATER)   bio.thirst = std::min(100.0f, bio.thirst + ic.restore_value);
        else                                         applyMedical(registry, player, bio, ic.restore_value);

        registry.destroy(best);
        return true;
    }

    // Called on interact press. Stores the nearest survival item in player inventory.
    std::pair<bool, ItemComponent::Type> playerCollectWithType(Registry& registry, Entity player) {
        if (!registry.has<TransformComponent>(player) || !registry.has<SurvivalInventoryComponent>(player))
            return {false, ItemComponent::FOOD};

        auto& pt = registry.get<TransformComponent>(player);

        Entity best = MAX_ENTITIES;
        float bestDist = 900.0f; // 30 WU radius squared

        auto items = registry.view<ItemComponent, TransformComponent>();
        for (Entity item : items) {
            auto& it = registry.get<TransformComponent>(item);
            float dx = it.x - pt.x, dy = it.y - pt.y;
            float d = dx*dx + dy*dy;
            if (d < bestDist) { bestDist = d; best = item; }
        }
        if (best == MAX_ENTITIES) return {false, ItemComponent::FOOD};

        auto& ic = registry.get<ItemComponent>(best);
        ItemComponent::Type t = ic.type;
        auto& inv = registry.get<SurvivalInventoryComponent>(player);
        if (t == ItemComponent::FOOD) {
            ++inv.food_count;
        } else if (t == ItemComponent::WATER) {
            ++inv.water_count;
        } else {
            ++inv.medical_count;
        }

        registry.destroy(best);
        return {true, t};
    }

    bool consumePlayerInventory(Registry& registry, Entity player, ItemComponent::Type type) {
        if (!registry.has<SurvivalInventoryComponent>(player) || !registry.has<BiologyComponent>(player))
            return false;

        auto& inv = registry.get<SurvivalInventoryComponent>(player);
        int* count = nullptr;
        if (type == ItemComponent::FOOD) {
            count = &inv.food_count;
        } else if (type == ItemComponent::WATER) {
            count = &inv.water_count;
        } else {
            count = &inv.medical_count;
        }

        if (!count || *count <= 0) return false;
        --(*count);

        auto& bio = registry.get<BiologyComponent>(player);
        if      (type == ItemComponent::FOOD)  bio.hunger = std::min(100.0f, bio.hunger + 40.0f);
        else if (type == ItemComponent::WATER) bio.thirst = std::min(100.0f, bio.thirst + 40.0f);
        else                                   applyMedical(registry, player, bio, 40.0f);
        return true;
    }

    // Returns {consumed, item_type}. Used by main.cpp to record cognitive memory.
    std::pair<bool, ItemComponent::Type> playerPickupWithType(Registry& registry, Entity player) {
        if (!registry.has<TransformComponent>(player) || !registry.has<BiologyComponent>(player))
            return {false, ItemComponent::FOOD};

        auto& pt  = registry.get<TransformComponent>(player);
        auto& bio = registry.get<BiologyComponent>(player);

        Entity best    = MAX_ENTITIES;
        float  bestDist = 900.0f;

        auto items = registry.view<ItemComponent, TransformComponent>();
        for (Entity item : items) {
            auto& it = registry.get<TransformComponent>(item);
            float dx = it.x - pt.x, dy = it.y - pt.y;
            float d  = dx*dx + dy*dy;
            if (d < bestDist) { bestDist = d; best = item; }
        }
        if (best == MAX_ENTITIES) return {false, ItemComponent::FOOD};

        auto& ic = registry.get<ItemComponent>(best);
        ItemComponent::Type t = ic.type;
        if      (t == ItemComponent::FOOD)    bio.hunger = std::min(100.0f, bio.hunger + ic.restore_value);
        else if (t == ItemComponent::WATER)   bio.thirst = std::min(100.0f, bio.thirst + ic.restore_value);
        else                                   applyMedical(registry, player, bio, ic.restore_value);

        registry.destroy(best);
        return {true, t};
    }
};

// ── L3 Wage System ────────────────────────────────────────────────────────────
// Runs at L3 tick (~1 Hz real-time). Pays employed citizens a per-tick wage slice
// proportional to time_scale. Applies pleasure penalty to broke unemployed citizens.
class WageSystem {
public:
    void update(Registry& registry, float time_scale) {
        auto view = registry.view<EconomicComponent>();
        for (Entity e : view) {
            auto& eco = registry.get<EconomicComponent>(e);

            if (eco.employer != MAX_ENTITIES) {
                if (!registry.alive(eco.employer)) {
                    // Employer destroyed — lay off citizen
                    eco.employer   = MAX_ENTITIES;
                    eco.daily_wage = 0.0f;
                } else {
                    // Pay wage slice: daily_wage * (game-minutes this tick) / (game-minutes per day)
                    eco.credits += eco.daily_wage * time_scale / 1440.0f;
                    if (eco.credits > 9999.0f) eco.credits = 9999.0f;
                }
            }

            // Financial stress → PAD feedback (broke + unemployed)
            if (eco.credits < 20.0f && eco.employer == MAX_ENTITIES) {
                if (registry.has<CognitiveComponent>(e)) {
                    auto& cog = registry.get<CognitiveComponent>(e);
                    cog.pleasure = std::max(-1.0f, cog.pleasure - 0.03f);
                }
            }
        }
    }
};

// ── L3 applyItemEffect ────────────────────────────────────────────────────────
// Applies the same biological restoration as ConsumableSystem for a given item
// type. Used by MarketSystem on purchase so no item entity is needed.
// Biology stats are 0-100; restore amounts match ItemComponent::restore_value default.
inline void applyItemEffect(Registry& registry, Entity npc, ItemComponent::Type type) {
    if (!registry.has<BiologyComponent>(npc)) return;
    auto& bio = registry.get<BiologyComponent>(npc);

    if (type == ItemComponent::FOOD) {
        bio.hunger = std::min(100.0f, bio.hunger + 40.0f);
    } else if (type == ItemComponent::WATER) {
        bio.thirst = std::min(100.0f, bio.thirst + 40.0f);
    } else { // MEDICAL
        // Reduce worst active injury slot by 0.5; fall back to health restore
        if (registry.has<InjuryComponent>(npc)) {
            auto& inj = registry.get<InjuryComponent>(npc);
            int worst = -1;
            float worstSev = 0.0f;
            for (int i = 0; i < InjuryComponent::MAX_SLOTS; ++i) {
                if (inj.slots[i].type != InjuryType::NONE && inj.slots[i].severity > worstSev) {
                    worstSev = inj.slots[i].severity;
                    worst = i;
                }
            }
            if (worst >= 0) {
                inj.slots[worst].severity -= 0.5f;
                if (inj.slots[worst].severity <= 0.0f)
                    inj.slots[worst] = InjurySlot{};
                return;
            }
        }
        bio.health = std::min(100.0f, bio.health + 20.0f);
    }
}

// ── L3 Market System ──────────────────────────────────────────────────────────
inline float effectiveMarketPrice(const MarketComponent& mc, float stock) {
    float ratio = std::clamp(stock / mc.max_stock, 0.1f, 2.0f);
    return mc.base_price / ratio;
}

inline float clampedMarketReputation(const EconomicComponent& eco) {
    return std::clamp(eco.market_reputation, -1.0f, 1.0f);
}

inline float marketGreedFactor(const MarketComponent& mc) {
    return std::clamp(1.0f + mc.greed_margin, 0.25f, 4.0f);
}

inline float marketReputationBuyFactor(const EconomicComponent& eco) {
    return std::clamp(1.0f - clampedMarketReputation(eco) * 0.15f, 0.75f, 1.25f);
}

inline float marketReputationSellFactor(const EconomicComponent& eco) {
    return std::clamp(1.0f + clampedMarketReputation(eco) * 0.15f, 0.75f, 1.25f);
}

inline float adjustedMarketBuyPrice(const MarketComponent& mc, float stock,
                                    const EconomicComponent& buyer) {
    float price = effectiveMarketPrice(mc, stock);
    price *= marketGreedFactor(mc);
    price *= marketReputationBuyFactor(buyer);
    return std::max(0.0f, price);
}

inline float adjustedMarketSellPayout(const MarketComponent& mc, float stock,
                                      const EconomicComponent& seller) {
    float payout = effectiveMarketPrice(mc, stock) * 0.5f;
    payout *= std::clamp(1.0f - mc.greed_margin * 0.5f, 0.25f, 1.5f);
    payout *= marketReputationSellFactor(seller);
    return std::max(0.0f, payout);
}

inline float& marketStockForType(MarketComponent& mc, ItemComponent::Type type) {
    if (type == ItemComponent::WATER) return mc.water_stock;
    if (type == ItemComponent::MEDICAL) return mc.medical_stock;
    return mc.food_stock;
}

inline const float& marketStockForType(const MarketComponent& mc, ItemComponent::Type type) {
    if (type == ItemComponent::WATER) return mc.water_stock;
    if (type == ItemComponent::MEDICAL) return mc.medical_stock;
    return mc.food_stock;
}

enum class PlayerTradeMode { NONE, BUY, SELL };

struct PlayerMarketTradePreview {
    bool active = false;
    bool available = false;
    PlayerTradeMode mode = PlayerTradeMode::NONE;
    Entity market = MAX_ENTITIES;
    ItemComponent::Type type = ItemComponent::FOOD;
    float price = 0.0f;
    float base_price = 0.0f;
    float credits = 0.0f;
    float stock = 0.0f;
    float greed_margin = 0.0f;
    float reputation = 0.0f;
    size_t inventory_slot = DiscreteInventoryComponent::CAPACITY;
    const char* reason = "NO TRADE";
};

inline bool isMarketTradeItem(ItemComponent::Type type) {
    return type == ItemComponent::FOOD ||
           type == ItemComponent::WATER ||
           type == ItemComponent::MEDICAL;
}

inline Entity nearestPlayerMarket(Registry& registry, Entity player, float trade_radius = 60.0f) {
    if (!registry.has<TransformComponent>(player)) return MAX_ENTITIES;

    const auto& pt = registry.get<TransformComponent>(player);
    const float maxDist = trade_radius * trade_radius;
    Entity best = MAX_ENTITIES;
    float bestDist = maxDist;
    auto markets = registry.view<MarketComponent, TransformComponent>();
    for (Entity market : markets) {
        const auto& mt = registry.get<TransformComponent>(market);
        float dx = mt.x - pt.x;
        float dy = mt.y - pt.y;
        float dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            best = market;
        }
    }
    return best;
}

inline PlayerMarketTradePreview previewPlayerMarketBuy(
    Registry& registry,
    Entity player,
    ItemComponent::Type type,
    float trade_radius = 60.0f
) {
    PlayerMarketTradePreview preview;
    preview.active = true;
    preview.mode = PlayerTradeMode::BUY;
    preview.type = type;

    if (!isMarketTradeItem(type)) {
        preview.reason = "UNSELLABLE ITEM";
        return preview;
    }
    if (!registry.has<EconomicComponent>(player) || !registry.has<DiscreteInventoryComponent>(player)) {
        preview.reason = "NO WALLET/BAG";
        return preview;
    }

    preview.market = nearestPlayerMarket(registry, player, trade_radius);
    preview.credits = registry.get<EconomicComponent>(player).credits;
    if (preview.market == MAX_ENTITIES) {
        preview.reason = "NO MARKET NEARBY";
        return preview;
    }

    const auto& market = registry.get<MarketComponent>(preview.market);
    const auto& eco = registry.get<EconomicComponent>(player);
    preview.stock = marketStockForType(market, type);
    preview.base_price = effectiveMarketPrice(market, preview.stock);
    preview.greed_margin = market.greed_margin;
    preview.reputation = clampedMarketReputation(eco);
    preview.price = adjustedMarketBuyPrice(market, preview.stock, eco);
    if (preview.stock < 1.0f) {
        preview.reason = "OUT OF STOCK";
        return preview;
    }
    if (preview.credits < preview.price) {
        preview.reason = "INSUFFICIENT CREDITS";
        return preview;
    }
    if (firstEmptyInventorySlot(registry.get<DiscreteInventoryComponent>(player)) < 0) {
        preview.reason = "BAG FULL";
        return preview;
    }

    preview.available = true;
    preview.reason = "CONFIRM";
    return preview;
}

inline bool selectNextMarketSellItem(DiscreteInventoryComponent& inventory) {
    constexpr size_t capacity = DiscreteInventoryComponent::CAPACITY;
    for (size_t offset = 0; offset < capacity; ++offset) {
        size_t index = (inventory.selected + 1 + offset) % capacity;
        const auto& item = inventory.slots[index];
        if (item.occupied && isMarketTradeItem(item.type)) {
            inventory.selected = index;
            return true;
        }
    }
    return false;
}

inline PlayerMarketTradePreview previewPlayerMarketSell(
    Registry& registry,
    Entity player,
    float trade_radius = 60.0f
) {
    PlayerMarketTradePreview preview;
    preview.active = true;
    preview.mode = PlayerTradeMode::SELL;

    if (!registry.has<EconomicComponent>(player) || !registry.has<DiscreteInventoryComponent>(player)) {
        preview.reason = "NO WALLET/BAG";
        return preview;
    }

    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    int selected = selectedInventorySlot(inventory);
    if (selected < 0 || !inventory.slots[static_cast<size_t>(selected)].occupied) {
        preview.reason = "NO ITEM SELECTED";
        return preview;
    }

    const auto& item = inventory.slots[static_cast<size_t>(selected)];
    preview.type = item.type;
    preview.inventory_slot = static_cast<size_t>(selected);
    if (!isMarketTradeItem(item.type)) {
        preview.reason = "ITEM NOT TRADED";
        return preview;
    }

    preview.market = nearestPlayerMarket(registry, player, trade_radius);
    preview.credits = registry.get<EconomicComponent>(player).credits;
    if (preview.market == MAX_ENTITIES) {
        preview.reason = "NO MARKET NEARBY";
        return preview;
    }

    const auto& market = registry.get<MarketComponent>(preview.market);
    const auto& eco = registry.get<EconomicComponent>(player);
    preview.stock = marketStockForType(market, item.type);
    preview.base_price = effectiveMarketPrice(market, preview.stock);
    preview.greed_margin = market.greed_margin;
    preview.reputation = clampedMarketReputation(eco);
    preview.price = adjustedMarketSellPayout(market, preview.stock, eco);
    preview.available = true;
    preview.reason = "CONFIRM";
    return preview;
}

inline bool executePlayerMarketTrade(Registry& registry, Entity player,
                                     const PlayerMarketTradePreview& preview) {
    if (!preview.active || !preview.available || preview.market == MAX_ENTITIES) return false;
    if (!registry.has<EconomicComponent>(player) || !registry.has<DiscreteInventoryComponent>(player) ||
        !registry.has<MarketComponent>(preview.market)) {
        return false;
    }

    auto& eco = registry.get<EconomicComponent>(player);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    auto& market = registry.get<MarketComponent>(preview.market);
    float& stock = marketStockForType(market, preview.type);

    if (preview.mode == PlayerTradeMode::BUY) {
        PlayerMarketTradePreview current = previewPlayerMarketBuy(registry, player, preview.type);
        if (!current.available) return false;
        uint32_t flags = ITEM_FLAG_LEGAL;
        ItemProvenance provenance = provenanceForMarketPurchase(flags, player, preview.market);
        if (!storeInventoryItem(inventory, preview.type, 40.0f, flags, preview.market, provenance)) {
            return false;
        }
        eco.credits -= current.price;
        stock = std::max(0.0f, stock - 1.0f);
        if (registry.has<SurvivalInventoryComponent>(player)) {
            incrementSurvivalCounter(registry.get<SurvivalInventoryComponent>(player), preview.type);
        }
        return true;
    }

    if (preview.mode == PlayerTradeMode::SELL) {
        PlayerMarketTradePreview current = previewPlayerMarketSell(registry, player);
        if (!current.available || current.inventory_slot != preview.inventory_slot ||
            current.type != preview.type) {
            return false;
        }
        auto& slot = inventory.slots[preview.inventory_slot];
        if (!slot.occupied || slot.type != preview.type) return false;

        eco.credits += current.price;
        stock = std::min(market.max_stock, stock + 1.0f);
        if (registry.has<SurvivalInventoryComponent>(player)) {
            decrementSurvivalCounter(registry.get<SurvivalInventoryComponent>(player), preview.type);
        }
        slot = {};
        return true;
    }

    return false;
}

// Runs at L3 tick (~1 Hz real-time).
// Pass 1 — restock: all stock fields grow at restock_rate units/sec, clamped to max_stock.
// Pass 2 — transactions: NPCs within 60 WU buy opportunistically based on need + credits.
//   Price = base_price / clamp(stock/max_stock, 0.1, 2.0) — scarce stock raises price.
class MarketSystem {
    // Effective price for a given stock level.
    static float effectivePrice(const MarketComponent& mc, float stock) {
        return effectiveMarketPrice(mc, stock);
    }

    // Execute one purchase: deduct credits, decrement stock, apply bio effect.
    static void doTransact(Registry& registry, Entity npc,
                           MarketComponent& mc, float& stock,
                           ItemComponent::Type type) {
        if (stock <= 0.0f) return;
        float price = effectivePrice(mc, stock);
        auto& eco = registry.get<EconomicComponent>(npc);
        if (eco.credits < price) return;

        eco.credits -= price;
        stock = std::max(0.0f, stock - 1.0f);
        applyItemEffect(registry, npc, type);
    }

public:
    void update(Registry& registry, float dt, float time_scale) {
        // ── Pass 1: restock ───────────────────────────────────────────────
        auto allMarkets = registry.view<MarketComponent>();
        for (Entity m : allMarkets) {
            auto& mc = registry.get<MarketComponent>(m);
            float gain = mc.restock_rate * dt * time_scale;
            mc.food_stock    = std::min(mc.max_stock, mc.food_stock    + gain);
            mc.water_stock   = std::min(mc.max_stock, mc.water_stock   + gain);
            mc.medical_stock = std::min(mc.max_stock, mc.medical_stock + gain);
        }

        // ── Pass 2: proximity-based NPC purchases ─────────────────────────
        // Build market list once so inner loop is a simple scan.
        struct MarketEntry { Entity e; float x, y; };
        std::vector<MarketEntry> markets;
        auto mView = registry.view<MarketComponent, TransformComponent>();
        for (Entity m : mView) {
            auto& t = registry.get<TransformComponent>(m);
            markets.push_back({m, t.x, t.y});
        }

        constexpr float RADIUS_SQ = 60.0f * 60.0f;

        auto citizens = registry.view<CitizenComponent, TransformComponent,
                                      BiologyComponent, EconomicComponent>();
        for (Entity e : citizens) {
            auto& ct  = registry.get<TransformComponent>(e);
            auto& bio = registry.get<BiologyComponent>(e);
            auto& eco = registry.get<EconomicComponent>(e);

            bool needFood  = bio.hunger < 40.0f;
            bool needWater = bio.thirst < 40.0f;
            if (!needFood && !needWater) continue;

            // Find nearest market within radius
            Entity nearest = MAX_ENTITIES;
            float  nearDist = RADIUS_SQ;
            for (auto& me : markets) {
                float dx = me.x - ct.x, dy = me.y - ct.y;
                float d  = dx*dx + dy*dy;
                if (d < nearDist) { nearDist = d; nearest = me.e; }
            }
            if (nearest == MAX_ENTITIES) continue;

            auto& mc = registry.get<MarketComponent>(nearest);

            if (needFood && mc.food_stock > 0.0f) {
                float price = effectivePrice(mc, mc.food_stock);
                if (eco.credits >= price)
                    doTransact(registry, e, mc, mc.food_stock, ItemComponent::FOOD);
            }
            if (needWater && mc.water_stock > 0.0f) {
                float price = effectivePrice(mc, mc.water_stock);
                if (eco.credits >= price)
                    doTransact(registry, e, mc, mc.water_stock, ItemComponent::WATER);
            }
        }
    }
};

class CognitiveSystem {
public:
    // Runs at L2 tick. Decays PAD toward neutral; feeds biology distress into PAD;
    // modulates citizen speed via arousal.
    void update(Registry& registry, float dt, float time_scale) {
        float decay = 0.05f * time_scale * dt;

        auto clamp = [](float v, float lo, float hi) {
            return v < lo ? lo : (v > hi ? hi : v);
        };
        auto decay_toward_zero = [&](float v) {
            if (v > 0.0f) return std::max(0.0f, v - decay);
            if (v < 0.0f) return std::min(0.0f, v + decay);
            return 0.0f;
        };

        auto view = registry.view<CognitiveComponent>();
        for (Entity e : view) {
            auto& cog = registry.get<CognitiveComponent>(e);

            // PAD decay toward neutral
            cog.pleasure  = decay_toward_zero(cog.pleasure);
            cog.arousal   = decay_toward_zero(cog.arousal);
            cog.dominance = decay_toward_zero(cog.dominance);

            // Biology → PAD feedback
            if (registry.has<BiologyComponent>(e)) {
                auto& bio = registry.get<BiologyComponent>(e);
                if (bio.hunger < 20.0f) {
                    cog.pleasure = clamp(cog.pleasure - 0.02f * time_scale * dt, -1.0f, 1.0f);
                    cog.arousal  = clamp(cog.arousal  + 0.01f * time_scale * dt, -1.0f, 1.0f);
                }
                if (bio.thirst < 20.0f) {
                    cog.pleasure = clamp(cog.pleasure - 0.03f * time_scale * dt, -1.0f, 1.0f);
                    cog.arousal  = clamp(cog.arousal  + 0.02f * time_scale * dt, -1.0f, 1.0f);
                }
            }

            // Arousal → citizen speed influence
            if (registry.has<CitizenComponent>(e)) {
                auto& c = registry.get<CitizenComponent>(e);
                c.speed = 30.0f * (1.0f + 0.5f * cog.arousal); // [15, 45] WU/s
            }
        }
    }
};

class PowerGridSystem {
public:
    // Runs at L2 or L3 tick. Aggregates supply and demand across the grid
    // and determines if nodes are powered.
    void update(Registry& registry, float dt, float time_scale) {
        float total_supply = 0.0f;
        float total_demand = 0.0f;

        auto nodes = registry.view<PowerNodeComponent>();
        for (Entity e : nodes) {
            auto& node = registry.get<PowerNodeComponent>(e);
            total_supply += node.supply;
            total_demand += node.demand;
        }

        bool grid_powered = total_supply >= total_demand && total_supply > 0.0f;

        for (Entity e : nodes) {
            auto& node = registry.get<PowerNodeComponent>(e);
            if (node.demand > 0.0f) {
                node.powered = grid_powered;
            } else if (node.supply > 0.0f) {
                node.powered = true;
            } else {
                node.powered = false;
            }
        }
    }
};
