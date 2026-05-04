#include <cassert>
#include "world_generation.h"
#include "world_builder.h"

static void testGeometryHelpers() {
    TransformComponent a{0.0f, 0.0f, 10.0f, 10.0f};
    TransformComponent b{4.0f, 0.0f, 10.0f, 10.0f};
    TransformComponent c{20.0f, 0.0f, 4.0f, 4.0f};

    assert(aabbOverlap(aabbFromTransform(a), aabbFromTransform(b)));
    assert(!aabbOverlap(aabbFromTransform(a), aabbFromTransform(c)));
    assert(deterministicStep(0xC0FFEEu) == deterministicStep(0xC0FFEEu));
}

static void testMinimalWorldValidation() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    assert(validateWorld(registry, config));
    assert(registry.view<MacroZoneComponent>().size() == 1);
    assert(registry.view<MicroZoneComponent>().size() == 1);
    assert(registry.view<BuildingComponent>().size() == 1);
    assert(registry.entity_count() == 4);
    assert(registry.view<ShelterListingComponent>().size() == 1);
}

int main() {
    testGeometryHelpers();
    testMinimalWorldValidation();
    return 0;
}
