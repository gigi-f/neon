#include <cassert>
#include <string>
#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "interior.h"
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

static void testConfiguredSupplyCreatesThirdPurposeBuilding() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;

    buildWorld(registry, config);

    assert(validateWorld(registry, config));
    assert(registry.view<MacroZoneComponent>().size() == 1);
    assert(registry.view<MicroZoneComponent>().size() == 2);

    size_t housing_count = 0;
    size_t supply_count = 0;
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    assert(buildings.size() == 2);
    for (Entity building : buildings) {
        const auto role = registry.get<BuildingUseComponent>(building).role;
        if (role == MicroZoneRole::HOUSING) ++housing_count;
        if (role == MicroZoneRole::SUPPLY) {
            ++supply_count;
            assert(registry.get<BuildingComponent>(building).is_enterable);
            assert(registry.has<GlyphComponent>(building));
            assert(registry.get<GlyphComponent>(building).chars == "s");
        }
    }
    assert(housing_count == 1);
    assert(supply_count == 1);
    assert(buildingsDoNotOverlap(registry));
}

static void testThreeRoleLayoutKeepsUsableBuildingFootprints() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;

    buildWorld(registry, config);

    assert(validateWorld(registry, config));
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    assert(buildings.size() == 3);
    for (Entity building : buildings) {
        const auto& transform = registry.get<TransformComponent>(building);
        assert(transform.width >= 48.0f);
        assert(transform.height >= 48.0f);
    }
}

static void testPedestrianPathRequiresHousingAndWorkplace() {
    Registry registry_without_workplace;
    WorldConfig housing_only = makeSandboxConfig();
    buildWorld(registry_without_workplace, housing_only);

    assert(deriveInfrastructure(registry_without_workplace, housing_only) == 0);
    assert(registry_without_workplace.view<PathComponent>().empty());
    assert(registry_without_workplace.view<PathStateComponent>().empty());

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
    assert(registry_with_workplace.has<PathStateComponent>(paths.front()));
    assert(registry_with_workplace.get<PathStateComponent>(paths.front()).state == PathState::LIT);
    assert(pathStateName(registry_with_workplace.get<PathStateComponent>(paths.front()).state) == std::string("LIT"));
    assert(pathStateInspectionDetail(registry_with_workplace.get<PathStateComponent>(paths.front()).state) == std::string("Foot path. LIT: low amber markers make the route easier to follow."));
    assert(registry_with_workplace.alive(path.from));
    assert(registry_with_workplace.alive(path.to));
    assert(registry_with_workplace.get<BuildingUseComponent>(path.from).role == MicroZoneRole::HOUSING);
    assert(registry_with_workplace.get<BuildingUseComponent>(path.to).role == MicroZoneRole::WORKPLACE);
    assert(!registry_with_workplace.has<SolidComponent>(paths.front()));
    assert(deriveInfrastructure(registry_with_workplace, connected) == 0);
    assert(registry_with_workplace.view<PathComponent>().size() == 1);
    assert(registry_with_workplace.view<PathStateComponent>().size() == 1);
}

static void testSupplyPathRequiresConfiguredSupply() {
    Registry without_supply;
    WorldConfig housing_only = makeSandboxConfig();
    buildWorld(without_supply, housing_only);

    assert(derivePedestrianPaths(without_supply, kWorkplaceToSupplyPedestrianAccess) == 0);
    assert(without_supply.view<PathComponent>().empty());

    Registry with_supply;
    WorldConfig supply_config = makeSandboxConfig();
    supply_config.workplace_micro_zone_count = 1;
    supply_config.workplace_building_count = 1;
    supply_config.supply_micro_zone_count = 1;
    supply_config.supply_building_count = 1;
    buildWorld(with_supply, supply_config);

    assert(deriveInfrastructure(with_supply, supply_config) == 2);
    auto paths = with_supply.view<PathComponent, TransformComponent, GlyphComponent>();
    assert(paths.size() == 2);
    bool found_workplace_supply = false;
    for (Entity path_entity : paths) {
        const auto& path = with_supply.get<PathComponent>(path_entity);
        assert(path.kind == PathKind::PEDESTRIAN);
        assert(with_supply.has<PathStateComponent>(path_entity));
        assert(!transformOverlapsSolid(with_supply, with_supply.get<TransformComponent>(path_entity), path_entity));
        const auto from_role = with_supply.get<BuildingUseComponent>(path.from).role;
        const auto to_role = with_supply.get<BuildingUseComponent>(path.to).role;
        if (from_role == MicroZoneRole::WORKPLACE && to_role == MicroZoneRole::SUPPLY) {
            found_workplace_supply = true;
        }
    }
    assert(found_workplace_supply);
    assert(deriveInfrastructure(with_supply, supply_config) == 0);
    assert(with_supply.view<PathComponent>().size() == 2);
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

    Registry supply_registry;
    WorldConfig supply_config = makeSandboxConfig();
    supply_config.supply_micro_zone_count = 1;
    supply_config.supply_building_count = 1;
    buildWorld(supply_registry, supply_config);

    Entity supply = firstBuildingByRole(supply_registry, MicroZoneRole::SUPPLY);
    assert(supply != MAX_ENTITIES);
    Entity supply_player = supply_registry.create();
    supply_registry.assign<PlayerComponent>(supply_player);
    supply_registry.assign<TransformComponent>(supply_player, supply_registry.get<TransformComponent>(supply));
    supply_registry.assign<BuildingInteractionComponent>(supply_player);
    assert(playerLocationState(supply_registry, supply_player, 18.0f) == PlayerLocationState::NEAR_SUPPLY);
    assert(enterBuildingInterior(supply_registry, supply_player, supply));
    assert(playerLocationState(supply_registry, supply_player, 18.0f) == PlayerLocationState::INSIDE_SUPPLY);
    assert(playerInspectionTarget(supply_registry, supply_player, 22.0f).type == InspectionTargetType::SUPPLY_INTERIOR);
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

    Registry supply_registry;
    WorldConfig supply_config = makeSandboxConfig();
    supply_config.supply_micro_zone_count = 1;
    supply_config.supply_building_count = 1;
    buildWorld(supply_registry, supply_config);

    Entity supply = firstBuildingByRole(supply_registry, MicroZoneRole::SUPPLY);
    assert(supply != MAX_ENTITIES);
    const auto& supply_transform = supply_registry.get<TransformComponent>(supply);
    assert(playerCanInspect(supply_registry, supply_transform, 22.0f));
    assert(nearestInspectionTargetInRange(supply_registry, supply_transform, 22.0f).type == InspectionTargetType::SUPPLY);
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
    assert(!transformOverlapsSolid(registry, after, actor));
}

static void testFixedWorkerTransitionsAcrossConnectedPaths() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity actor = registry.view<FixedActorComponent, TransformComponent>().front();
    const Entity first_path = registry.get<FixedActorComponent>(actor).path_entity;

    updateFixedActors(registry, 10.0f);

    const auto& actor_component = registry.get<FixedActorComponent>(actor);
    const auto& actor_transform = registry.get<TransformComponent>(actor);
    assert(actor_component.path_entity != first_path);
    assert(registry.alive(actor_component.path_entity));
    const auto& path = registry.get<PathComponent>(actor_component.path_entity);
    assert(registry.get<BuildingUseComponent>(path.from).role == MicroZoneRole::WORKPLACE);
    assert(registry.get<BuildingUseComponent>(path.to).role == MicroZoneRole::SUPPLY);
    assert(!transformOverlapsSolid(registry, actor_transform, actor));
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

static void testInteriorLayoutsAreRoleSpecific() {
    const InteriorLayout housing = interiorLayoutForRole(MicroZoneRole::HOUSING);
    const InteriorLayout workplace = interiorLayoutForRole(MicroZoneRole::WORKPLACE);

    assert(housing.role == MicroZoneRole::HOUSING);
    assert(workplace.role == MicroZoneRole::WORKPLACE);
    assert(housing.width != workplace.width);
    assert(housing.height != workplace.height);
    assert(interiorPositionWithinLayout(housing, housing.spawn));
    assert(interiorPositionWithinLayout(workplace, workplace.spawn));
}

static void testEnterBuildingInitializesInteriorState() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    TransformComponent exterior{0.0f, -115.0f, 12.0f, 12.0f};

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, exterior);
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, building));

    const auto& interaction = registry.get<BuildingInteractionComponent>(player);
    const InteriorLayout layout = interiorLayoutForBuilding(registry, building);
    assert(interaction.inside_building);
    assert(interaction.building_entity == building);
    assert(interaction.building_role == MicroZoneRole::HOUSING);
    assert(interaction.exterior_position.x == exterior.x);
    assert(interaction.exterior_position.y == exterior.y);
    assert(interaction.interior_position.x == layout.spawn.x);
    assert(interaction.interior_position.y == layout.spawn.y);
    assert(interiorPositionWithinLayout(layout, interaction.interior_position));
}

static void testInteriorMovementChangesLocalPositionAndClampsToRoom() {
    const InteriorLayout layout = interiorLayoutForRole(MicroZoneRole::HOUSING);
    TransformComponent current = layout.spawn;

    TransformComponent moved = movedInteriorPosition(layout, current, 1.0f, 0.0f, 90.0f, 0.25f);
    assert(moved.x > current.x);
    assert(moved.y == current.y);
    assert(interiorPositionWithinLayout(layout, moved));

    TransformComponent far_edge = movedInteriorPosition(layout, current, 100.0f, 0.0f, 90.0f, 10.0f);
    assert(interiorPositionWithinLayout(layout, far_edge));
    assert(far_edge.x + far_edge.width * 0.5f <= layout.width * 0.5f + 0.001f);
}

static void testExitInteriorRestoresExteriorModeOutsideSolid() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    const auto& building_transform = registry.get<TransformComponent>(building);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, building_transform);
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, building));
    assert(exitBuildingInterior(registry, player));
    assert(!registry.get<BuildingInteractionComponent>(player).inside_building);
    assert(!transformOverlapsSolid(registry, registry.get<TransformComponent>(player), player));
}

static void testInsideInspectionIgnoresExteriorCarryables() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(object));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, building));
    const InspectionTarget target = playerInspectionTarget(registry, player, 22.0f);
    assert(target.entity == building);
    assert(target.type == InspectionTargetType::HOUSING_INTERIOR);
}

static void testCarryableObjectInteractions() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    const auto& object_transform = registry.get<TransformComponent>(object);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, object_transform);
    registry.assign<BuildingInteractionComponent>(player);

    Entity found_object = nearestCarryableObjectInRange(registry, registry.get<TransformComponent>(player), 18.0f);
    assert(found_object == object);

    auto& player_comp = registry.get<PlayerComponent>(player);

    // Pick up
    player_comp.carried_object = found_object;
    registry.get<TransformComponent>(found_object).x = 99999.0f; // hidden
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    // Enter housing
    Entity building = registry.view<BuildingComponent, TransformComponent>().front();
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(building);

    auto& interaction = registry.get<BuildingInteractionComponent>(player);
    interaction.building_entity = building;
    interaction.building_role = registry.get<BuildingUseComponent>(building).role;
    interaction.inside_building = true;

    assert(registry.get<BuildingInteractionComponent>(player).inside_building);
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    // Drop while inside (should fail, meaning carried object remains)
    bool inside = registry.get<BuildingInteractionComponent>(player).inside_building;
    if (!inside && !transformOverlapsSolid(registry, registry.get<TransformComponent>(player))) {
        registry.get<TransformComponent>(player_comp.carried_object) = registry.get<TransformComponent>(player);
        player_comp.carried_object = MAX_ENTITIES;
    }
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    // Exit housing
    interaction.inside_building = false;
    assert(!registry.get<BuildingInteractionComponent>(player).inside_building);

    // Drop outside
    TransformComponent new_pos = registry.get<TransformComponent>(player);
    new_pos.x = -1000.0f; // move to safe clear area
    new_pos.y = -1000.0f;
    registry.get<TransformComponent>(player) = new_pos;

    inside = registry.get<BuildingInteractionComponent>(player).inside_building;
    if (!inside && !transformOverlapsSolid(registry, registry.get<TransformComponent>(player))) {
        registry.get<TransformComponent>(player_comp.carried_object) = registry.get<TransformComponent>(player);
        player_comp.carried_object = MAX_ENTITIES;
    }

    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(registry.get<TransformComponent>(object).x == new_pos.x);
}
int main() {
    testBuildWorldCreatesOnlyHousingBaseline();
    testConfiguredHousingCountCreatesNonOverlappingBuildings();
    testConfiguredWorkplaceCreatesSecondBuildingType();
    testConfiguredSupplyCreatesThirdPurposeBuilding();
    testThreeRoleLayoutKeepsUsableBuildingFootprints();
    testPedestrianPathRequiresHousingAndWorkplace();
    testSupplyPathRequiresConfiguredSupply();
    testValidationRejectsBuildingOutsideMicroZone();
    testValidationRejectsOverlappingBuildings();
    testPlayerSpawnValidationRejectsSolids();
    testBuildingInteractionRangeHelpers();
    testInspectionTargetHelpers();
    testInsideHousingInspectionTarget();
    testInsideWorkplaceInspectionTarget();
    testInteriorLayoutsAreRoleSpecific();
    testEnterBuildingInitializesInteriorState();
    testInteriorMovementChangesLocalPositionAndClampsToRoom();
    testExitInteriorRestoresExteriorModeOutsideSolid();
    testInsideInspectionIgnoresExteriorCarryables();
    testWorkerInspectionRangeAndPriority();
    testFixedWorkerCountIsConfigDriven();
    testFixedWorkerMovesOnAssignedPath();
    testFixedWorkerTransitionsAcrossConnectedPaths();
    testWorkerAcknowledgementToggle();
    testCarryableObjectInteractions();
    return 0;
}
