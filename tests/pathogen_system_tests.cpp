#include <cassert>
#include <cmath>
#include "ecs.h"
#include "components.h"
#include "simulation_systems.h"

static Entity makeBiologicalEntity(Registry& registry, float x, float y) {
    Entity e = registry.create();
    registry.assign<TransformComponent>(e, x, y, 8.0f, 8.0f);
    registry.assign<BiologyComponent>(e);
    return e;
}

static void testOutdoorExposureIsDeterministic() {
    Registry registry;
    PathogenSystem system;

    Entity source = makeBiologicalEntity(registry, 0.0f, 0.0f);
    Entity target = makeBiologicalEntity(registry, 12.0f, 0.0f);
    Entity distant = makeBiologicalEntity(registry, 80.0f, 0.0f);

    PathogenComponent pathogen;
    pathogen.infection_load = 80.0f;
    pathogen.infectiousness = 50.0f;
    pathogen.immune_response = 0.0f;
    registry.assign<PathogenComponent>(source, pathogen);

    system.update(registry, 1.0f, 1.0f);

    assert(registry.has<PathogenComponent>(target));
    assert(!registry.has<PathogenComponent>(distant));
    assert(std::fabs(registry.get<PathogenComponent>(target).infection_load - 49.92f) < 0.001f);
}

static void testProgressionPressuresBiology() {
    Registry registry;
    PathogenSystem system;

    Entity infected = makeBiologicalEntity(registry, 0.0f, 0.0f);
    PathogenComponent pathogen;
    pathogen.infection_load = 80.0f;
    pathogen.severity = 0.5f;
    pathogen.incubation_timer = 0.0f;
    pathogen.immune_response = 0.0f;
    registry.assign<PathogenComponent>(infected, pathogen);

    auto before = registry.get<BiologyComponent>(infected);
    system.update(registry, 1.0f, 1.0f);
    auto& after = registry.get<BiologyComponent>(infected);

    assert(registry.get<PathogenComponent>(infected).severity > 0.5f);
    assert(after.health < before.health);
    assert(after.organs.lungs < before.organs.lungs);
    assert(after.vitals.oxygen_sat < before.vitals.oxygen_sat);
    assert(after.vitals.heart_rate > before.vitals.heart_rate);
}

static void testSharedInteriorExposureIgnoresDistance() {
    Registry registry;
    PathogenSystem system;

    Entity buildingA = registry.create();
    Entity buildingB = registry.create();
    Entity source = makeBiologicalEntity(registry, 0.0f, 0.0f);
    Entity sharedTarget = makeBiologicalEntity(registry, 500.0f, 0.0f);
    Entity otherBuildingTarget = makeBiologicalEntity(registry, 500.0f, 40.0f);

    registry.assign<InteriorComponent>(source, buildingA);
    registry.assign<InteriorComponent>(sharedTarget, buildingA);
    registry.assign<InteriorComponent>(otherBuildingTarget, buildingB);

    PathogenComponent pathogen;
    pathogen.infection_load = 80.0f;
    pathogen.infectiousness = 50.0f;
    pathogen.immune_response = 0.0f;
    registry.assign<PathogenComponent>(source, pathogen);

    system.update(registry, 1.0f, 1.0f);

    assert(registry.has<PathogenComponent>(sharedTarget));
    assert(!registry.has<PathogenComponent>(otherBuildingTarget));
    assert(std::fabs(registry.get<PathogenComponent>(sharedTarget).infection_load - 17.42f) < 0.001f);
}

static void testL2MoodAndTraumaCascade() {
    Registry registry;
    PathogenSystem system;

    Entity infected = makeBiologicalEntity(registry, 0.0f, 0.0f);
    registry.assign<CognitiveComponent>(infected);

    PathogenComponent pathogen;
    pathogen.infection_load = 80.0f;
    pathogen.severity = 0.5f;
    pathogen.incubation_timer = 0.0f;
    pathogen.immune_response = 0.0f;
    registry.assign<PathogenComponent>(infected, pathogen);

    system.update(registry, 1.0f, 1.0f, true, 9.5f);

    auto& cognitive = registry.get<CognitiveComponent>(infected);
    auto& updatedPathogen = registry.get<PathogenComponent>(infected);
    assert(cognitive.pleasure < 0.0f);
    assert(cognitive.arousal > 0.0f);
    assert(cognitive.mem_size == 1);
    assert(cognitive.memory[0].event == MemoryEventType::BECAME_ILL);
    assert(std::fabs(cognitive.memory[0].timestamp - 9.5f) < 0.001f);
    assert(updatedPathogen.trauma_recorded);

    system.update(registry, 1.0f, 1.0f, true, 9.6f);
    assert(cognitive.mem_size == 1);
}

static void testInfectedCounterUsesPathogenComponents() {
    Registry registry;
    Entity a = registry.create();
    Entity b = registry.create();
    registry.assign<PathogenComponent>(a);
    registry.assign<PathogenComponent>(b);

    assert(PathogenSystem::infectedCount(registry) == 2);
}

int main() {
    testOutdoorExposureIsDeterministic();
    testProgressionPressuresBiology();
    testSharedInteriorExposureIgnoresDistance();
    testL2MoodAndTraumaCascade();
    testInfectedCounterUsesPathogenComponents();
    return 0;
}
