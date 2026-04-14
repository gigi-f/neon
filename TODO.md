# Neon Oubliette - TODO List

## Phase 1: Foundation
- [x] Basic C++ / SDL2 Setup
- [x] Lightweight ECS implementation
- [x] Basic Playable Area with Grid
- [x] Player Movement (WASD)
- [x] Basic AABB Collision & Obstacles
- [x] Asset pipeline preparation (stb_image, Player PNG)
- [x] Art Direction & Asset List documented

## Phase 2: World Navigation & Camera (Done)
- [x] Implement Camera System to follow the player
- [x] Render world relative to camera position
- [x] Expand the playable grid area to be larger than the screen

## Phase 3: Infrastructure & Stateful Vehicles (Current)
- [ ] Define Road segments and rendering
- [x] Implement Stateful Vehicles (owner, home location, purpose)
- [x] Implement Vehicle Interaction (Enter/Exit vehicles, player driving)
- [x] Vehicle movement logic (Player controlled vs AI driven)
- [ ] Implement simple Traffic Lights (RED/GREEN cycles)

## Phase 4: City Generation (from OpenSpec)
- [ ] Procedural block and lot subdivision (Chicago-style)
- [ ] Procedural building placement and rendering
- [ ] Distance-based building height gradients
