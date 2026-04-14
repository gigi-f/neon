// feature_flags.cpp
// Implementation for Neon Oubliette Feature Flags
#include "feature_flags.h"
#include <iostream>

namespace neon {
namespace feature_flags {

// Default values (can be changed at runtime)
bool ENABLE_NPCS = true;
bool ENABLE_WEATHER = true;
bool ENABLE_ECONOMY = true;
bool ENABLE_PATHOGENS = true;
bool ENABLE_RELATIONSHIPS = true;
bool ENABLE_CRISIS = true;
bool ENABLE_DEBUG_HUD = true;

void load_from_config(const char* /*path*/) {
    // TODO: Implement config file loading (e.g., from JSON or INI)
}

void print_flags() {
    std::cout << "Feature Flags:\n";
    std::cout << "  NPCs: " << (ENABLE_NPCS ? "ON" : "OFF") << "\n";
    std::cout << "  Weather: " << (ENABLE_WEATHER ? "ON" : "OFF") << "\n";
    std::cout << "  Economy: " << (ENABLE_ECONOMY ? "ON" : "OFF") << "\n";
    std::cout << "  Pathogens: " << (ENABLE_PATHOGENS ? "ON" : "OFF") << "\n";
    std::cout << "  Relationships: " << (ENABLE_RELATIONSHIPS ? "ON" : "OFF") << "\n";
    std::cout << "  Crisis: " << (ENABLE_CRISIS ? "ON" : "OFF") << "\n";
    std::cout << "  Debug HUD: " << (ENABLE_DEBUG_HUD ? "ON" : "OFF") << "\n";
}

}} // namespace neon::feature_flags
