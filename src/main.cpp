#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "components.h"
#include "ecs.h"
#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "stb_image.h"
#include "world_builder.h"
#include "world_generation.h"

static constexpr int FONT_GLYPH_W = 16;
static constexpr int FONT_GLYPH_H = 16;
static constexpr int FONT_COLS = 16;
static constexpr int FONT_FIRST_CHAR = 32;
static constexpr float BUILDING_INTERACTION_RANGE_WU = 18.0f;
static constexpr float INSPECTION_RANGE_WU = 22.0f;

static SDL_Texture* loadFontTexture(SDL_Renderer* renderer, const char* path) {
    int w = 0, h = 0, ch = 0;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        std::cerr << "Failed to load font: " << path << std::endl;
        return nullptr;
    }

    for (int i = 0; i < w * h * 4; i += 4) {
        data[i] = 255 - data[i];
        data[i + 1] = 255 - data[i + 1];
        data[i + 2] = 255 - data[i + 2];
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        data, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA32);
    if (!surf) {
        stbi_image_free(data);
        return nullptr;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
    SDL_FreeSurface(surf);
    stbi_image_free(data);
    return tex;
}

static SDL_Texture* loadFontTextureFromKnownPaths(SDL_Renderer* renderer) {
    const char* paths[] = {
        "assets/om_large_plain_black_rgba.png",
        "../assets/om_large_plain_black_rgba.png",
        "../../assets/om_large_plain_black_rgba.png",
        "/Users/gm1/Code/neon/assets/om_large_plain_black_rgba.png"
    };

    for (const char* path : paths) {
        if (SDL_Texture* font = loadFontTexture(renderer, path)) {
            return font;
        }
    }

    std::cerr << "Font texture unavailable; using rectangle-only fallback rendering." << std::endl;
    return nullptr;
}

static bool glyphSourceRect(char c, SDL_Rect& src) {
    int idx = static_cast<int>(static_cast<unsigned char>(c)) - FONT_FIRST_CHAR;
    if (idx < 0 || idx >= FONT_COLS * 6) return false;

    src = {(idx % FONT_COLS) * FONT_GLYPH_W,
           (idx / FONT_COLS) * FONT_GLYPH_H,
           FONT_GLYPH_W,
           FONT_GLYPH_H};
    return true;
}

static void drawGlyph(SDL_Renderer* renderer, SDL_Texture* font, char c,
                      int x, int y, SDL_Color color, float scale) {
    if (!font) return;
    SDL_Rect src{};
    if (!glyphSourceRect(c, src)) return;

    SDL_Rect dst = {x, y,
                    static_cast<int>(FONT_GLYPH_W * scale),
                    static_cast<int>(FONT_GLYPH_H * scale)};
    SDL_SetTextureColorMod(font, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font, color.a);
    SDL_RenderCopy(renderer, font, &src, &dst);
}

static void drawGlyphToRect(SDL_Renderer* renderer, SDL_Texture* font, char c,
                            const SDL_Rect& dst, SDL_Color color) {
    if (!font || dst.w <= 0 || dst.h <= 0) return;
    SDL_Rect src{};
    if (!glyphSourceRect(c, src)) return;

    SDL_SetTextureColorMod(font, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font, color.a);
    SDL_RenderCopy(renderer, font, &src, &dst);
}

static void drawText(SDL_Renderer* renderer, SDL_Texture* font, const char* text,
                     int x, int y, SDL_Color color, float scale) {
    int gw = static_cast<int>(FONT_GLYPH_W * scale);
    for (int i = 0; text[i]; ++i) {
        drawGlyph(renderer, font, text[i], x + i * gw, y, color, scale);
    }
}

static void drawTextCentered(SDL_Renderer* renderer, SDL_Texture* font, const char* text,
                             int cx, int cy, SDL_Color color, float scale) {
    int len = static_cast<int>(std::strlen(text));
    int gw = static_cast<int>(FONT_GLYPH_W * scale);
    int gh = static_cast<int>(FONT_GLYPH_H * scale);
    drawText(renderer, font, text, cx - (len * gw) / 2, cy - gh / 2, color, scale);
}

static void updatePlayer(Registry& registry, Entity player, float dt, const uint8_t* keys) {
    if (!registry.has<PlayerComponent>(player) || !registry.has<TransformComponent>(player)) return;
    if (registry.has<BuildingInteractionComponent>(player) &&
        registry.get<BuildingInteractionComponent>(player).inside_building) {
        return;
    }

    auto& player_component = registry.get<PlayerComponent>(player);
    auto current = registry.get<TransformComponent>(player);
    float dx = 0.0f;
    float dy = 0.0f;

    if (keys[SDL_SCANCODE_W]) dy -= 1.0f;
    if (keys[SDL_SCANCODE_S]) dy += 1.0f;
    if (keys[SDL_SCANCODE_A]) dx -= 1.0f;
    if (keys[SDL_SCANCODE_D]) dx += 1.0f;

    if (dx == 0.0f && dy == 0.0f) return;

    if (std::fabs(dx) > std::fabs(dy)) {
        player_component.facing = dx < 0.0f ? Facing::LEFT : Facing::RIGHT;
    } else {
        player_component.facing = dy < 0.0f ? Facing::UP : Facing::DOWN;
    }

    float len = std::sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;

    TransformComponent proposed = current;
    proposed.x += dx * player_component.speed * dt;
    proposed.y += dy * player_component.speed * dt;

    if (!transformOverlapsSolid(registry, proposed, player)) {
        registry.get<TransformComponent>(player) = proposed;
    }
}

static void toggleBuildingInteraction(Registry& registry, Entity player, float range_wu) {
    if (!registry.alive(player) || !registry.has<TransformComponent>(player) ||
        !registry.has<BuildingInteractionComponent>(player)) {
        return;
    }

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    if (interaction.inside_building) {
        interaction.inside_building = false;
        return;
    }

    const Entity nearest = nearestInteractableBuildingInRange(registry,
        registry.get<TransformComponent>(player),
        range_wu);
    if (nearest != MAX_ENTITIES) {
        interaction.building_entity = nearest;
        interaction.building_role = registry.get<BuildingUseComponent>(nearest).role;
        interaction.inside_building = true;
    }
}

static void performInspection(Registry& registry, Entity player, float range_wu) {
    if (!registry.alive(player) || !registry.has<InspectionComponent>(player) ||
        !registry.has<TransformComponent>(player)) {
        return;
    }

    auto& inspection = registry.get<InspectionComponent>(player);
    const InspectionTarget target = playerInspectionTarget(registry, player, range_wu);
    
    inspection.target_entity = target.entity;
    inspection.target_type = target.type;
    inspection.has_result = true;
}

static const char* locationStateName(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "OUTSIDE";
        case PlayerLocationState::NEAR_HOUSING: return "NEAR HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "INSIDE HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "NEAR WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "INSIDE WORKPLACE";
    }
    return "OUTSIDE";
}

static const char* locationPrompt(PlayerLocationState state) {
    switch (state) {
        case PlayerLocationState::OUTSIDE: return "";
        case PlayerLocationState::NEAR_HOUSING: return "E ENTER HOUSING";
        case PlayerLocationState::INSIDE_HOUSING: return "E EXIT HOUSING";
        case PlayerLocationState::NEAR_WORKPLACE: return "E ENTER WORKPLACE";
        case PlayerLocationState::INSIDE_WORKPLACE: return "E EXIT WORKPLACE";
    }
    return "";
}

static const char* inspectionTargetName(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::NONE: return "NO TARGET";
        case InspectionTargetType::HOUSING: return "HOUSING";
        case InspectionTargetType::WORKPLACE: return "WORKPLACE";
        case InspectionTargetType::PEDESTRIAN_PATH: return "PATH";
        case InspectionTargetType::WORKER: return "WORKER";
        case InspectionTargetType::HOUSING_INTERIOR: return "HOUSING INTERIOR";
        case InspectionTargetType::WORKPLACE_INTERIOR: return "WORKPLACE INTERIOR";
    }
    return "NO TARGET";
}

static const char* inspectionDetail(InspectionTargetType type) {
    switch (type) {
        case InspectionTargetType::NONE:
            return "Nothing close enough to read.";
        case InspectionTargetType::HOUSING:
            return "Private shelter. Enterable. Solid exterior.";
        case InspectionTargetType::WORKPLACE:
            return "Work site. Enterable. Linked to housing.";
        case InspectionTargetType::PEDESTRIAN_PATH:
            return "Foot path. Non-solid access between buildings.";
        case InspectionTargetType::WORKER:
            return "Fixed worker. Path route. Count locked at one.";
        case InspectionTargetType::HOUSING_INTERIOR:
            return "SLEEPING MAT: Tattered synthetic weave.";
        case InspectionTargetType::WORKPLACE_INTERIOR:
            return "WORK BENCH: Heavy machinery array.";
    }
    return "";
}

static void updateCamera(Registry& registry, Entity camera_entity, float screen_w, float screen_h) {
    if (!registry.has<CameraComponent>(camera_entity)) return;
    auto& camera = registry.get<CameraComponent>(camera_entity);
    camera.screenWidth = screen_w;
    camera.screenHeight = screen_h;

    if (camera.target_entity != MAX_ENTITIES &&
        registry.alive(camera.target_entity) &&
        registry.has<TransformComponent>(camera.target_entity)) {
        const auto& target = registry.get<TransformComponent>(camera.target_entity);
        camera.x = target.x;
        camera.y = target.y;
    }
}

static void renderWorld(SDL_Renderer* renderer, SDL_Texture* font, Registry& registry,
                        const CameraComponent& camera) {
    const float center_x = camera.screenWidth * 0.5f;
    const float center_y = camera.screenHeight * 0.5f;
    const float scale = camera.scale;
    const float view_left = camera.x - center_x / scale;
    const float view_right = camera.x + center_x / scale;
    const float view_top = camera.y - center_y / scale;
    const float view_bottom = camera.y + center_y / scale;

    auto view = registry.view<TransformComponent, GlyphComponent>();
    for (Entity e : view) {
        const auto& t = registry.get<TransformComponent>(e);
        if (t.x + t.width * 0.5f < view_left || t.x - t.width * 0.5f > view_right ||
            t.y + t.height * 0.5f < view_top || t.y - t.height * 0.5f > view_bottom) {
            continue;
        }

        const auto& g = registry.get<GlyphComponent>(e);
        SDL_Color color{g.r, g.g, g.b, g.a};
        SDL_Rect rect = {
            static_cast<int>(center_x + (t.x - t.width * 0.5f - camera.x) * scale),
            static_cast<int>(center_y + (t.y - t.height * 0.5f - camera.y) * scale),
            std::max(2, static_cast<int>(t.width * scale)),
            std::max(2, static_cast<int>(t.height * scale))
        };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        if (!font) {
            if (registry.has<SolidComponent>(e)) {
                SDL_SetRenderDrawColor(renderer, 26, 54, 78, 180);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else {
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 235);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 10, 12, 16, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
            continue;
        }

        if (g.tiled && !g.chars.empty()) {
            const float target_tile_w = std::max(4.0f, FONT_GLYPH_W * scale * g.scale);
            const float target_tile_h = std::max(4.0f, FONT_GLYPH_H * scale * g.scale);
            const int cols = std::max(1, static_cast<int>(std::round(rect.w / target_tile_w)));
            const int rows = std::max(1, static_cast<int>(std::round(rect.h / target_tile_h)));
            for (int row = 0; row < rows; ++row) {
                const int tile_y0 = rect.y + (rect.h * row) / rows;
                const int tile_y1 = rect.y + (rect.h * (row + 1)) / rows;
                for (int col = 0; col < cols; ++col) {
                    const int tile_x0 = rect.x + (rect.w * col) / cols;
                    const int tile_x1 = rect.x + (rect.w * (col + 1)) / cols;
                    SDL_Rect tile{tile_x0, tile_y0,
                                  std::max(1, tile_x1 - tile_x0),
                                  std::max(1, tile_y1 - tile_y0)};
                    drawGlyphToRect(renderer, font, g.chars[0], tile, color);
                }
            }
        } else if (!g.chars.empty()) {
            drawGlyphToRect(renderer, font, g.chars[0], rect, color);
        }
    }
}

int main(int, char**) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Neon Oubliette", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* font = loadFontTextureFromKnownPaths(renderer);

    Registry registry;
    WorldConfig world_config = makeSandboxConfig();
    world_config.workplace_micro_zone_count = 1;
    world_config.workplace_building_count = 1;
    world_config.fixed_worker_count = 1;
    buildWorld(registry, world_config);
    if (!validateWorld(registry, world_config)) {
        std::cerr << "Generated world validation failed during startup." << std::endl;
        if (font) SDL_DestroyTexture(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    deriveInfrastructure(registry, world_config);
    spawnFixedActors(registry, world_config);

    Entity player = registry.create();
    registry.assign<TransformComponent>(player, 0.0f, -115.0f, 12.0f, 12.0f);
    registry.assign<PlayerComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<InspectionComponent>(player);
    registry.assign<GlyphComponent>(player, std::string("@"),
        static_cast<uint8_t>(245), static_cast<uint8_t>(245), static_cast<uint8_t>(210),
        static_cast<uint8_t>(255), 1.0f, true, false);
    if (!validatePlayerSpawn(registry, player)) {
        std::cerr << "Player spawn validation failed during startup." << std::endl;
        if (font) SDL_DestroyTexture(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Entity camera_entity = registry.create();
    auto& camera = registry.assign<CameraComponent>(camera_entity);
    camera.target_entity = player;
    camera.scale = 2.0f;

    bool running = true;
    uint32_t last_ticks = SDL_GetTicks();

    while (running) {
        uint32_t now = SDL_GetTicks();
        float dt = std::min(0.05f, (now - last_ticks) / 1000.0f);
        last_ticks = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_E) {
                Entity near_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), BUILDING_INTERACTION_RANGE_WU);
                bool inside = registry.has<BuildingInteractionComponent>(player) && registry.get<BuildingInteractionComponent>(player).inside_building;
                if (near_worker != MAX_ENTITIES && !inside) {
                    auto& actor_comp = registry.get<FixedActorComponent>(near_worker);
                    actor_comp.acknowledged = !actor_comp.acknowledged;
                } else {
                    toggleBuildingInteraction(registry, player, BUILDING_INTERACTION_RANGE_WU);
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                performInspection(registry, player, INSPECTION_RANGE_WU);
            }
            if (event.type == SDL_MOUSEWHEEL && registry.has<CameraComponent>(camera_entity)) {
                auto& c = registry.get<CameraComponent>(camera_entity);
                c.scale = std::clamp(c.scale + event.wheel.y * 0.15f, 0.75f, 4.0f);
            }
        }

        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        updatePlayer(registry, player, dt, keys);
        updateFixedActors(registry, dt);

        int screen_w = 0;
        int screen_h = 0;
        SDL_GetRendererOutputSize(renderer, &screen_w, &screen_h);
        updateCamera(registry, camera_entity, static_cast<float>(screen_w), static_cast<float>(screen_h));
        const auto& active_camera = registry.get<CameraComponent>(camera_entity);

        SDL_SetRenderDrawColor(renderer, 10, 12, 16, 255);
        SDL_RenderClear(renderer);

        renderWorld(renderer, font, registry, active_camera);

        if (font) {
            SDL_Color hud{140, 230, 180, 230};
            char line[160];
            float fps = dt > 0.0f ? 1.0f / dt : 0.0f;
            std::snprintf(line, sizeof(line), "FPS:%03.0f ENT:%zu BASELINE:PLAYER + HOUSING + WORKPLACE + WORKER", fps, registry.entity_count());
            drawText(renderer, font, line, 6, 6, hud, 0.7f);
            std::snprintf(line, sizeof(line), "WASD MOVE  SPACE INSPECT  WHEEL ZOOM  ESC QUIT  CAM:%.0f,%.0f Z:%.2f",
                          active_camera.x, active_camera.y, active_camera.scale);
            drawText(renderer, font, line, 6, 20, SDL_Color{110, 190, 230, 220}, 0.65f);
            const PlayerLocationState location_state =
                playerLocationState(registry, player, BUILDING_INTERACTION_RANGE_WU);
            Entity near_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), BUILDING_INTERACTION_RANGE_WU);
            bool inside = registry.has<BuildingInteractionComponent>(player) && registry.get<BuildingInteractionComponent>(player).inside_building;
            
            if (near_worker != MAX_ENTITIES && !inside) {
                bool ack = registry.get<FixedActorComponent>(near_worker).acknowledged;
                std::snprintf(line, sizeof(line), "LOCATION:%s  E %s",
                              locationStateName(location_state),
                              ack ? "DISMISS WORKER [WORKER ACKNOWLEDGED]" : "ACKNOWLEDGE WORKER");
            } else {
                std::snprintf(line, sizeof(line), "LOCATION:%s  %s",
                              locationStateName(location_state),
                              locationPrompt(location_state));
            }
            drawText(renderer, font, line, 6, 34, SDL_Color{245, 205, 120, 230}, 0.65f);
            const bool can_inspect = playerInspectionTarget(registry, player, INSPECTION_RANGE_WU).entity != MAX_ENTITIES;
            std::snprintf(line, sizeof(line), "INSPECT:%s",
                          can_inspect ? "SPACE READ NEARBY" : "NO NEARBY TARGET");
            drawText(renderer, font, line, 6, 48, SDL_Color{180, 220, 190, 220}, 0.65f);
            if (registry.has<InspectionComponent>(player) &&
                registry.get<InspectionComponent>(player).has_result) {
                const auto& inspection = registry.get<InspectionComponent>(player);
                std::snprintf(line, sizeof(line), "READOUT:%s - %s",
                              inspectionTargetName(inspection.target_type),
                              inspectionDetail(inspection.target_type));
                drawText(renderer, font, line, 6, 62, SDL_Color{245, 230, 150, 230}, 0.65f);
            }
        }

        SDL_RenderPresent(renderer);
    }

    if (font) SDL_DestroyTexture(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
