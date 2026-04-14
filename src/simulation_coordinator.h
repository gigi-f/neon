#pragma once
#include <cstdint>

// Tick-rate scheduler for the 5-layer simulation.
// Prevents every simulation layer from running at 60 Hz.
//
// Layer rates (at 60 Hz base):
//   L0 Physics:   every frame  (60 Hz)
//   L1 Biology:   every 10f    (6 Hz)
//   L2 Cognitive: every 30f    (2 Hz)
//   L3 Economic:  every 60f    (1 Hz)
//   L4 Political: every 300f   (0.2 Hz)
//
// Usage in game loop:
//   coordinator.advance();
//   if (coordinator.tick_l0()) physicsSystem.update(registry, dt);
//   if (coordinator.tick_l1()) biologySystem.update(registry, dt);
//   ...
struct SimulationCoordinator {
    uint64_t frame = 0;

    bool tick_l0() const { return true; }
    bool tick_l1() const { return (frame % 10) == 0; }
    bool tick_l2() const { return (frame % 30) == 0; }
    bool tick_l3() const { return (frame % 60) == 0; }
    bool tick_l4() const { return (frame % 300) == 0; }

    void advance() { ++frame; }
};
