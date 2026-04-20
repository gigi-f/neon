#include <cassert>
#include <cmath>
#include "ecs.h"
#include "components.h"
#include "simulation_systems.h"

static void testAlertStackBoundsAndExpiry() {
    SimulationAlertStack alerts(2);
    alerts.push(SimulationAlertSeverity::INFO, SimulationAlertCategory::WEATHER, "A", 8.0f, 1.0f);
    alerts.push(SimulationAlertSeverity::WARNING, SimulationAlertCategory::FLOOD, "B", 8.1f, 1.0f);
    alerts.push(SimulationAlertSeverity::DANGER, SimulationAlertCategory::STRUCTURE, "C", 8.2f, 1.0f);

    assert(alerts.size() == 2);
    auto visible = alerts.visible(4);
    assert(visible.size() == 2);
    assert(visible[0].message == "C");
    assert(visible[1].message == "B");

    alerts.update(1.1f);
    assert(alerts.visible(4).empty());
}

static void testWeatherAndFloodHelpersEmitAlerts() {
    SimulationAlertStack alerts(4);

    emitWeatherAlertIfChanged(alerts, WeatherState::CLEAR, WeatherState::ACID_RAIN, 9.0f);
    emitWeatherAlertIfChanged(alerts, WeatherState::ACID_RAIN, WeatherState::ACID_RAIN, 9.1f);
    emitFloodAlert(alerts, true, 9.2f);
    emitFloodAlert(alerts, false, 9.3f);

    auto visible = alerts.visible(4);
    assert(visible.size() == 3);
    assert(visible[0].category == SimulationAlertCategory::FLOOD);
    assert(visible[0].message == "FLOODING CLEARED");
    assert(visible[1].message == "FLOODING ACTIVE");
    assert(visible[2].category == SimulationAlertCategory::WEATHER);
    assert(visible[2].severity == SimulationAlertSeverity::WARNING);
}

static Entity makeInfectedEntity(Registry& registry) {
    Entity e = registry.create();
    registry.assign<TransformComponent>(e, 0.0f, 0.0f, 8.0f, 8.0f);
    registry.assign<BiologyComponent>(e);

    PathogenComponent pathogen;
    pathogen.infection_load = 100.0f;
    pathogen.infectiousness = 0.0f;
    pathogen.severity = 0.34f;
    pathogen.incubation_timer = 0.0f;
    pathogen.immune_response = 0.0f;
    registry.assign<PathogenComponent>(e, pathogen);
    return e;
}

static void testInfectionAlertEmitsOncePerTier() {
    Registry registry;
    PathogenSystem system;
    SimulationAlertStack alerts(4);
    Entity infected = makeInfectedEntity(registry);

    system.update(registry, 1.0f, 1.0f, false, 10.0f, &alerts);
    assert(alerts.size() == 1);
    assert(registry.get<PathogenComponent>(infected).alert_tier == 1);
    assert(alerts.visible(4)[0].category == SimulationAlertCategory::INFECTION);

    system.update(registry, 1.0f, 1.0f, false, 10.1f, &alerts);
    assert(alerts.size() == 1);
}

static void testStructuralCollapseEmitsAlert() {
    Registry registry;
    StructuralDecaySystem system;
    SimulationAlertStack alerts(4);

    Entity world = registry.create();
    registry.assign<WorldConfigComponent>(world);
    auto& tod = registry.assign<TimeOfDayComponent>(world);
    tod.game_hour = 11.5f;
    tod.weather = WeatherState::ACID_RAIN;

    Entity building = registry.create();
    auto& structural = registry.assign<StructuralComponent>(building);
    structural.integrity = 0.0001f;
    structural.material_type = MaterialType::SCRAP;
    structural.is_exposed = true;

    system.update(registry, 1.0f, 1.0f, &alerts);

    assert(registry.has<CollapsedComponent>(building));
    auto visible = alerts.visible(4);
    assert(visible.size() == 1);
    assert(visible[0].category == SimulationAlertCategory::STRUCTURE);
    assert(visible[0].severity == SimulationAlertSeverity::DANGER);
    assert(std::fabs(visible[0].game_hour - 11.5f) < 0.001f);
}

int main() {
    testAlertStackBoundsAndExpiry();
    testWeatherAndFloodHelpersEmitAlerts();
    testInfectionAlertEmitsOncePerTier();
    testStructuralCollapseEmitsAlert();
    return 0;
}
