#include <cassert>
#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "world_builder.h"

static void testBuildWorldCreatesOnlyHousingBaseline() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    auto macros = buildWorld(registry, config);

    assert(macros.size() == 1);
    assert(validateWorld(registry, config));
    assert(registry.entity_count() == 3);

    auto macro_view = registry.view<MacroZoneComponent>();
    auto micro_view = registry.view<MicroZoneComponent>();
    auto buildings = registry.view<BuildingComponent, TransformComponent, BuildingUseComponent>();

    assert(macro_view.size() == 1);
    assert(micro_view.size() == 1);
    assert(buildings.size() == 1);

    const Entity macro = macro_view.front();
    const Entity micro = micro_view.front();
    const Entity building = buildings.front();

    assert(registry.get<MacroZoneComponent>(macro).type == ZoneType::SLUM);
    assert(registry.get<MicroZoneComponent>(micro).parent_macro == macro);
    assert(registry.get<MicroZoneComponent>(micro).role == MicroZoneRole::HOUSING);
    assert(registry.get<BuildingUseComponent>(building).role == MicroZoneRole::HOUSING);
    assert(registry.has<SolidComponent>(building));
    assert(registry.has<GlyphComponent>(building));
}

static void testConfiguredHousingCountCreatesNonOverlappingBuildings() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.housing_building_count = 3;

    auto macros = buildWorld(registry, config);

    assert(macros.size() == 1);
    assert(validateWorld(registry, config));
    assert(registry.view<MacroZoneComponent>().size() == 1);
    assert(registry.view<MicroZoneComponent>().size() == 1);

    auto buildings = registry.view<BuildingComponent, TransformComponent, BuildingUseComponent>();
    assert(buildings.size() == 3);
    assert(registry.entity_count() == 5);
    assert(buildingsDoNotOverlap(registry));

    for (Entity building : buildings) {
        assert(registry.get<BuildingUseComponent>(building).role == MicroZoneRole::HOUSING);
        assert(registry.has<SolidComponent>(building));
        assert(registry.has<GlyphComponent>(building));
    }
}

static void testConfiguredWorkplaceCreatesSecondBuildingType() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;

    buildWorld(registry, config);

    assert(validateWorld(registry, config));
    assert(registry.view<MacroZoneComponent>().size() == 1);
    assert(registry.view<MicroZoneComponent>().size() == 2);

    size_t housing_count = 0;
    size_t workplace_count = 0;
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    assert(buildings.size() == 2);
    for (Entity building : buildings) {
        const auto role = registry.get<BuildingUseComponent>(building).role;
        if (role == MicroZoneRole::HOUSING) ++housing_count;
        if (role == MicroZoneRole::WORKPLACE) ++workplace_count;
    }
    assert(housing_count == 1);
    assert(workplace_count == 1);
    assert(buildingsDoNotOverlap(registry));
}

static void testPedestrianPathRequiresHousingAndWorkplace() {
    Registry registry_without_workplace;
    WorldConfig housing_only = makeSandboxConfig();
    buildWorld(registry_without_workplace, housing_only);

    assert(deriveInfrastructure(registry_without_workplace, housing_only) == 0);
    assert(registry_without_workplace.view<PathComponent>().empty());

    Registry registry_with_workplace;
    WorldConfig connected = makeSandboxConfig();
    connected.workplace_micro_zone_count = 1;
    connected.workplace_building_count = 1;
    buildWorld(registry_with_workplace, connected);

    assert(deriveInfrastructure(registry_with_workplace, connected) == 1);
    auto paths = registry_with_workplace.view<PathComponent, TransformComponent, GlyphComponent>();
    assert(paths.size() == 1);
    const auto& path = registry_with_workplace.get<PathComponent>(paths.front());
    assert(path.kind == PathKind::PEDESTRIAN);
    assert(registry_with_workplace.alive(path.from));
    assert(registry_with_workplace.alive(path.to));
    assert(registry_with_workplace.get<BuildingUseComponent>(path.from).role == MicroZoneRole::HOUSING);
    assert(registry_with_workplace.get<BuildingUseComponent>(path.to).role == MicroZoneRole::WORKPLACE);
    assert(!registry_with_workplace.has<SolidComponent>(paths.front()));
    assert(deriveInfrastructure(registry_with_workplace, connected) == 0);
    assert(registry_with_workplace.view<PathComponent>().size() == 1);
}

static void testValidationRejectsBuildingOutsideMicroZone() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    auto& transform = registry.get<TransformComponent>(building);
    transform.x = 1000.0f;
    transform.y = 1000.0f;

    assert(!validateWorld(registry, config));
}

static void testValidationRejectsOverlappingBuildings() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.housing_building_count = 2;
    buildWorld(registry, config);

    auto buildings = registry.view<BuildingComponent, TransformComponent>();
    assert(buildings.size() == 2);

    registry.get<TransformComponent>(buildings[1]) = registry.get<TransformComponent>(buildings[0]);

    assert(!buildingsDoNotOverlap(registry));
    assert(!validateWorld(registry, config));
}

static void testPlayerSpawnValidationRejectsSolids() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    const auto& building_transform = registry.get<TransformComponent>(building);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player,
        building_transform.x, building_transform.y, 12.0f, 12.0f);

    assert(!validatePlayerSpawn(registry, player));

    registry.get<TransformComponent>(player) = TransformComponent{0.0f, 115.0f, 12.0f, 12.0f};
    assert(validatePlayerSpawn(registry, player));

    Registry connected_registry;
    WorldConfig connected = makeSandboxConfig();
    connected.workplace_micro_zone_count = 1;
    connected.workplace_building_count = 1;
    buildWorld(connected_registry, connected);

    Entity connected_player = connected_registry.create();
    connected_registry.assign<PlayerComponent>(connected_player);
    connected_registry.assign<TransformComponent>(connected_player,
        0.0f, -115.0f, 12.0f, 12.0f);

    assert(validatePlayerSpawn(connected_registry, connected_player));
}

static void testBuildingInteractionRangeHelpers() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);

    TransformComponent near_housing{0.0f, -115.0f, 12.0f, 12.0f};
    TransformComponent near_workplace{0.0f, 115.0f, 12.0f, 12.0f};
    TransformComponent far_from_buildings{160.0f, 0.0f, 12.0f, 12.0f};

    assert(playerCanInteractWithHousing(registry, near_housing, 18.0f));
    assert(!playerCanInteractWithHousing(registry, far_from_buildings, 18.0f));
    assert(playerCanInteractWithWorkplace(registry, near_workplace, 18.0f));
    assert(!playerCanInteractWithWorkplace(registry, far_from_buildings, 18.0f));

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, near_housing);
    registry.assign<BuildingInteractionComponent>(player);

    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::NEAR_HOUSING);
    registry.get<BuildingInteractionComponent>(player).inside_building = true;
    registry.get<BuildingInteractionComponent>(player).building_role = MicroZoneRole::HOUSING;
    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::INSIDE_HOUSING);
    registry.get<BuildingInteractionComponent>(player).inside_building = false;
    registry.get<TransformComponent>(player) = near_workplace;
    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::NEAR_WORKPLACE);
    registry.get<BuildingInteractionComponent>(player).inside_building = true;
    registry.get<BuildingInteractionComponent>(player).building_role = MicroZoneRole::WORKPLACE;
    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::INSIDE_WORKPLACE);
    registry.get<BuildingInteractionComponent>(player).inside_building = false;
    registry.get<TransformComponent>(player) = far_from_buildings;
    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::OUTSIDE);
}

static void testInspectionTargetHelpers() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    TransformComponent near_housing{0.0f, -115.0f, 12.0f, 12.0f};
    TransformComponent near_workplace{0.0f, 115.0f, 12.0f, 12.0f};
    TransformComponent near_path{0.0f, 0.0f, 12.0f, 12.0f};
    TransformComponent far_from_targets{180.0f, 0.0f, 12.0f, 12.0f};

    assert(playerCanInspect(registry, near_housing, 22.0f));
    assert(nearestInspectionTargetInRange(registry, near_housing, 22.0f).type == InspectionTargetType::HOUSING);
    assert(playerCanInspect(registry, near_workplace, 22.0f));
    assert(nearestInspectionTargetInRange(registry, near_workplace, 22.0f).type == InspectionTargetType::WORKPLACE);
    assert(playerCanInspect(registry, near_path, 22.0f));
    assert(nearestInspectionTargetInRange(registry, near_path, 22.0f).type == InspectionTargetType::PEDESTRIAN_PATH);
    assert(!playerCanInspect(registry, far_from_targets, 22.0f));
    assert(nearestInspectionTargetInRange(registry, far_from_targets, 22.0f).type == InspectionTargetType::NONE);
}

static void testWorkerInspectionRangeAndPriority() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = registry.view<FixedActorComponent, TransformComponent>().front();
    const auto& worker_transform = registry.get<TransformComponent>(worker);
    const auto target = nearestInspectionTargetInRange(registry, worker_transform, 22.0f);
    assert(target.entity == worker);
    assert(target.type == InspectionTargetType::WORKER);
    assert(playerCanInspect(registry, worker_transform, 22.0f));

    TransformComponent far_from_worker = worker_transform;
    far_from_worker.x += 80.0f;
    assert(nearestInspectionTargetInRange(registry, far_from_worker, 22.0f).type != InspectionTargetType::WORKER);
}

static void testFixedWorkerCountIsConfigDriven() {
    Registry none_registry;
    WorldConfig none_config = makeSandboxConfig();
    none_config.workplace_micro_zone_count = 1;
    none_config.workplace_building_count = 1;
    none_config.fixed_worker_count = 0;
    buildWorld(none_registry, none_config);
    deriveInfrastructure(none_registry, none_config);

    assert(spawnFixedActors(none_registry, none_config) == 0);
    assert(none_registry.view<FixedActorComponent>().empty());

    Registry one_registry;
    WorldConfig one_config = makeSandboxConfig();
    one_config.workplace_micro_zone_count = 1;
    one_config.workplace_building_count = 1;
    one_config.fixed_worker_count = 1;
    buildWorld(one_registry, one_config);
    deriveInfrastructure(one_registry, one_config);

    assert(spawnFixedActors(one_registry, one_config) == 1);
    auto actors = one_registry.view<FixedActorComponent, TransformComponent, GlyphComponent>();
    assert(actors.size() == 1);
    const auto& actor = one_registry.get<FixedActorComponent>(actors.front());
    assert(actor.kind == FixedActorKind::WORKER);
    assert(one_registry.alive(actor.path_entity));
    assert(one_registry.has<PathComponent>(actor.path_entity));
    assert(spawnFixedActors(one_registry, one_config) == 0);
    assert(one_registry.view<FixedActorComponent>().size() == 1);
}

static void testFixedWorkerMovesOnAssignedPath() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity actor = registry.view<FixedActorComponent, TransformComponent>().front();
    const auto before = registry.get<TransformComponent>(actor);
    const Entity path_entity = registry.get<FixedActorComponent>(actor).path_entity;
    const auto& path = registry.get<TransformComponent>(path_entity);

    updateFixedActors(registry, 1.0f);

    const auto& after = registry.get<TransformComponent>(actor);
    assert(after.y != before.y || after.x != before.x);
    assert(after.x >= path.x - path.width * 0.5f - 0.01f);
    assert(after.x <= path.x + path.width * 0.5f + 0.01f);
    assert(after.y >= path.y - path.height * 0.5f - 0.01f);
    assert(after.y <= path.y + path.height * 0.5f + 0.01f);
}

static void testWorkerAcknowledgementToggle() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = registry.view<FixedActorComponent, TransformComponent>().front();
    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(worker));

    Entity found_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), 18.0f);
    assert(found_worker == worker);
    assert(!registry.get<FixedActorComponent>(worker).acknowledged);

    registry.get<FixedActorComponent>(worker).acknowledged = true;
    assert(registry.get<FixedActorComponent>(worker).acknowledged);

    TransformComponent far_transform = registry.get<TransformComponent>(worker);
    far_transform.x += 100.0f;
    registry.get<TransformComponent>(player) = far_transform;
    Entity far_worker = nearestWorkerInRange(registry, registry.get<TransformComponent>(player), 18.0f);
    assert(far_worker == MAX_ENTITIES);
}

static void testInsideHousingInspectionTarget() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    const auto& building_transform = registry.get<TransformComponent>(building);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, building_transform);
    auto& interaction = registry.assign<BuildingInteractionComponent>(player);
    interaction.building_entity = building;
    interaction.building_role = MicroZoneRole::HOUSING;
    interaction.inside_building = true;

    InspectionTarget target = playerInspectionTarget(registry, player, 22.0f);
    assert(target.entity == building);
    assert(target.type == InspectionTargetType::HOUSING_INTERIOR);
}

static void testInsideWorkplaceInspectionTarget() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);

    Entity workplace_building = MAX_ENTITIES;
    auto buildings = registry.view<BuildingComponent, TransformComponent, BuildingUseComponent>();
    for (Entity b : buildings) {
        if (registry.get<BuildingUseComponent>(b).role == MicroZoneRole::WORKPLACE) {
            workplace_building = b;
            break;
        }
    }
    assert(workplace_building != MAX_ENTITIES);

    const auto& building_transform = registry.get<TransformComponent>(workplace_building);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, building_transform);
    auto& interaction = registry.assign<BuildingInteractionComponent>(player);
    interaction.building_entity = workplace_building;
    interaction.building_role = MicroZoneRole::WORKPLACE;
    interaction.inside_building = true;

    InspectionTarget target = playerInspectionTarget(registry, player, 22.0f);
    assert(target.entity == workplace_building);
    assert(target.type == InspectionTargetType::WORKPLACE_INTERIOR);
}

int main() {
    testBuildWorldCreatesOnlyHousingBaseline();
    testConfiguredHousingCountCreatesNonOverlappingBuildings();
    testConfiguredWorkplaceCreatesSecondBuildingType();
    testPedestrianPathRequiresHousingAndWorkplace();
    testValidationRejectsBuildingOutsideMicroZone();
    testValidationRejectsOverlappingBuildings();
    testPlayerSpawnValidationRejectsSolids();
    testBuildingInteractionRangeHelpers();
    testInspectionTargetHelpers();
    testInsideHousingInspectionTarget();
    testInsideWorkplaceInspectionTarget();
    testWorkerInspectionRangeAndPriority();
    testFixedWorkerCountIsConfigDriven();
    testFixedWorkerMovesOnAssignedPath();
    testWorkerAcknowledgementToggle();
    return 0;
}
