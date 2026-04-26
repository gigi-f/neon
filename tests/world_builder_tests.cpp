#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include "fixed_actor_system.h"
#include "infrastructure_solver.h"
#include "interior.h"
#include "save_state.h"
#include "world_builder.h"

static bool closeTo(float a, float b) {
    return std::fabs(a - b) < 0.001f;
}

static Entity firstPathBetweenRoles(Registry& registry, MicroZoneRole from_role, MicroZoneRole to_role) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        const auto& path = registry.get<PathComponent>(path_entity);
        if (path.kind != PathKind::PEDESTRIAN ||
            !registry.alive(path.from) ||
            !registry.alive(path.to) ||
            !registry.has<BuildingUseComponent>(path.from) ||
            !registry.has<BuildingUseComponent>(path.to)) {
            continue;
        }

        const auto from = registry.get<BuildingUseComponent>(path.from).role;
        const auto to = registry.get<BuildingUseComponent>(path.to).role;
        if ((from == from_role && to == to_role) ||
            (from == to_role && to == from_role)) {
            return path_entity;
        }
    }
    return MAX_ENTITIES;
}

static Entity routeSignpostTargetEntity(Registry& registry, Entity marker) {
    const auto& signpost = registry.get<RouteSignpostComponent>(marker);
    const auto& path = registry.get<PathComponent>(signpost.path_entity);
    return path.from == signpost.endpoint_entity ? path.to : path.from;
}

static Entity firstRouteSignpostForPath(Registry& registry, Entity path_entity) {
    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        if (registry.get<RouteSignpostComponent>(marker).path_entity == path_entity) {
            return marker;
        }
    }
    return MAX_ENTITIES;
}

static Entity firstRouteSignpostNotForPath(Registry& registry, Entity path_entity) {
    auto signposts = registry.view<RouteSignpostComponent>();
    for (Entity marker : signposts) {
        if (registry.get<RouteSignpostComponent>(marker).path_entity != path_entity) {
            return marker;
        }
    }
    return MAX_ENTITIES;
}

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
    assert(registry_without_workplace.view<RouteSignpostComponent>().empty());

    Registry registry_with_workplace;
    WorldConfig connected = makeSandboxConfig();
    connected.workplace_micro_zone_count = 1;
    connected.workplace_building_count = 1;
    buildWorld(registry_with_workplace, connected);

    assert(deriveInfrastructure(registry_with_workplace, connected) == 1);
    auto paths = registry_with_workplace.view<PathComponent, TransformComponent, GlyphComponent>();
    assert(paths.size() == 1);
    const auto& path = registry_with_workplace.get<PathComponent>(paths.front());
    auto signposts = registry_with_workplace.view<RouteSignpostComponent, TransformComponent, GlyphComponent>();
    assert(signposts.size() == 2);
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
    bool found_to_housing = false;
    bool found_to_workplace = false;
    for (Entity marker : signposts) {
        const auto& signpost = registry_with_workplace.get<RouteSignpostComponent>(marker);
        assert(signpost.path_entity == paths.front());
        assert(registry_with_workplace.alive(signpost.endpoint_entity));
        assert(!registry_with_workplace.has<SolidComponent>(marker));
        assert(!transformOverlapsSolid(registry_with_workplace,
                                       registry_with_workplace.get<TransformComponent>(marker),
                                       marker));
        const Entity target = routeSignpostTargetEntity(registry_with_workplace, marker);
        assert(target != MAX_ENTITIES);
        const char expected_glyph = routeSignpostGlyphForTarget(
            registry_with_workplace.get<TransformComponent>(marker),
            registry_with_workplace.get<TransformComponent>(target));
        assert(registry_with_workplace.get<GlyphComponent>(marker).chars ==
               std::string(1, expected_glyph));
        const std::string readout = routeSignpostReadout(registry_with_workplace, marker);
        if (readout.rfind("TO HOUSING", 0) == 0) found_to_housing = true;
        if (readout.rfind("TO WORKPLACE", 0) == 0) found_to_workplace = true;
        assert(readout.find("SIGNAL: CLEAR") != std::string::npos);
        assert(readout.find("CONSEQUENCE: NONE") != std::string::npos);
    }
    assert(found_to_housing);
    assert(found_to_workplace);
    assert(deriveInfrastructure(registry_with_workplace, connected) == 0);
    assert(registry_with_workplace.view<PathComponent>().size() == 1);
    assert(registry_with_workplace.view<PathStateComponent>().size() == 1);
    assert(registry_with_workplace.view<RouteSignpostComponent>().size() == 2);
}

static void testSupplyPathRequiresConfiguredSupply() {
    Registry without_supply;
    WorldConfig housing_only = makeSandboxConfig();
    buildWorld(without_supply, housing_only);

    assert(derivePedestrianPaths(without_supply, kWorkplaceToSupplyPedestrianAccess) == 0);
    assert(without_supply.view<PathComponent>().empty());
    assert(without_supply.view<RouteSignpostComponent>().empty());

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
    auto signposts = with_supply.view<RouteSignpostComponent, TransformComponent, GlyphComponent>();
    assert(signposts.size() == 4);
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
    bool found_to_housing = false;
    bool found_to_workplace = false;
    bool found_to_supply = false;
    for (Entity marker : signposts) {
        assert(!with_supply.has<SolidComponent>(marker));
        assert(with_supply.alive(with_supply.get<RouteSignpostComponent>(marker).path_entity));
        assert(with_supply.alive(with_supply.get<RouteSignpostComponent>(marker).endpoint_entity));
        assert(!transformOverlapsSolid(with_supply,
                                       with_supply.get<TransformComponent>(marker),
                                       marker));
        const Entity target = routeSignpostTargetEntity(with_supply, marker);
        assert(target != MAX_ENTITIES);
        const char expected_glyph = routeSignpostGlyphForTarget(
            with_supply.get<TransformComponent>(marker),
            with_supply.get<TransformComponent>(target));
        assert(with_supply.get<GlyphComponent>(marker).chars == std::string(1, expected_glyph));
        const std::string readout = routeSignpostReadout(with_supply, marker);
        if (readout.rfind("TO HOUSING", 0) == 0) found_to_housing = true;
        if (readout.rfind("TO WORKPLACE", 0) == 0) found_to_workplace = true;
        if (readout.rfind("TO SUPPLY", 0) == 0) found_to_supply = true;
        assert(readout.find("SIGNAL: CLEAR") != std::string::npos);
        assert(readout.find("CONSEQUENCE: NONE") != std::string::npos);
    }
    assert(found_to_housing);
    assert(found_to_workplace);
    assert(found_to_supply);
    assert(deriveInfrastructure(with_supply, supply_config) == 0);
    assert(with_supply.view<PathComponent>().size() == 2);
    assert(with_supply.view<RouteSignpostComponent>().size() == 4);
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

static void testRouteSignpostInspectionTarget() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(signposts.size() == 2);
    Entity marker = signposts.front();
    const auto& marker_transform = registry.get<TransformComponent>(marker);

    const auto target = nearestInspectionTargetInRange(registry, marker_transform, 22.0f);
    assert(target.entity == marker);
    assert(target.type == InspectionTargetType::ROUTE_SIGNPOST);
    assert(routeSignpostReadout(registry, marker).rfind("TO ", 0) == 0);
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

static void testTinySaveRoundTripRestoresCurrentScopeState() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 12.0f, -96.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(supply != MAX_ENTITIES);
    assert(enterBuildingInterior(registry, player, supply));
    registry.get<BuildingInteractionComponent>(player).interior_position =
        TransformComponent{9.0f, 11.0f, 12.0f, 12.0f};

    Entity object = firstCarryableObject(registry);
    assert(object != MAX_ENTITIES);
    registry.get<PlayerComponent>(player).carried_object = object;
    registry.get<TransformComponent>(object) = TransformComponent{99999.0f, 99999.0f, 8.0f, 8.0f};

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.route_t = 0.6f;
    worker_component.direction = -1.0f;
    worker_component.acknowledged = true;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(worker_component.path_entity),
                        worker_component.route_t);

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    registry.get<TransformComponent>(player) = TransformComponent{200.0f, 200.0f, 12.0f, 12.0f};
    registry.get<BuildingInteractionComponent>(player).inside_building = false;
    registry.get<BuildingInteractionComponent>(player).building_entity = MAX_ENTITIES;
    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = TransformComponent{-120.0f, -120.0f, 8.0f, 8.0f};
    worker_component.route_t = 0.0f;
    worker_component.direction = 1.0f;
    worker_component.acknowledged = false;

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);

    const auto& restored_player = registry.get<TransformComponent>(player);
    assert(closeTo(restored_player.x, 12.0f));
    assert(closeTo(restored_player.y, -96.0f));
    const auto& restored_interaction = registry.get<BuildingInteractionComponent>(player);
    assert(restored_interaction.inside_building);
    assert(restored_interaction.building_entity == supply);
    assert(restored_interaction.building_role == MicroZoneRole::SUPPLY);
    assert(closeTo(restored_interaction.interior_position.x, 9.0f));
    assert(closeTo(restored_interaction.interior_position.y, 11.0f));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(closeTo(registry.get<TransformComponent>(object).x, 99999.0f));

    const auto& restored_worker = registry.get<FixedActorComponent>(worker);
    assert(closeTo(restored_worker.route_t, 0.6f));
    assert(restored_worker.direction < 0.0f);
    assert(restored_worker.acknowledged);
    assert(registry.alive(restored_worker.path_entity));
}

static void testTinySaveFileFailureAndRoundTrip() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 3.0f, 4.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    TinySaveState ignored;
    assert(deserializeTinySaveState("bad save", ignored) == TinySaveStatus::INVALID_FORMAT);
    assert(loadTinyStateFromFile(registry, player, "/tmp/neon_missing_tiny_save_file.txt") ==
           TinySaveStatus::MISSING_FILE);

    const std::string path = "/tmp/neon_tiny_save_round_trip.txt";
    std::remove(path.c_str());
    assert(saveTinyStateToFile(registry, player, path) == TinySaveStatus::OK);

    registry.get<TransformComponent>(player) = TransformComponent{77.0f, 88.0f, 12.0f, 12.0f};
    assert(loadTinyStateFromFile(registry, player, path) == TinySaveStatus::OK);
    assert(closeTo(registry.get<TransformComponent>(player).x, 3.0f));
    assert(closeTo(registry.get<TransformComponent>(player).y, 4.0f));

    std::remove(path.c_str());
}

static void testTinySaveStatusTextSelection() {
    assert(startupSaveStatusLine(false) == "NO SAVE FILE - F5 SAVE");
    assert(startupSaveStatusLine(true) == "SAVE FILE READY - F9 LOAD");
    assert(saveResultStatusLine(TinySaveStatus::OK) == "SAVE OK");
    assert(saveResultStatusLine(TinySaveStatus::INVALID_WORLD) == "SAVE INVALID WORLD");

    assert(loadResultStatusLine(TinySaveStatus::OK,
                                PlayerLocationState::INSIDE_WORKPLACE,
                                "SUPPLY") ==
           "LOAD OK - INSIDE WORKPLACE - CARRYING SUPPLY");
    assert(loadResultStatusLine(TinySaveStatus::OK,
                                PlayerLocationState::NEAR_HOUSING,
                                "") ==
           "LOAD OK - NEAR HOUSING - CARRYING NONE");
    assert(loadResultStatusLine(TinySaveStatus::MISSING_FILE,
                                PlayerLocationState::OUTSIDE,
                                "") ==
           "LOAD MISSING FILE - F5 SAVE FIRST");
    assert(loadResultStatusLine(TinySaveStatus::INVALID_FORMAT,
                                PlayerLocationState::OUTSIDE,
                                "") ==
           "LOAD INVALID FORMAT");

    const std::string path = "/tmp/neon_tiny_save_status_exists.txt";
    std::remove(path.c_str());
    assert(!tinySaveFileExists(path));
    {
        std::ofstream out(path);
        out << "present";
    }
    assert(tinySaveFileExists(path));
    std::remove(path.c_str());
}

static void testCarryableItemKindLabelsAndSaveRoundTrip() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity object = firstCarryableObject(registry);
    assert(object != MAX_ENTITIES);
    assert(registry.get<CarryableComponent>(object).kind == ItemKind::SUPPLY);
    assert(std::string(carryableObjectLabel(registry, object)) == "SUPPLY");
    assert(carryableObjectReadout(registry, object) == "SUPPLY: Carryable object.");
    assert(nearestCarryableObjectInRange(registry,
                                         registry.get<TransformComponent>(object),
                                         18.0f) == object);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(std::string(carryableObjectLabel(
               registry,
               registry.get<PlayerComponent>(player).carried_object)) == "SUPPLY");

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);
    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("NEON_TINY_SAVE_V7") != std::string::npos);
    assert(serialized.find("CARRYABLE 1 SUPPLY") != std::string::npos);

    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);
    assert(parsed.carryable_kind == ItemKind::SUPPLY);

    const std::string invalid_kind_save =
        "NEON_TINY_SAVE_V7\n"
        "PLAYER 0 0 12 12 0\n"
        "BUILDING 0 HOUSING 0 0 0 12 12 0 0 12 12\n"
        "CARRYABLE 1 UNKNOWN 0 0 8 8\n"
        "SHELTER 0\n"
        "WORKPLACE_BENCH EMPTY\n"
        "BUILDING_IMPROVEMENT 0\n"
        "WORKER 0 0 0 0 1 0 0\n"
        "SPOOFED_SIGNPOSTS 0\n"
        "END\n";
    assert(deserializeTinySaveState(invalid_kind_save, parsed) ==
           TinySaveStatus::INVALID_FORMAT);
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

    assert(playerCanTakeNearbyCarryableObject(registry, player, 18.0f));
    assert(takeNearbyCarryableObject(registry, player, 18.0f));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);

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

static void testNearbyCarryablePickupRequiresOutsideAndEmptyHands() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(object));
    registry.assign<BuildingInteractionComponent>(player);

    assert(playerCanTakeNearbyCarryableObject(registry, player, 18.0f));

    registry.get<PlayerComponent>(player).carried_object = object;
    assert(!playerCanTakeNearbyCarryableObject(registry, player, 18.0f));
    assert(!takeNearbyCarryableObject(registry, player, 18.0f));

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(supply);
    assert(enterBuildingInterior(registry, player, supply));
    assert(!playerCanTakeNearbyCarryableObject(registry, player, 18.0f));
    assert(!takeNearbyCarryableObject(registry, player, 18.0f));
}

static void testSupplyInteriorPickupUsesExistingObject() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(playerCanTakeSupplyObject(registry, player));
    assert(takeSupplyObjectFromInterior(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);
    assert(!playerCanTakeSupplyObject(registry, player));
    assert(!takeSupplyObjectFromInterior(registry, player));
}

static void testSupplyInteriorPickupIsSupplyOnly() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(housing != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(housing));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, housing));
    assert(!playerCanTakeSupplyObject(registry, player));
    assert(!takeSupplyObjectFromInterior(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);

    assert(exitBuildingInterior(registry, player));
    assert(enterBuildingInterior(registry, player, supply));
    assert(playerCanTakeSupplyObject(registry, player));
}

static void testSupplyInteriorPickupRequiresEmptyHands() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);

    assert(enterBuildingInterior(registry, player, supply));
    assert(!playerCanTakeSupplyObject(registry, player));
    assert(!takeSupplyObjectFromInterior(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
}

static void testShelterDropOffStoresSupplyOnlyInHousing() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(registry.has<ShelterStockComponent>(housing));
    assert(shelterSupplyReadout(registry) == "SHELTER SUPPLY: 0/1");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(takeSupplyObjectFromInterior(registry, player));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!storeSupplyAtShelter(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(workplace);
    assert(enterBuildingInterior(registry, player, workplace));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!storeSupplyAtShelter(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(playerCanStoreSupplyAtShelter(registry, player));
    assert(storeSupplyAtShelter(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(shelterSupplyCount(registry) == 1);
    assert(shelterSupplyReadout(registry) == "SHELTER SUPPLY: 1/1");
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);
    assert(!playerCanTakeSupplyObject(registry, player));
}

static void testShelterDropOffRequiresCapacityAndCarriedObject() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(housing != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(housing));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, housing));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!storeSupplyAtShelter(registry, player));

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    registry.get<ShelterStockComponent>(housing).current_supply = 1;
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!storeSupplyAtShelter(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(shelterSupplyCount(registry) == 1);
}

static void testTinySaveRoundTripRestoresShelterStock() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(housing != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(takeSupplyObjectFromInterior(registry, player));
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(storeSupplyAtShelter(registry, player));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    registry.get<ShelterStockComponent>(housing).current_supply = 0;
    assert(shelterSupplyReadout(registry) == "SHELTER SUPPLY: 0/1");

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(shelterSupplyCount(registry) == 1);
    assert(shelterSupplyReadout(registry) == "SHELTER SUPPLY: 1/1");
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(!playerCanTakeSupplyObject(registry, player));
}

static void testWorkplaceDeliveryStocksBenchOnlyInWorkplace() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(registry.has<WorkplaceBenchComponent>(workplace));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(takeSupplyObjectFromInterior(registry, player));
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!stockWorkplaceBench(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!stockWorkplaceBench(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(workplace);
    assert(enterBuildingInterior(registry, player, workplace));
    assert(playerCanStockWorkplaceBench(registry, player));
    assert(stockWorkplaceBench(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(workplaceBenchStocked(registry));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: STOCKED");
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);
    assert(!playerCanTakeSupplyObject(registry, player));
}

static void testWorkplaceDeliveryRequiresEmptyBenchAndCarriedObject() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    assert(workplace != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, workplace));
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!stockWorkplaceBench(registry, player));

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!stockWorkplaceBench(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(workplaceBenchStocked(registry));
}

static void testInsideWorkplaceInspectionReflectsBenchState() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, workplace));
    InspectionTarget target = playerInspectionTarget(registry, player, 22.0f);
    assert(target.entity == workplace);
    assert(target.type == InspectionTargetType::WORKPLACE_INTERIOR);
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    target = playerInspectionTarget(registry, player, 22.0f);
    assert(target.entity == workplace);
    assert(target.type == InspectionTargetType::WORKPLACE_INTERIOR);
    assert(workplaceBenchReadout(registry) == "WORK BENCH: STOCKED");
}

static void testTinySaveRoundTripRestoresWorkplaceBench() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(takeSupplyObjectFromInterior(registry, player));
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(workplace);
    assert(enterBuildingInterior(registry, player, workplace));
    assert(stockWorkplaceBench(registry, player));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::EMPTY;
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(workplaceBenchStocked(registry));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: STOCKED");
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(!playerCanTakeSupplyObject(registry, player));
}

static void testPlayerBenchWorkCreatesOutputOnlyWhenStocked() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    assert(!playerCanWorkWorkplaceBench(registry, player));
    assert(!workWorkplaceBench(registry, player));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(playerCanWorkWorkplaceBench(registry, player));
    assert(workWorkplaceBench(registry, player));
    assert(workplaceBenchOutputReady(registry));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: OUTPUT READY");
    assert(!playerCanWorkWorkplaceBench(registry, player));
    assert(!workWorkplaceBench(registry, player));
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 0);

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("WORKPLACE_BENCH OUTPUT_READY") != std::string::npos);

    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::EMPTY;
    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(workplaceBenchReadout(registry) == "WORK BENCH: OUTPUT READY");
}

static void testPlayerTakesFinishedOutputAsSingleCarriedItem() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity object = firstCarryableObject(registry);
    assert(workplace != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(playerCanTakeWorkplaceOutput(registry, player));
    assert(takeWorkplaceOutput(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(registry.get<CarryableComponent>(object).kind == ItemKind::PART);
    assert(std::string(carryableObjectLabel(registry, object)) == "PART");
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(!playerCanTakeWorkplaceOutput(registry, player));
    assert(!takeWorkplaceOutput(registry, player));
    assert(!playerCanStockWorkplaceBench(registry, player));

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
}

static void testPlayerImprovesBuildingWithFinishedItemOnly() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = firstCarryableObject(registry);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(buildingImprovementReadout(registry) == "BUILDING IMPROVED: NO");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);

    assert(enterBuildingInterior(registry, player, supply));
    assert(takeSupplyObjectFromInterior(registry, player));
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(!playerCanImproveBuilding(registry, player));
    assert(!improveBuilding(registry, player));

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(workplace);
    assert(enterBuildingInterior(registry, player, workplace));
    assert(stockWorkplaceBench(registry, player));
    assert(workWorkplaceBench(registry, player));
    assert(takeWorkplaceOutput(registry, player));
    assert(registry.get<CarryableComponent>(object).kind == ItemKind::PART);

    assert(!playerCanImproveBuilding(registry, player));
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    assert(playerCanImproveBuilding(registry, player));
    assert(improveBuilding(registry, player));

    assert(buildingImproved(registry));
    assert(buildingImprovementReadout(registry) == "BUILDING IMPROVED: YES");
    assert(housingInteriorReadout(registry) ==
           "SHELTER SUPPLY: 0/1; BUILDING: IMPROVED; BUILDING IMPROVED: YES");
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(shelterSupplyCount(registry) == 0);
    assert(!playerCanImproveBuilding(registry, player));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("BUILDING_IMPROVEMENT 1") != std::string::npos);
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);
    registry.get<BuildingImprovementComponent>(housing).improved = false;
    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(buildingImproved(registry));
}

static void testWorkerSupplyPickupRequiresSupplyEndpoint() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = 0.5f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(!workerAtSupplyEndpoint(registry, worker));
    assert(!workerCanTakeSupplyObject(registry, worker));
    assert(!takeSupplyObjectForWorker(registry, worker));
    assert(worker_component.carried_object == MAX_ENTITIES);

    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(workerAtSupplyEndpoint(registry, worker));
    assert(workerCanTakeSupplyObject(registry, worker));
    assert(takeSupplyObjectForWorker(registry, worker));

    assert(worker_component.carried_object == object);
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);
    assert(!workerCanTakeSupplyObject(registry, worker));
}

static void testWorkerSupplyPickupBlockedWhilePlayerCarriesObject() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.assign<BuildingInteractionComponent>(player);
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(workerAtSupplyEndpoint(registry, worker));
    assert(!workerCanTakeSupplyObject(registry, worker));
    assert(!takeSupplyObjectForWorker(registry, worker));
    assert(worker_component.carried_object == MAX_ENTITIES);
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: PICKING UP SUPPLY; REASON: WAGE ROUTE; CARRYING: NONE; BLOCKED: WAITING FOR SUPPLY; CONSEQUENCE: SHIFT STALLED");
}

static void testWorkerSupplyPickupBlockedByStoredSupplyStates() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    registry.get<ShelterStockComponent>(housing).current_supply = 1;
    assert(!workerCanTakeSupplyObject(registry, worker));
    registry.get<ShelterStockComponent>(housing).current_supply = 0;

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(!workerCanTakeSupplyObject(registry, worker));
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::EMPTY;

    assert(workerCanTakeSupplyObject(registry, worker));
}

static void testWorkerSupplyDeliveryMovesTowardWorkplaceEndpoint() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 1.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));

    const float start_t = worker_component.route_t;
    const float target_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    assert(!closeTo(start_t, target_t));
    const float start_distance = std::fabs(start_t - target_t);

    assert(updateWorkerSupplyDeliveryRoutes(registry, 1.0f) == 1);
    assert(worker_component.path_entity == workplace_supply_path);
    assert(std::fabs(worker_component.route_t - target_t) < start_distance);
    assert(worker_component.carried_object != MAX_ENTITIES);
    assert(registry.get<GlyphComponent>(worker).chars == "A");
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");
}

static void testWorkerSupplyDeliveryStopsAtWorkplaceEndpoint() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));

    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(worker_component.path_entity == workplace_supply_path);
    assert(workerAtEndpointRole(registry, worker, MicroZoneRole::WORKPLACE));
    assert(closeTo(worker_component.direction, 0.0f));
}

static void testWorkerSupplyDeliveryMissingSupplyPathNoOp() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 0;
    config.supply_building_count = 0;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity housing_workplace_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(housing_workplace_path != MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY) ==
           MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = housing_workplace_path;
    worker_component.route_t = 0.25f;
    worker_component.direction = 1.0f;
    worker_component.carried_object = object;
    hideCarryableObject(registry, object);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(housing_workplace_path),
                        worker_component.route_t);
    const TransformComponent before_transform = registry.get<TransformComponent>(worker);

    assert(updateWorkerSupplyDeliveryRoutes(registry, 10.0f) == 0);
    assert(worker_component.path_entity == housing_workplace_path);
    assert(closeTo(worker_component.route_t, 0.25f));
    assert(closeTo(registry.get<TransformComponent>(worker).x, before_transform.x));
    assert(closeTo(registry.get<TransformComponent>(worker).y, before_transform.y));
    assert(registry.get<GlyphComponent>(worker).chars == "A");
}

static void testWorkerSupplyDeliveryPreservesFixedWorkerCount() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);
    assert(registry.view<FixedActorComponent>().size() == 1);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));

    updateWorkerSupplyDeliveryRoutes(registry, 5.0f);
    assert(registry.view<FixedActorComponent>().size() == 1);
}

static void testWorkerBenchDropOffStocksSharedWorkplaceBench() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);
    assert(registry.has<WorkplaceBenchComponent>(workplace));

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(!workerCanStockWorkplaceBench(registry, worker));

    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(workerAtWorkplaceEndpoint(registry, worker));
    assert(workerCanStockWorkplaceBench(registry, worker));
    assert(stockWorkplaceBenchForWorker(registry, worker));

    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::STOCKED);
    assert(workplaceBenchStocked(registry));
    assert(worker_component.carried_object == MAX_ENTITIES);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<TransformComponent>(object).y == 99999.0f);
    assert(registry.get<GlyphComponent>(worker).chars == "a");
    assert(workerReturningToSupply(registry, worker));
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: WORKING BENCH; REASON: WAGE ROUTE; CARRYING: NONE; WORK BENCH: STOCKED");
}

static void testWorkerBenchDropOffRequiresWorkplaceEndpointAndEmptyBench() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(!stockWorkplaceBenchForWorker(registry, worker));
    assert(worker_component.carried_object == object);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::EMPTY);

    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(!workerCanStockWorkplaceBench(registry, worker));
    assert(!stockWorkplaceBenchForWorker(registry, worker));
    assert(worker_component.carried_object == object);
}

static void testWorkerBenchDropOffUpdateAvoidsDuplicateObjects() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));

    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 1);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 0);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::STOCKED);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(registry.view<FixedActorComponent>().size() == 1);
    assert(worker_component.carried_object == MAX_ENTITIES);
}

static void testWorkerReturnRouteMovesBackToSupplyAfterBenchDropOff() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(stockWorkplaceBenchForWorker(registry, worker));

    const float workplace_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    const float supply_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    assert(closeTo(worker_component.route_t, workplace_t));
    assert(closeTo(worker_component.direction, 0.0f));
    assert(workerReturningToSupply(registry, worker));
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: WORKING BENCH; REASON: WAGE ROUTE; CARRYING: NONE; WORK BENCH: STOCKED");

    const float start_distance = std::fabs(worker_component.route_t - supply_t);
    assert(updateWorkerReturnRoutes(registry, 1.0f) == 1);
    assert(std::fabs(worker_component.route_t - supply_t) < start_distance);
    assert(worker_component.carried_object == MAX_ENTITIES);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(registry.view<FixedActorComponent>().size() == 1);

    assert(updateWorkerReturnRoutes(registry, 100.0f) == 1);
    assert(closeTo(worker_component.route_t, supply_t));
    assert(closeTo(worker_component.direction, 0.0f));
    assert(!workerReturningToSupply(registry, worker));
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: PICKING UP SUPPLY; REASON: WAGE ROUTE; CARRYING: NONE; WORK BENCH: STOCKED; BLOCKED: BENCH OCCUPIED");
}

static void testWorkerReturnRouteMissingSupplyPathNoOp() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity housing_workplace_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    assert(worker != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(housing_workplace_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = housing_workplace_path;
    worker_component.route_t = routeTForPathEndpoint(registry, housing_workplace_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(housing_workplace_path),
                        worker_component.route_t);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;

    const float original_t = worker_component.route_t;
    assert(!workerReturningToSupply(registry, worker));
    assert(!routeWorkerReturningToSupply(registry, worker));
    assert(updateWorkerReturnRoutes(registry, 10.0f) == 0);
    assert(closeTo(worker_component.route_t, original_t));
    assert(closeTo(worker_component.direction, 0.0f));
    assert(registry.view<FixedActorComponent>().size() == 1);
}

static void testTinySaveRoundTripRestoresWorkerBenchDropOff() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(stockWorkplaceBenchForWorker(registry, worker));
    assert(closeTo(worker_component.direction, 0.0f));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::EMPTY;
    worker_component.carried_object = object;
    registry.get<TransformComponent>(object) = TransformComponent{-120.0f, -120.0f, 8.0f, 8.0f};

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::STOCKED);
    assert(registry.get<FixedActorComponent>(worker).carried_object == MAX_ENTITIES);
    assert(closeTo(registry.get<FixedActorComponent>(worker).direction, 0.0f));
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: WORKING BENCH; REASON: WAGE ROUTE; CARRYING: NONE; WORK BENCH: STOCKED");
}

static void testWorkerBenchWorkCreatesSharedOutputOnlyWhenStocked() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(!workerCanWorkWorkplaceBench(registry, worker));
    assert(!workWorkplaceBenchForWorker(registry, worker));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(workerCanWorkWorkplaceBench(registry, worker));
    assert(workWorkplaceBenchForWorker(registry, worker));
    assert(workplaceBenchOutputReady(registry));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: OUTPUT READY");
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: TAKING PART; REASON: WAGE ROUTE; CARRYING: NONE; WORK BENCH: OUTPUT READY");
    assert(!workerReturningToSupply(registry, worker));
    assert(!workerCanWorkWorkplaceBench(registry, worker));
    assert(!workWorkplaceBenchForWorker(registry, worker));
    assert(updateWorkerWorkplaceBenchWork(registry) == 0);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
}

static void testWorkerBenchWorkRequiresWorkplaceEndpointAndEmptyHands() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(!workerCanWorkWorkplaceBench(registry, worker));

    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.carried_object = object;
    hideCarryableObject(registry, object);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(!workerCanWorkWorkplaceBench(registry, worker));
    assert(!workWorkplaceBenchForWorker(registry, worker));
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::STOCKED);
}

static void testWorkerTakesFinishedOutputAsSharedPart() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(!workerCanTakeWorkplaceOutput(registry, worker));
    assert(!takeWorkplaceOutputForWorker(registry, worker));

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    assert(workerCanTakeWorkplaceOutput(registry, worker));
    assert(takeWorkplaceOutputForWorker(registry, worker));
    assert(worker_component.carried_object == object);
    assert(carryableObjectIsKind(registry, object, ItemKind::PART));
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::EMPTY);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<GlyphComponent>(worker).chars == "P");
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING PART; REASON: WAGE ROUTE; CARRYING: PART");
    assert(!workerCanTakeWorkplaceOutput(registry, worker));
    assert(updateWorkerWorkplaceOutputPickups(registry) == 0);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
}

static void testWorkerFinishedItemRoutesTowardHousing() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    Entity housing_workplace_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);
    assert(housing_workplace_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    worker_component.carried_object = object;
    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    hideCarryableObject(registry, object);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(routeWorkerCarryingPartTowardHousing(registry, worker));
    assert(worker_component.path_entity == housing_workplace_path);
    const float housing_t = routeTForPathEndpoint(registry, housing_workplace_path, housing);
    assert(std::fabs(worker_component.route_t - housing_t) > 0.001f);

    const float start_distance = std::fabs(worker_component.route_t - housing_t);
    assert(updateWorkerFinishedItemDeliveryRoutes(registry, 1.0f) == 1);
    assert(std::fabs(worker_component.route_t - housing_t) < start_distance);
    assert(worker_component.carried_object == object);

    assert(updateWorkerFinishedItemDeliveryRoutes(registry, 100.0f) == 1);
    assert(closeTo(worker_component.route_t, housing_t));
    assert(closeTo(worker_component.direction, 0.0f));
    assert(workerAtHousingEndpoint(registry, worker));
}

static void testWorkerBuildingDeliveryCompletesSharedImprovement() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity housing_workplace_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(housing_workplace_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = housing_workplace_path;
    worker_component.route_t = routeTForPathEndpoint(registry, housing_workplace_path, workplace);
    worker_component.direction = 0.0f;
    worker_component.carried_object = object;
    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    hideCarryableObject(registry, object);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(housing_workplace_path),
                        worker_component.route_t);
    assert(!workerCanImproveBuilding(registry, worker));
    assert(!improveBuildingForWorker(registry, worker));
    assert(!buildingImproved(registry));

    worker_component.route_t = routeTForPathEndpoint(registry, housing_workplace_path, housing);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(housing_workplace_path),
                        worker_component.route_t);
    assert(workerCanImproveBuilding(registry, worker));
    assert(improveBuildingForWorker(registry, worker));
    assert(buildingImproved(registry));
    assert(buildingImprovementReadout(registry) == "BUILDING IMPROVED: YES");
    assert(worker_component.carried_object == MAX_ENTITIES);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(registry.get<GlyphComponent>(worker).chars == "a");
    assert(!improveBuildingForWorker(registry, worker));
    assert(updateWorkerBuildingDeliveries(registry) == 0);
    assert((registry.view<CarryableComponent, TransformComponent>().size() == 1));
    assert(registry.view<FixedActorComponent>().size() == 1);
}

static void testWorkerFullLoopCanImproveBuildingWithoutPlayerActions() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(updateWorkerSupplyPickups(registry) == 1);
    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 1);
    assert(updateWorkerWorkplaceBenchWork(registry) == 1);
    assert(updateWorkerWorkplaceOutputPickups(registry) == 1);
    assert(updateWorkerFinishedItemDeliveryRoutes(registry, 100.0f) == 1);
    assert(updateWorkerBuildingDeliveries(registry) == 1);

    assert(buildingImproved(registry));
    assert(workplaceBenchReadout(registry) == "WORK BENCH: EMPTY");
    assert(worker_component.carried_object == MAX_ENTITIES);
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: DONE; REASON: WAGE ROUTE; CARRYING: NONE; BUILDING IMPROVED: YES; BLOCKED: BUILDING ALREADY IMPROVED");
}

static void testWorkerRoutineStateCoversCurrentLoopStages() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(workerRoutineState(registry, worker) == "PICKING UP SUPPLY");
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(workerRoutineState(registry, worker) == "DELIVERING SUPPLY");

    assert(updateWorkerSupplyDeliveryRoutes(registry, 100.0f) == 1);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 1);
    assert(workerRoutineState(registry, worker) == "WORKING BENCH");

    assert(updateWorkerWorkplaceBenchWork(registry) == 1);
    assert(workerRoutineState(registry, worker) == "TAKING PART");

    assert(updateWorkerWorkplaceOutputPickups(registry) == 1);
    assert(workerRoutineState(registry, worker) == "DELIVERING PART");

    assert(updateWorkerFinishedItemDeliveryRoutes(registry, 100.0f) == 1);
    assert(updateWorkerBuildingDeliveries(registry) == 1);
    assert(workerRoutineState(registry, worker) == "DONE");
}

static void testWorkerLaborReasonTagIsReadoutOnly() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    assert(std::string(workerLaborReasonTag(registry, worker)) == "WAGE ROUTE");
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: PICKING UP SUPPLY; REASON: WAGE ROUTE; CARRYING: NONE");
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(worker_component.carried_object != MAX_ENTITIES);
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");
}

static void testWorkerRoutineStateHandlesZeroWorkerConfig() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 0;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    assert(firstFixedWorker(registry) == MAX_ENTITIES);
    assert(workerRoutineState(registry, MAX_ENTITIES) == "UNKNOWN");
    assert(workerCarryReadout(registry, MAX_ENTITIES) ==
           "WORKER ROUTINE: UNKNOWN; REASON: WAGE ROUTE; CARRYING: NONE");
}

static void testWorkerBlockedReasonLabelsCurrentScopeStates() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    assert(workerBlockedReason(registry, worker) == "NO SUPPLY");
    registry.get<CarryableComponent>(object).kind = ItemKind::SUPPLY;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(workerBlockedReason(registry, worker) == "WAITING FOR SUPPLY");
    assert(workerCarryReadout(registry, worker).find("BLOCKED: WAITING FOR SUPPLY") !=
           std::string::npos);

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(workerBlockedReason(registry, worker) == "BENCH OCCUPIED");

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(workerBlockedReason(registry, worker) == "OUTPUT WAITING");

    registry.get<BuildingImprovementComponent>(housing).improved = true;
    assert(workerBlockedReason(registry, worker) == "BUILDING ALREADY IMPROVED");
}

static void testBenchAndBuildingLoopReadoutsExposeBlockages() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity object = firstCarryableObject(registry);
    assert(workplace != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    assert(workplaceBenchLoopReadout(registry) == "WORK BENCH: EMPTY; LOOP: NEEDS SUPPLY");
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::STOCKED;
    assert(workplaceBenchLoopReadout(registry) ==
           "WORK BENCH: STOCKED; LOOP: BENCH OCCUPIED");

    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(workplaceBenchLoopReadout(registry) ==
           "WORK BENCH: OUTPUT WAITING; LOOP: READY FOR PICKUP");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(workplaceBenchLoopReadout(registry) ==
           "WORK BENCH: OUTPUT WAITING; BLOCKED: BLOCKED BY CARRIER");

    assert(buildingImprovementLoopReadout(registry) ==
           "BUILDING: NEEDS PART; BUILDING IMPROVED: NO");
    registry.get<BuildingImprovementComponent>(housing).improved = true;
    assert(buildingImprovementLoopReadout(registry) ==
           "BUILDING: IMPROVED; BUILDING IMPROVED: YES");

    registry.get<CarryableComponent>(object).kind = ItemKind::PART;
    assert(buildingImprovementLoopReadout(registry) ==
           "BUILDING: ALREADY COMPLETE; BUILDING IMPROVED: YES");
    assert(housingInteriorReadout(registry) ==
           "SHELTER SUPPLY: 0/1; BUILDING: ALREADY COMPLETE; BUILDING IMPROVED: YES");
}

static void testWorkerBlockedSupplyPickupResumesAfterPlayerReleasesSupply() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);

    assert(updateWorkerSupplyPickups(registry) == 0);
    assert(workerBlockedReason(registry, worker) == "WAITING FOR SUPPLY");
    assert(workerCarryReadout(registry, worker).find("BLOCKED: WAITING FOR SUPPLY") !=
           std::string::npos);
    assert(productionConsequenceReadout(registry) == "CONSEQUENCE: SHIFT STALLED");
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: BLOCKED; CONSEQUENCE: SHIFT STALLED");

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = TransformComponent{0.0f, 0.0f, 8.0f, 8.0f};
    assert(availableSupplyCarryableObject(registry) == MAX_ENTITIES);
    assert(!workerCanTakeSupplyObject(registry, worker));
    assert(updateWorkerSupplyPickups(registry) == 0);
    assert(workerBlockedReason(registry, worker) == "NO SUPPLY");

    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(availableSupplyCarryableObject(registry) == object);
    assert(workerBlockedReason(registry, worker).empty());
    assert(productionConsequenceReadout(registry) == "CONSEQUENCE: NONE");
    assert(updateWorkerSupplyPickups(registry) == 1);
    assert(worker_component.carried_object == object);
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");
}

static void testPlayerHeldExpectedSupplyAddsLocalConsequence() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));

    assert(!playerCarryingExpectedSupplyForWorker(registry, worker));
    assert(productionConsequenceReadout(registry) == "CONSEQUENCE: NONE");

    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(playerCarryingExpectedSupplyForWorker(registry, worker));
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: PICKING UP SUPPLY; REASON: WAGE ROUTE; CARRYING: NONE; BLOCKED: WAITING FOR SUPPLY; CONSEQUENCE: SHIFT STALLED");

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(!playerCarryingExpectedSupplyForWorker(registry, worker));
    assert(productionConsequenceReadout(registry) == "CONSEQUENCE: NONE");
    assert(workerCarryReadout(registry, worker) ==
           "WORKER ROUTINE: PICKING UP SUPPLY; REASON: WAGE ROUTE; CARRYING: NONE");
}

static void testProductionLoopSummaryReadoutCoversRunningBlockedAndComplete() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply));
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: BLOCKED; CONSEQUENCE: SHIFT STALLED");

    registry.get<PlayerComponent>(player).carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");

    registry.get<BuildingImprovementComponent>(housing).improved = true;
    assert(productionLoopSummaryReadout(registry) == "LOOP: COMPLETE");
}

static void testTinySaveRoundTripRestoresWorkerCarriedPart() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeWorkplaceOutputForWorker(registry, worker));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    worker_component.carried_object = MAX_ENTITIES;
    registry.get<CarryableComponent>(object).kind = ItemKind::SUPPLY;
    registry.get<TransformComponent>(object) = TransformComponent{-120.0f, -120.0f, 8.0f, 8.0f};

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(registry.get<FixedActorComponent>(worker).carried_object == object);
    assert(carryableObjectIsKind(registry, object, ItemKind::PART));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING PART; REASON: WAGE ROUTE; CARRYING: PART");
}

static void testTinySaveRoundTripRestoresWorkerCarriedSupply() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    Entity worker = firstFixedWorker(registry);
    Entity object = registry.view<CarryableComponent, TransformComponent>().front();
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, supply);
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);
    assert(takeSupplyObjectForWorker(registry, worker));

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    worker_component.carried_object = MAX_ENTITIES;
    registry.get<TransformComponent>(object) = TransformComponent{-120.0f, -120.0f, 8.0f, 8.0f};

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(registry.get<FixedActorComponent>(worker).carried_object == object);
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(registry.get<TransformComponent>(object).x == 99999.0f);
    assert(workerCarryReadout(registry, worker) == "WORKER ROUTINE: DELIVERING SUPPLY; REASON: WAGE ROUTE; CARRYING: SUPPLY");
}

static void testInheritedGadgetReadoutAndCarryableBehavior() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.carryable_object_count = 1;
    buildWorld(registry, config);

    Entity player = registry.create();
    Entity object = firstCarryableObject(registry);
    assert(object != MAX_ENTITIES);
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(object));

    assert(playerHasInheritedGadget(registry, player));
    assert(inheritedGadgetLabel(registry, player) == "MOTHER'S DEBUGGER");
    assert(inheritedGadgetReadout(registry, player) == "GADGET:MOTHER'S DEBUGGER READY");
    assert(inheritedGadgetResultReadout(registry, player) == "GADGET RESULT: IDLE");

    assert(playerCanTakeNearbyCarryableObject(registry, player, 18.0f));
    assert(takeNearbyCarryableObject(registry, player, 18.0f));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(carryableObjectIsKind(registry, object, ItemKind::SUPPLY));
    assert(inheritedGadgetReadout(registry, player) == "GADGET:MOTHER'S DEBUGGER READY");
}

static void testInheritedGadgetPromptAndNoSignalBehavior() {
    Registry registry;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);

    assert(!playerCanUseInheritedGadget(registry, player));
    assert(inheritedGadgetPromptReadout(registry, player, 18.0f) == "GADGET:NONE");

    registry.assign<InheritedGadgetComponent>(player);

    assert(playerCanUseInheritedGadget(registry, player));
    assert(inheritedGadgetPromptReadout(registry, player, 18.0f) ==
           "G USE DEBUGGER: NO SIGNAL");
    assert(inheritedGadgetResultReadout(registry, player) == "GADGET RESULT: IDLE");
    assert(!useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "GADGET RESULT: NO SIGNAL");
}

static void testInheritedGadgetTargetResultReplacesPreviousResult() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(worker));

    assert(inheritedGadgetPromptReadout(registry, player, 18.0f) ==
           "G USE DEBUGGER ON WORKER");
    assert(useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: WORKER SIGNAL: DEBT WORK; PAY DOCKED IF STALLED; ROUTE QUOTA: 1");

    registry.get<TransformComponent>(player) = TransformComponent{4000.0f, 4000.0f, 12.0f, 12.0f};
    assert(!useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "GADGET RESULT: NO SIGNAL");
}

static void testInheritedGadgetWorkerScanRevealsHiddenLaborDetail() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    const std::string ordinary_readout = workerCarryReadout(registry, worker);
    assert(ordinary_readout.find("DEBT WORK") == std::string::npos);
    assert(ordinary_readout.find("PAY DOCKED") == std::string::npos);
    assert(ordinary_readout.find("ROUTE QUOTA") == std::string::npos);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(worker));

    assert(useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: WORKER SIGNAL: DEBT WORK; PAY DOCKED IF STALLED; ROUTE QUOTA: 1");
    assert(workerCarryReadout(registry, worker) == ordinary_readout);
}

static void testInheritedGadgetSiteMetadataScan() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    assert(enterBuildingInterior(registry, player, workplace));

    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: WORKPLACE PURPOSE: CONVERT SUPPLY TO PART");

    exitBuildingInterior(registry, player);
    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(marker);

    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player).find("ROUTE CARRIES: SUPPLY/PART") !=
           std::string::npos);
}

static void testInheritedGadgetInvalidTargetDoesNotAlterWorkerInspection() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    const std::string ordinary_readout = workerCarryReadout(registry, worker);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, 4000.0f, 4000.0f, 12.0f, 12.0f);

    assert(!useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "GADGET RESULT: NO SIGNAL");
    assert(workerCarryReadout(registry, worker) == ordinary_readout);
}

static void testInheritedGadgetSpoofCandidateSelection() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(housing));

    assert(!inheritedGadgetCanSpoofTarget(playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "SHIFT+G SPOOF:N/A");
    assert(!useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: SPOOF FAILED: SIGNPOST REQUIRED");
    assert(!anyRouteSignpostSpoofed(registry));

    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(marker);

    assert(inheritedGadgetCanSpoofTarget(playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "SHIFT+G SPOOF SIGNPOST");
    assert(routeSignpostReadout(registry, marker).find("SPOOFED") == std::string::npos);
}

static void testInheritedGadgetSpoofTogglesSignpostConsequence() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(marker));

    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");
    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostSpoofed(registry, marker));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED: ROUTE SIGNAL CONFUSED") !=
           std::string::npos);
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: SPOOFED SIGNPOST: ROUTE SIGNAL CONFUSED");
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "SHIFT+G RESTORE SIGNPOST");
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: SPOOFED; INTERFERENCE: ROUTE; CONSEQUENCE: ROUTE SIGNAL CONFUSED");

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!routeSignpostSpoofed(registry, marker));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED") == std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("RECOVERY: ROUTE SIGNAL CLEAR") !=
           std::string::npos);
    assert(inheritedGadgetResultReadout(registry, player) ==
           "GADGET RESULT: RESTORED SIGNPOST: ROUTE SIGNAL CLEAR");
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");
}

static void testRouteSignpostRecoveryReadoutAppearsOnlyAfterRestore() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(marker));

    assert(routeSignpostReadout(registry, marker).find("ROUTE SIGNAL CLEAR") ==
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED: ROUTE SIGNAL CONFUSED") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("ROUTE SIGNAL CLEAR") ==
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED") == std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("RECOVERY: ROUTE SIGNAL CLEAR") !=
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED: ROUTE SIGNAL CONFUSED") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("ROUTE SIGNAL CLEAR") ==
           std::string::npos);
}

static void testRouteSignpostConsequenceReadoutCoversLocalStates() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    auto& worker_component = registry.get<FixedActorComponent>(worker);
    Entity marker = firstRouteSignpostForPath(registry, worker_component.path_entity);
    assert(marker != MAX_ENTITIES);

    std::string readout = routeSignpostReadout(registry, marker);
    assert(readout.find("SIGNAL: CLEAR") != std::string::npos);
    assert(readout.find("CONSEQUENCE: NONE") != std::string::npos);
    assert(readout.find("SPOOFED") == std::string::npos);
    assert(readout.find("RECOVERY") == std::string::npos);

    const float start_t = worker_component.route_t;
    registry.get<RouteSignpostComponent>(marker).spoofed = true;
    readout = routeSignpostReadout(registry, marker);
    assert(readout.find("SIGNAL: CORRUPTED") != std::string::npos);
    assert(readout.find("SPOOFED: ROUTE SIGNAL CONFUSED") != std::string::npos);
    assert(readout.find("CONSEQUENCE: ROUTE DELAY") != std::string::npos);
    assert(readout.find("RECOVERY") == std::string::npos);

    updateFixedActors(registry, 10.0f);
    assert(closeTo(worker_component.route_t, start_t));
    readout = routeSignpostReadout(registry, marker);
    assert(readout.find("SIGNAL: CORRUPTED") != std::string::npos);
    assert(readout.find("CONSEQUENCE: ROUTE DELAY") != std::string::npos);

    registry.get<RouteSignpostComponent>(marker).spoofed = false;
    registry.get<RouteSignpostComponent>(marker).signal_recovered = true;
    readout = routeSignpostReadout(registry, marker);
    assert(readout.find("SIGNAL: CLEAR") != std::string::npos);
    assert(readout.find("RECOVERY: ROUTE SIGNAL CLEAR") != std::string::npos);
    assert(readout.find("CONSEQUENCE: NONE") != std::string::npos);
    assert(readout.find("SPOOFED") == std::string::npos);
}

static void testProductionLoopSummaryReadoutCoversRouteInterferenceStates() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    auto& worker_component = registry.get<FixedActorComponent>(worker);
    Entity marker = firstRouteSignpostForPath(registry, worker_component.path_entity);
    assert(marker != MAX_ENTITIES);

    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");

    const float start_t = worker_component.route_t;
    registry.get<RouteSignpostComponent>(marker).spoofed = true;
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: SPOOFED; INTERFERENCE: ROUTE; CONSEQUENCE: ROUTE SIGNAL CONFUSED");

    updateFixedActors(registry, 10.0f);
    assert(closeTo(worker_component.route_t, start_t));
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: SPOOFED; INTERFERENCE: ROUTE; CONSEQUENCE: ROUTE SIGNAL CONFUSED");

    registry.get<RouteSignpostComponent>(marker).spoofed = false;
    registry.get<RouteSignpostComponent>(marker).signal_recovered = true;
    assert(routeSignpostReadout(registry, marker).find("RECOVERY: ROUTE SIGNAL CLEAR") !=
           std::string::npos);
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");
}

static void testInheritedGadgetSpoofLeavesWorkerAndLoopStateUnchanged() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();

    const auto worker_before = registry.get<FixedActorComponent>(worker);
    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state;
    const bool building_improved_before = buildingImproved(registry);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(marker));

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));

    const auto& worker_after = registry.get<FixedActorComponent>(worker);
    assert(worker_after.path_entity == worker_before.path_entity);
    assert(worker_after.carried_object == worker_before.carried_object);
    assert(closeTo(worker_after.route_t, worker_before.route_t));
    assert(closeTo(worker_after.direction, worker_before.direction));
    assert(registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state ==
           bench_before);
    assert(buildingImproved(registry) == building_improved_before);
}

static void testSpoofedRouteSignalAffectsOnlyRelevantWorkerPathReadout() {
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

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    const Entity worker_path = registry.get<FixedActorComponent>(worker).path_entity;
    Entity relevant_signpost = firstRouteSignpostForPath(registry, worker_path);
    Entity unrelated_signpost = firstRouteSignpostNotForPath(registry, worker_path);
    assert(relevant_signpost != MAX_ENTITIES);
    assert(unrelated_signpost != MAX_ENTITIES);

    registry.get<RouteSignpostComponent>(unrelated_signpost).spoofed = true;
    assert(!workerCurrentPathHasSpoofedRouteSignpost(registry, worker));
    assert(workerCarryReadout(registry, worker).find("ROUTE SIGNAL CONFUSED") ==
           std::string::npos);

    registry.get<RouteSignpostComponent>(unrelated_signpost).spoofed = false;
    registry.get<RouteSignpostComponent>(relevant_signpost).spoofed = true;
    assert(workerCurrentPathHasSpoofedRouteSignpost(registry, worker));
    assert(workerCarryReadout(registry, worker).find("BLOCKED: ROUTE SIGNAL CONFUSED") !=
           std::string::npos);

    registry.get<RouteSignpostComponent>(relevant_signpost).spoofed = false;
    assert(!workerCurrentPathHasSpoofedRouteSignpost(registry, worker));
    assert(workerCarryReadout(registry, worker).find("ROUTE SIGNAL CONFUSED") ==
           std::string::npos);
}

static void testSpoofedRouteSignalSourceLabelAppearsAndClears() {
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

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    const Entity worker_path = registry.get<FixedActorComponent>(worker).path_entity;
    Entity relevant_signpost = firstRouteSignpostForPath(registry, worker_path);
    Entity unrelated_signpost = firstRouteSignpostNotForPath(registry, worker_path);
    assert(relevant_signpost != MAX_ENTITIES);
    assert(unrelated_signpost != MAX_ENTITIES);

    assert(workerConsequenceSourceReadout(registry, worker).empty());
    assert(workerRouteConsequenceReadout(registry, worker).empty());
    assert(workerCarryReadout(registry, worker).find("SOURCE: CORRUPTED ROUTE SIGNAL") ==
           std::string::npos);
    assert(workerCarryReadout(registry, worker).find("WAITING ON ROUTE SIGNAL") ==
           std::string::npos);

    registry.get<RouteSignpostComponent>(unrelated_signpost).spoofed = true;
    assert(workerConsequenceSourceReadout(registry, worker).empty());
    assert(workerRouteConsequenceReadout(registry, worker).empty());
    assert(workerCarryReadout(registry, worker).find("SOURCE: CORRUPTED ROUTE SIGNAL") ==
           std::string::npos);
    assert(workerCarryReadout(registry, worker).find("WAITING ON ROUTE SIGNAL") ==
           std::string::npos);

    registry.get<RouteSignpostComponent>(unrelated_signpost).spoofed = false;
    registry.get<RouteSignpostComponent>(relevant_signpost).spoofed = true;
    assert(workerConsequenceSourceReadout(registry, worker) ==
           "SOURCE: CORRUPTED ROUTE SIGNAL");
    assert(workerRouteConsequenceReadout(registry, worker) ==
           "WAITING ON ROUTE SIGNAL");
    assert(workerCarryReadout(registry, worker).find("SOURCE: CORRUPTED ROUTE SIGNAL") !=
           std::string::npos);
    assert(workerCarryReadout(registry, worker).find("WAITING ON ROUTE SIGNAL") !=
           std::string::npos);

    registry.get<RouteSignpostComponent>(relevant_signpost).spoofed = false;
    assert(workerConsequenceSourceReadout(registry, worker).empty());
    assert(workerRouteConsequenceReadout(registry, worker).empty());
    assert(workerCarryReadout(registry, worker).find("SOURCE: CORRUPTED ROUTE SIGNAL") ==
           std::string::npos);
    assert(workerCarryReadout(registry, worker).find("WAITING ON ROUTE SIGNAL") ==
           std::string::npos);
}

static void testSpoofedRouteSignalPausesAndRestoresWorkerPathMovement() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    auto& worker_component = registry.get<FixedActorComponent>(worker);
    Entity signpost = firstRouteSignpostForPath(registry, worker_component.path_entity);
    assert(signpost != MAX_ENTITIES);

    const float start_t = worker_component.route_t;
    registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    updateFixedActors(registry, 10.0f);
    assert(closeTo(worker_component.route_t, start_t));

    registry.get<RouteSignpostComponent>(signpost).spoofed = false;
    updateFixedActors(registry, 1.0f);
    assert(worker_component.route_t > start_t);
}

static void testSpoofedRouteSignalPausesAndRestoresSupplyDeliveryMovement() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity supply_path = firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    Entity object = firstCarryableObject(registry);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(supply_path), worker_component.route_t);
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(takeSupplyObjectForWorker(registry, worker));

    Entity signpost = firstRouteSignpostForPath(registry, worker_component.path_entity);
    assert(signpost != MAX_ENTITIES);
    const float start_t = worker_component.route_t;

    registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    assert(updateWorkerSupplyDeliveryRoutes(registry, 10.0f) == 1);
    assert(closeTo(worker_component.route_t, start_t));

    registry.get<RouteSignpostComponent>(signpost).spoofed = false;
    assert(updateWorkerSupplyDeliveryRoutes(registry, 1.0f) == 1);
    assert(!closeTo(worker_component.route_t, start_t));
}

static void testSpoofedRouteSignalConsequenceDoesNotMutateProductionState() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity worker = firstFixedWorker(registry);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity workplace = firstWorkplaceBenchBuilding(registry);
    Entity supply_path = firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    Entity object = firstCarryableObject(registry);
    assert(worker != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, supply_path, supply);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(supply_path), worker_component.route_t);
    registry.get<TransformComponent>(object) = registry.get<TransformComponent>(supply);
    assert(takeSupplyObjectForWorker(registry, worker));

    Entity signpost = firstRouteSignpostForPath(registry, worker_component.path_entity);
    assert(signpost != MAX_ENTITIES);

    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(workplace).state;
    const bool building_improved_before = buildingImproved(registry);
    const Entity carried_before = worker_component.carried_object;
    const float route_before = worker_component.route_t;

    registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    assert(workerBlockedReason(registry, worker) == "ROUTE SIGNAL CONFUSED");
    assert(workerConsequenceSourceReadout(registry, worker) ==
           "SOURCE: CORRUPTED ROUTE SIGNAL");
    assert(updateWorkerSupplyDeliveryRoutes(registry, 10.0f) == 1);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 0);
    assert(updateWorkerWorkplaceBenchWork(registry) == 0);
    assert(updateWorkerBuildingDeliveries(registry) == 0);

    assert(closeTo(worker_component.route_t, route_before));
    assert(worker_component.carried_object == carried_before);
    assert(carryableObjectIsHeld(registry, object));
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == bench_before);
    assert(buildingImproved(registry) == building_improved_before);
}

static void testTinySaveRoundTripRestoresSpoofedSignpostState() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    auto signposts = registry.view<RouteSignpostComponent>();
    assert(signposts.size() >= 2);
    Entity saved_marker = signposts.front();
    Entity unsaved_marker = signposts.back();
    assert(saved_marker != unsaved_marker);
    registry.get<RouteSignpostComponent>(saved_marker).spoofed = true;
    registry.get<RouteSignpostComponent>(unsaved_marker).spoofed = false;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("SPOOFED_SIGNPOSTS 1") != std::string::npos);

    TinySaveState parsed;
    assert(deserializeTinySaveState(serialized, parsed) == TinySaveStatus::OK);

    registry.get<RouteSignpostComponent>(saved_marker).spoofed = false;
    registry.get<RouteSignpostComponent>(unsaved_marker).spoofed = true;

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(routeSignpostSpoofed(registry, saved_marker));
    assert(!routeSignpostSpoofed(registry, unsaved_marker));
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
    testRouteSignpostInspectionTarget();
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
    testTinySaveRoundTripRestoresCurrentScopeState();
    testTinySaveFileFailureAndRoundTrip();
    testTinySaveStatusTextSelection();
    testCarryableItemKindLabelsAndSaveRoundTrip();
    testCarryableObjectInteractions();
    testNearbyCarryablePickupRequiresOutsideAndEmptyHands();
    testSupplyInteriorPickupUsesExistingObject();
    testSupplyInteriorPickupIsSupplyOnly();
    testSupplyInteriorPickupRequiresEmptyHands();
    testShelterDropOffStoresSupplyOnlyInHousing();
    testShelterDropOffRequiresCapacityAndCarriedObject();
    testTinySaveRoundTripRestoresShelterStock();
    testWorkplaceDeliveryStocksBenchOnlyInWorkplace();
    testWorkplaceDeliveryRequiresEmptyBenchAndCarriedObject();
    testInsideWorkplaceInspectionReflectsBenchState();
    testTinySaveRoundTripRestoresWorkplaceBench();
    testPlayerBenchWorkCreatesOutputOnlyWhenStocked();
    testPlayerTakesFinishedOutputAsSingleCarriedItem();
    testPlayerImprovesBuildingWithFinishedItemOnly();
    testWorkerSupplyPickupRequiresSupplyEndpoint();
    testWorkerSupplyPickupBlockedWhilePlayerCarriesObject();
    testWorkerSupplyPickupBlockedByStoredSupplyStates();
    testWorkerSupplyDeliveryMovesTowardWorkplaceEndpoint();
    testWorkerSupplyDeliveryStopsAtWorkplaceEndpoint();
    testWorkerSupplyDeliveryMissingSupplyPathNoOp();
    testWorkerSupplyDeliveryPreservesFixedWorkerCount();
    testWorkerBenchDropOffStocksSharedWorkplaceBench();
    testWorkerBenchDropOffRequiresWorkplaceEndpointAndEmptyBench();
    testWorkerBenchDropOffUpdateAvoidsDuplicateObjects();
    testWorkerReturnRouteMovesBackToSupplyAfterBenchDropOff();
    testWorkerReturnRouteMissingSupplyPathNoOp();
    testTinySaveRoundTripRestoresWorkerBenchDropOff();
    testWorkerBenchWorkCreatesSharedOutputOnlyWhenStocked();
    testWorkerBenchWorkRequiresWorkplaceEndpointAndEmptyHands();
    testWorkerTakesFinishedOutputAsSharedPart();
    testWorkerFinishedItemRoutesTowardHousing();
    testWorkerBuildingDeliveryCompletesSharedImprovement();
    testWorkerFullLoopCanImproveBuildingWithoutPlayerActions();
    testWorkerRoutineStateCoversCurrentLoopStages();
    testWorkerLaborReasonTagIsReadoutOnly();
    testWorkerRoutineStateHandlesZeroWorkerConfig();
    testWorkerBlockedReasonLabelsCurrentScopeStates();
    testBenchAndBuildingLoopReadoutsExposeBlockages();
    testWorkerBlockedSupplyPickupResumesAfterPlayerReleasesSupply();
    testPlayerHeldExpectedSupplyAddsLocalConsequence();
    testProductionLoopSummaryReadoutCoversRunningBlockedAndComplete();
    testTinySaveRoundTripRestoresWorkerCarriedPart();
    testTinySaveRoundTripRestoresWorkerCarriedSupply();
    testInheritedGadgetReadoutAndCarryableBehavior();
    testInheritedGadgetPromptAndNoSignalBehavior();
    testInheritedGadgetTargetResultReplacesPreviousResult();
    testInheritedGadgetWorkerScanRevealsHiddenLaborDetail();
    testInheritedGadgetSiteMetadataScan();
    testInheritedGadgetInvalidTargetDoesNotAlterWorkerInspection();
    testInheritedGadgetSpoofCandidateSelection();
    testInheritedGadgetSpoofTogglesSignpostConsequence();
    testRouteSignpostRecoveryReadoutAppearsOnlyAfterRestore();
    testRouteSignpostConsequenceReadoutCoversLocalStates();
    testProductionLoopSummaryReadoutCoversRouteInterferenceStates();
    testInheritedGadgetSpoofLeavesWorkerAndLoopStateUnchanged();
    testSpoofedRouteSignalAffectsOnlyRelevantWorkerPathReadout();
    testSpoofedRouteSignalSourceLabelAppearsAndClears();
    testSpoofedRouteSignalPausesAndRestoresWorkerPathMovement();
    testSpoofedRouteSignalPausesAndRestoresSupplyDeliveryMovement();
    testSpoofedRouteSignalConsequenceDoesNotMutateProductionState();
    testTinySaveRoundTripRestoresSpoofedSignpostState();
    return 0;
}
