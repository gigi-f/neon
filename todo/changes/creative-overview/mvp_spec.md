# Neon Oubliette: MVP / Proof of Concept

This document defines the minimal viable product (MVP) for the **Neon Oubliette** SDL2 successor. The goal is to establish a functional technical foundation (The "Walking Simulator" phase) without deep simulation logic.

## 1. Technical Stack & Tooling

- **Language**: C++20
- **Build System**: CMake 3.20+
- **Graphics/Input**: SDL2 (Simple 2D Top-Down Renderer)
- **ECS**: EnTT (Entity Component System)
- **Math**: GLM (OpenGL Mathematics)
- **Logging**: spdlog

## 2. Minimal Project Structure

```text
/
├── CMakeLists.txt
├── src/
│   ├── main.cpp                 # Entry point & SDL Loop
│   ├── core/
│   │   ├── engine.cpp/h         # Main loop & system orchestration
│   │   ├── world.cpp/h          # Zone management
│   ├── ecs/
│   │   ├── components.h         # Position, Sprite, Velocity, Tag
│   │   ├── systems/             # Rendering, Movement, Input
│   ├── render/
│   │   ├── camera.cpp/h         # 2D Camera for scrolling
│   │   ├── renderer.cpp/h       # SDL_Renderer wrapper
```

## 3. Core Components (ECS)

| Component | Purpose |
|-----------|---------|
| `PositionComponent` | `glm::vec2` world coordinates. |
| `VelocityComponent` | `glm::vec2` for movement. |
| `RenderComponent` | `SDL_Color` and `glm::vec2` dimensions for basic box rendering. |
| `PlayerTag` | Identifies the player entity. |
| `BuildingTag` | Identifies static structural entities. |
| `NPCTag` | Identifies mobile agent entities. |

## 4. The Single-Zone Sandbox

### World Generation (Static Grid)
- **Size**: 128x128 World Units (WU).
- **Layout**: A single **URBAN_CORE** zone.
- **Buildings**: Generate a 4x4 grid of "Block" rectangles. Each block contains 4 "Building" boxes.
- **Roads**: 6 WU wide strips between blocks.

### Entities
- **Player**: A single 1x1 WU Green box.
- **NPCs**: 50 random 1x1 WU Grey boxes spawned in "Sidewalk" areas.
- **Buildings**: Various height Grey/Blue boxes representing structures.

## 5. Minimal Gameplay Loop

1. **Input System**: 
   - Capture `SDL_KEYDOWN`.
   - Update `VelocityComponent` of entity with `PlayerTag` based on WASD.
2. **Movement System**:
   - `Position += Velocity * DeltaTime`.
   - Implement basic AABB collision between `PlayerTag` and `BuildingTag`.
3. **Camera System**:
   - The camera SHALL follow the Player entity, keeping them centered.
4. **Render System**:
   - Clear Screen (Deep Black: `#0A0A0F`).
   - Draw all `BuildingTag` entities as outlined rectangles.
   - Draw all `NPCTag` entities as solid small squares.
   - Draw `PlayerTag` entity as a solid Green square.
   - Present Screen.

## 6. Success Criteria for PoC

- [ ] Project compiles using CMake with all dependencies linked.
- [ ] An SDL2 window opens and displays a black background.
- [ ] A 128x128 grid of buildings is generated and rendered correctly.
- [ ] The player can move around the zone using WASD.
- [ ] The camera follows the player smoothly.
- [ ] Static NPCs are visible in the world.
- [ ] AABB collision prevents the player from walking through buildings.

## 7. Next Steps (Post-MVP)

1. Integrate **Lucide/Heroicons** as textures for the boxes.
2. Implement the **L0 Physics Layer** (Structural Integrity tracking).
3. Add the **Acoustic Stress Probe** (L0 Investigative Tool).
