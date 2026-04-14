// feature_flags.h
// Neon Oubliette Feature Flags
// Toggle features at compile-time or runtime for debugging and development.
#pragma once

namespace neon {
namespace feature_flags {

// Core toggles (set to true to enable, false to disable)
extern bool ENABLE_NPCS;
extern bool ENABLE_WEATHER;
extern bool ENABLE_ECONOMY;
extern bool ENABLE_PATHOGENS;
extern bool ENABLE_RELATIONSHIPS;
extern bool ENABLE_CRISIS;
extern bool ENABLE_DEBUG_HUD;

// Utility
void load_from_config(const char* path); // Optionally load flags from a config file
void print_flags(); // Print current flag states for debugging

}} // namespace neon::feature_flags
