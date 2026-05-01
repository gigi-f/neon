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

static bool pathTouchesEndpointTransform(const TransformComponent& path,
                                         const TransformComponent& endpoint) {
    constexpr float eps = 0.01f;
    const AabbRect path_box = aabbFromTransform(path);
    const AabbRect endpoint_box = aabbFromTransform(endpoint);
    const bool vertical = path.height >= path.width;
    if (vertical) {
        const bool x_aligned = path_box.left >= endpoint_box.left - eps &&
                               path_box.right <= endpoint_box.right + eps;
        const bool y_touches = std::fabs(path_box.top - endpoint_box.bottom) <= eps ||
                               std::fabs(path_box.bottom - endpoint_box.top) <= eps;
        return x_aligned && y_touches;
    }

    const bool y_aligned = path_box.top >= endpoint_box.top - eps &&
                           path_box.bottom <= endpoint_box.bottom + eps;
    const bool x_touches = std::fabs(path_box.left - endpoint_box.right) <= eps ||
                           std::fabs(path_box.right - endpoint_box.left) <= eps;
    return y_aligned && x_touches;
}

static bool pathTouchesBothEndpoints(Registry& registry, Entity path_entity) {
    if (!registry.alive(path_entity) ||
        !registry.has<PathComponent>(path_entity) ||
        !registry.has<TransformComponent>(path_entity)) {
        return false;
    }

    const auto& path = registry.get<PathComponent>(path_entity);
    return registry.alive(path.from) &&
           registry.alive(path.to) &&
           registry.has<TransformComponent>(path.from) &&
           registry.has<TransformComponent>(path.to) &&
           pathTouchesEndpointTransform(registry.get<TransformComponent>(path_entity),
                                        registry.get<TransformComponent>(path.from)) &&
           pathTouchesEndpointTransform(registry.get<TransformComponent>(path_entity),
                                        registry.get<TransformComponent>(path.to));
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

static void testCommercialSiteRolePlacementAndInspection() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;

    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    assert(validateWorld(registry, config));
    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    assert(market != MAX_ENTITIES);
    assert(registry.get<BuildingUseComponent>(market).role == MicroZoneRole::MARKET);
    assert(!registry.get<BuildingComponent>(market).is_enterable);
    assert(!registry.has<ShelterStockComponent>(market));
    assert(!registry.has<WorkplaceBenchComponent>(market));
    assert(!registry.has<BuildingImprovementComponent>(market));

    const auto& glyph = registry.get<GlyphComponent>(market);
    assert(glyph.chars == "m");
    assert(glyph.r == 210);
    assert(glyph.g == 120);
    assert(glyph.b == 235);

    size_t market_micros = 0;
    for (Entity micro : registry.view<MicroZoneComponent>()) {
        if (registry.get<MicroZoneComponent>(micro).role == MicroZoneRole::MARKET) {
            ++market_micros;
        }
    }
    assert(market_micros == 1);
    assert(buildingsDoNotOverlap(registry));

    const auto market_box = aabbFromTransform(registry.get<TransformComponent>(market));
    for (Entity path : registry.view<PathComponent, TransformComponent>()) {
        assert(!aabbOverlap(market_box, aabbFromTransform(registry.get<TransformComponent>(path))));
    }

    const TransformComponent player_at_market = registry.get<TransformComponent>(market);
    assert(nearestInspectionTargetInRange(registry, player_at_market, 22.0f).type ==
           InspectionTargetType::MARKET);
    assert(nearestInteractableBuildingInRange(registry, player_at_market, 22.0f) == MAX_ENTITIES);

    const std::string readout = buildingInspectionReadout(registry, market);
    assert(readout.find("MARKET; PURPOSE: EXCHANGE") != std::string::npos);
    assert(readout.find("FUNCTION: EXCHANGE SITE") != std::string::npos);
    assert(readout.find("SITE STATUS: OBSERVATION ONLY") != std::string::npos);
}

static void testFiveRoleLayoutKeepsWorkerPathsConnectedAndMarketClear() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;

    buildWorld(registry, config);
    assert(validateWorld(registry, config));
    assert(deriveInfrastructure(registry, config) == 2);
    assert(spawnFixedActors(registry, config) == 1);

    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    Entity housing_workplace =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    Entity workplace_supply =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    Entity worker = firstFixedWorker(registry);
    assert(market != MAX_ENTITIES);
    assert(housing_workplace != MAX_ENTITIES);
    assert(workplace_supply != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);

    const auto market_box = aabbFromTransform(registry.get<TransformComponent>(market));
    for (Entity path : registry.view<PathComponent, TransformComponent>()) {
        assert(pathTouchesBothEndpoints(registry, path));
        assert(!aabbOverlap(market_box, aabbFromTransform(registry.get<TransformComponent>(path))));
    }

    updateFixedActors(registry, 10.0f);

    const auto& worker_component = registry.get<FixedActorComponent>(worker);
    assert(worker_component.path_entity == workplace_supply);
    assert(pathTouchesBothEndpoints(registry, worker_component.path_entity));

    const Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    const auto& worker_transform = registry.get<TransformComponent>(worker);
    const auto workplace_box = aabbFromTransform(registry.get<TransformComponent>(workplace));
    assert(worker_transform.x >= workplace_box.left - 0.01f);
    assert(worker_transform.x <= workplace_box.right + 0.01f);
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
    assert(pathInspectionReadout(registry_with_workplace, paths.front()).find("ROUTE: LABOR ROUTE") !=
           std::string::npos);
    assert(pathInspectionReadout(registry_with_workplace, paths.front()).find("CARRIES: LABOR") !=
           std::string::npos);
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
        assert(readout.find("ROUTE: LABOR ROUTE") != std::string::npos);
        assert(readout.find("CARRIES: LABOR") != std::string::npos);
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
            assert(pathInspectionReadout(with_supply, path_entity).find("ROUTE: SUPPLY ROUTE") !=
                   std::string::npos);
            assert(pathInspectionReadout(with_supply, path_entity).find("CARRIES: MATERIAL") !=
                   std::string::npos);
        } else {
            assert(pathInspectionReadout(with_supply, path_entity).find("ROUTE: LABOR ROUTE") !=
                   std::string::npos);
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
        assert(readout.find("ROUTE: ") != std::string::npos);
        assert(readout.find("CARRIES: ") != std::string::npos);
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
    assert(serialized.find("NEON_TINY_SAVE_V12") != std::string::npos);
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
    assert(shelterSupplyReadout(registry) == "BUILDING SUPPLY: 0/1");

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
    assert(shelterSupplyReadout(registry) == "BUILDING SUPPLY: 1/1");
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
    assert(shelterSupplyReadout(registry) == "BUILDING SUPPLY: 0/1");

    assert(applyTinySaveState(registry, player, parsed) == TinySaveStatus::OK);
    assert(shelterSupplyCount(registry) == 1);
    assert(shelterSupplyReadout(registry) == "BUILDING SUPPLY: 1/1");
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
           "PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY; BUILDING SUPPLY: 0/1; BUILDING: IMPROVED; BUILDING IMPROVED: YES");
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

static void testBuildingPurposeForCurrentRoles() {
    const BuildingPurposeInfo housing = buildingPurposeForRole(MicroZoneRole::HOUSING);
    const BuildingPurposeInfo workplace = buildingPurposeForRole(MicroZoneRole::WORKPLACE);
    const BuildingPurposeInfo supply = buildingPurposeForRole(MicroZoneRole::SUPPLY);
    const BuildingPurposeInfo market = buildingPurposeForRole(MicroZoneRole::MARKET);

    assert(housing.role == MicroZoneRole::HOUSING);
    assert(std::string(housing.label) == "DWELLING");
    assert(std::string(housing.function) == "BUILDING RECOVERY");
    assert(buildingPurposeReadoutForRole(MicroZoneRole::HOUSING) ==
           "PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY");

    assert(workplace.role == MicroZoneRole::WORKPLACE);
    assert(std::string(workplace.label) == "PRODUCTION");
    assert(std::string(workplace.function) == "CONVERT SUPPLY TO PART");
    assert(buildingPurposeReadoutForRole(MicroZoneRole::WORKPLACE) ==
           "PURPOSE: PRODUCTION; FUNCTION: CONVERT SUPPLY TO PART");

    assert(supply.role == MicroZoneRole::SUPPLY);
    assert(std::string(supply.label) == "MATERIAL SOURCE");
    assert(std::string(supply.function) == "SOURCE LOOP MATERIAL");
    assert(buildingPurposeReadoutForRole(MicroZoneRole::SUPPLY) ==
           "PURPOSE: MATERIAL SOURCE; FUNCTION: SOURCE LOOP MATERIAL");

    assert(market.role == MicroZoneRole::MARKET);
    assert(std::string(market.label) == "EXCHANGE");
    assert(std::string(market.function) == "EXCHANGE SITE");
    assert(buildingPurposeReadoutForRole(MicroZoneRole::MARKET) ==
           "PURPOSE: EXCHANGE; FUNCTION: EXCHANGE SITE");
}

static void testBuildingInspectionReadoutsIncludePurposeForCurrentRoles() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(market != MAX_ENTITIES);

    std::string readout = buildingInspectionReadout(registry, housing);
    assert(readout.find("HOUSING; PURPOSE: DWELLING") != std::string::npos);
    assert(readout.find("FUNCTION: BUILDING RECOVERY") != std::string::npos);
    assert(readout.find("BUILDING: NEEDS PART") != std::string::npos);

    readout = buildingInspectionReadout(registry, workplace);
    assert(readout.find("WORKPLACE; PURPOSE: PRODUCTION") != std::string::npos);
    assert(readout.find("FUNCTION: CONVERT SUPPLY TO PART") != std::string::npos);
    assert(readout.find("WORK BENCH: EMPTY") != std::string::npos);

    readout = buildingInspectionReadout(registry, supply);
    assert(readout.find("SUPPLY; PURPOSE: MATERIAL SOURCE") != std::string::npos);
    assert(readout.find("FUNCTION: SOURCE LOOP MATERIAL") != std::string::npos);
    assert(readout.find("SITE STATUS: STOCK POINT") != std::string::npos);

    readout = buildingInspectionReadout(registry, market);
    assert(readout.find("MARKET; PURPOSE: EXCHANGE") != std::string::npos);
    assert(readout.find("FUNCTION: EXCHANGE SITE") != std::string::npos);
    assert(readout.find("SITE STATUS: OBSERVATION ONLY") != std::string::npos);

    assert(buildingPurposeScanReadoutForRole(MicroZoneRole::HOUSING) ==
           "HOUSING PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY");
    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{housing, InspectionTargetType::HOUSING}) ==
           "HOUSING PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY");
    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{supply, InspectionTargetType::SUPPLY}) ==
           "SUPPLY PURPOSE: MATERIAL SOURCE; FUNCTION: SOURCE LOOP MATERIAL; FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; SUPPORTS: WORKPLACE");
    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{market, InspectionTargetType::MARKET}) ==
           "MARKET PURPOSE: EXCHANGE; ACCESS: RESTRICTED");
}

static void testCommercialSiteDebuggerScanMetadataAndNoTarget() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    buildWorld(registry, config);

    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    assert(market != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(market));
    registry.assign<BuildingInteractionComponent>(player);

    assert(playerInspectionTarget(registry, player, 22.0f).type == InspectionTargetType::MARKET);
    assert(inheritedGadgetPromptReadout(registry, player, 22.0f) ==
           "SPACE DEBUGGER ON MARKET");
    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "DEBUGGER RESULT: MARKET PURPOSE: EXCHANGE; ACCESS: RESTRICTED");

    registry.get<TransformComponent>(player) = TransformComponent{4000.0f, 4000.0f, 12.0f, 12.0f};
    assert(inheritedGadgetPromptReadout(registry, player, 22.0f) ==
           "SPACE DEBUGGER: NO SIGNAL");
    assert(!useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: NO SIGNAL");
}

static void testCommercialSiteObservationOnlyBoundary() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    Entity worker = firstFixedWorker(registry);
    Entity housing_workplace_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(market != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(housing_workplace_path != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::MARKET) ==
           MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::MARKET) ==
           MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::SUPPLY, MicroZoneRole::MARKET) ==
           MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(market));
    registry.assign<BuildingInteractionComponent>(player);

    const auto worker_before = registry.get<FixedActorComponent>(worker);
    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state;
    const bool building_improved_before = buildingImproved(registry);
    const size_t path_count_before = registry.view<PathComponent>().size();

    assert(!enterBuildingInterior(registry, player, market));
    assert(playerLocationState(registry, player, 22.0f) == PlayerLocationState::OUTSIDE);
    assert(!playerCanTakeSupplyObject(registry, player));
    assert(!takeSupplyObjectFromInterior(registry, player));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!playerCanWorkWorkplaceBench(registry, player));
    assert(!playerCanTakeWorkplaceOutput(registry, player));
    assert(!playerCanImproveBuilding(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);

    assert(updateWorkerSupplyPickups(registry) == 0);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 0);
    assert(updateWorkerWorkplaceBenchWork(registry) == 0);
    assert(updateWorkerWorkplaceOutputPickups(registry) == 0);
    assert(updateWorkerBuildingDeliveries(registry) == 0);

    const auto& worker_after = registry.get<FixedActorComponent>(worker);
    assert(worker_after.path_entity == worker_before.path_entity);
    assert(worker_after.carried_object == worker_before.carried_object);
    assert(closeTo(worker_after.route_t, worker_before.route_t));
    assert(closeTo(worker_after.direction, worker_before.direction));
    assert(registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state ==
           bench_before);
    assert(buildingImproved(registry) == building_improved_before);
    assert(registry.view<PathComponent>().size() == path_count_before);
}

static void testPublicSitePlacementAndInspection() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;

    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    assert(validateWorld(registry, config));
    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    assert(clinic != MAX_ENTITIES);
    assert(registry.get<BuildingUseComponent>(clinic).role == MicroZoneRole::CLINIC);
    assert(!registry.get<BuildingComponent>(clinic).is_enterable);
    assert(registry.has<ClinicAccessLedgerComponent>(clinic));
    assert(!registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);
    assert(!registry.has<ShelterStockComponent>(clinic));
    assert(!registry.has<WorkplaceBenchComponent>(clinic));
    assert(!registry.has<BuildingImprovementComponent>(clinic));

    const auto& glyph = registry.get<GlyphComponent>(clinic);
    assert(glyph.chars == "+");
    assert(glyph.r == 255);
    assert(glyph.g == 90);
    assert(glyph.b == 90);

    size_t clinic_micros = 0;
    for (Entity micro : registry.view<MicroZoneComponent>()) {
        if (registry.get<MicroZoneComponent>(micro).role == MicroZoneRole::CLINIC) {
            ++clinic_micros;
        }
    }
    assert(clinic_micros == 1);
    assert(buildingsDoNotOverlap(registry));

    assert(inspectionTypeForRole(MicroZoneRole::CLINIC) == InspectionTargetType::CLINIC);
    assert(nearestInteractableBuildingInRange(registry,
        registry.get<TransformComponent>(clinic), 22.0f) == MAX_ENTITIES);

    const std::string readout = buildingInspectionReadout(registry, clinic);
    assert(readout.find("CLINIC; PURPOSE: PUBLIC HEALTH") != std::string::npos);
    assert(readout.find("FUNCTION: MEDICAL SERVICE") != std::string::npos);
    assert(readout.find("SITE STATUS: OBSERVATION ONLY") != std::string::npos);
    assert(readout.find("CONTEXT: MUNICIPAL") != std::string::npos);
}

static void testSiteContextTagsInInspectionReadout() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    buildWorld(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);

    assert(buildingInspectionReadout(registry, housing).find("CONTEXT: HOUSEHOLD") != std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("CONTEXT: PRIVATE") != std::string::npos);
    assert(buildingInspectionReadout(registry, supply).find("CONTEXT: PRIVATE") != std::string::npos);
    assert(buildingInspectionReadout(registry, market).find("CONTEXT: COMMERCIAL") != std::string::npos);
    assert(buildingInspectionReadout(registry, clinic).find("CONTEXT: MUNICIPAL") != std::string::npos);

    assert(std::string(siteContextTagForRole(MicroZoneRole::HOUSING)) == "HOUSEHOLD");
    assert(std::string(siteContextTagForRole(MicroZoneRole::WORKPLACE)) == "PRIVATE");
    assert(std::string(siteContextTagForRole(MicroZoneRole::SUPPLY)) == "PRIVATE");
    assert(std::string(siteContextTagForRole(MicroZoneRole::MARKET)) == "COMMERCIAL");
    assert(std::string(siteContextTagForRole(MicroZoneRole::CLINIC)) == "MUNICIPAL");
}

static void testPublicSiteDebuggerScanMetadata() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    buildWorld(registry, config);

    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    assert(clinic != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(clinic));
    registry.assign<BuildingInteractionComponent>(player);

    assert(playerInspectionTarget(registry, player, 22.0f).type == InspectionTargetType::CLINIC);
    assert(inheritedGadgetPromptReadout(registry, player, 22.0f) ==
           "SPACE DEBUGGER ON CLINIC");
    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "DEBUGGER RESULT: CLINIC PURPOSE: PUBLIC HEALTH; SERVICE: RATIONED; AUTHORITY: MUNICIPAL");
}

static void testPublicSiteBoundaryDoesNotBreakLoop() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    Entity worker = firstFixedWorker(registry);
    assert(clinic != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);

    assert(firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::CLINIC) ==
           MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::CLINIC) ==
           MAX_ENTITIES);
    assert(firstPathBetweenRoles(registry, MicroZoneRole::SUPPLY, MicroZoneRole::CLINIC) ==
           MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(clinic));
    registry.assign<BuildingInteractionComponent>(player);

    const auto worker_before = registry.get<FixedActorComponent>(worker);
    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state;
    const bool building_improved_before = buildingImproved(registry);
    const size_t path_count_before = registry.view<PathComponent>().size();

    assert(!enterBuildingInterior(registry, player, clinic));
    assert(playerLocationState(registry, player, 22.0f) == PlayerLocationState::OUTSIDE);
    assert(!playerCanTakeSupplyObject(registry, player));
    assert(!takeSupplyObjectFromInterior(registry, player));
    assert(!playerCanStoreSupplyAtShelter(registry, player));
    assert(!playerCanStockWorkplaceBench(registry, player));
    assert(!playerCanWorkWorkplaceBench(registry, player));
    assert(!playerCanTakeWorkplaceOutput(registry, player));
    assert(!playerCanImproveBuilding(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);

    assert(updateWorkerSupplyPickups(registry) == 0);
    assert(updateWorkerWorkplaceBenchDropOffs(registry) == 0);
    assert(updateWorkerWorkplaceBenchWork(registry) == 0);
    assert(updateWorkerWorkplaceOutputPickups(registry) == 0);
    assert(updateWorkerBuildingDeliveries(registry) == 0);

    const auto& worker_after = registry.get<FixedActorComponent>(worker);
    assert(worker_after.path_entity == worker_before.path_entity);
    assert(worker_after.carried_object == worker_before.carried_object);
    assert(closeTo(worker_after.route_t, worker_before.route_t));
    assert(closeTo(worker_after.direction, worker_before.direction));
    assert(registry.get<WorkplaceBenchComponent>(firstWorkplaceBenchBuilding(registry)).state ==
           bench_before);
    assert(buildingImproved(registry) == building_improved_before);
    assert(registry.view<PathComponent>().size() == path_count_before);

    assert(std::string(siteContextTagForRole(MicroZoneRole::CLINIC)) == "MUNICIPAL");
    assert(!roleIsEnterable(MicroZoneRole::CLINIC));
}

static void testBuildingPurposeReadoutsDoNotMutateLoopState() {
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
    Entity object = firstCarryableObject(registry);
    assert(worker != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    const auto worker_before = registry.get<FixedActorComponent>(worker);
    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(workplace).state;
    const bool building_improved_before = buildingImproved(registry);
    const TransformComponent object_before = registry.get<TransformComponent>(object);

    assert(!buildingInspectionReadout(registry, housing).empty());
    assert(!buildingInspectionReadout(registry, workplace).empty());
    assert(!buildingInspectionReadout(registry, supply).empty());
    assert(!housingInteriorReadout(registry).empty());
    assert(!workplaceBenchLoopReadout(registry).empty());

    const auto& worker_after = registry.get<FixedActorComponent>(worker);
    assert(worker_after.path_entity == worker_before.path_entity);
    assert(worker_after.carried_object == worker_before.carried_object);
    assert(closeTo(worker_after.route_t, worker_before.route_t));
    assert(closeTo(worker_after.direction, worker_before.direction));
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == bench_before);
    assert(buildingImproved(registry) == building_improved_before);
    assert(closeTo(registry.get<TransformComponent>(object).x, object_before.x));
    assert(closeTo(registry.get<TransformComponent>(object).y, object_before.y));
}

static void testDependencyEdgeInspectionReadouts() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    assert(housing != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(clinic != MAX_ENTITIES);

    assert(dependencyEdgeResolved(registry));
    assert(dependencyEndpointForRole(registry, MicroZoneRole::WORKPLACE) == workplace);
    assert(dependencyEndpointForRole(registry, MicroZoneRole::SUPPLY) == supply);
    assert(dependencyEndpointForRole(registry, MicroZoneRole::HOUSING) == MAX_ENTITIES);

    const std::string workplace_readout = buildingInspectionReadout(registry, workplace);
    const std::string supply_readout = buildingInspectionReadout(registry, supply);
    assert(workplace_readout.find("DEPENDS ON: SUPPLY") != std::string::npos);
    assert(supply_readout.find("SUPPORTS: WORKPLACE") != std::string::npos);
    assert(buildingInspectionReadout(registry, housing).find("DEPENDS ON") == std::string::npos);
    assert(buildingInspectionReadout(registry, clinic).find("DEPENDS ON") == std::string::npos);
    assert(buildingInspectionReadout(registry, clinic).find("SUPPORTS") == std::string::npos);
}

static void testDependencyEdgeMissingTargetReadouts() {
    Registry missing_supply;
    WorldConfig workplace_only = makeSandboxConfig();
    workplace_only.workplace_micro_zone_count = 1;
    workplace_only.workplace_building_count = 1;
    buildWorld(missing_supply, workplace_only);

    Entity workplace = firstBuildingByRole(missing_supply, MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);
    assert(!dependencyEdgeResolved(missing_supply));
    assert(buildingInspectionReadout(missing_supply, workplace).find("DEPENDS ON: MISSING SUPPLY") !=
           std::string::npos);
    assert(dependencyScanReadout(missing_supply, MicroZoneRole::WORKPLACE) ==
           "FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; TARGET: MISSING");

    Registry missing_workplace;
    WorldConfig supply_only = makeSandboxConfig();
    supply_only.supply_micro_zone_count = 1;
    supply_only.supply_building_count = 1;
    buildWorld(missing_workplace, supply_only);

    Entity supply = firstBuildingByRole(missing_workplace, MicroZoneRole::SUPPLY);
    assert(supply != MAX_ENTITIES);
    assert(!dependencyEdgeResolved(missing_workplace));
    assert(buildingInspectionReadout(missing_workplace, supply).find("SUPPORTS: NO WORKPLACE") !=
           std::string::npos);
    assert(dependencyScanReadout(missing_workplace, MicroZoneRole::SUPPLY) ==
           "FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; TARGET: MISSING");
}

static void testDependencyDebuggerScanEnrichment() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);

    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{workplace, InspectionTargetType::WORKPLACE}) ==
           "WORKPLACE PURPOSE: PRODUCTION; FUNCTION: CONVERT SUPPLY TO PART; FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; SOURCE: SUPPLY");
    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{supply, InspectionTargetType::SUPPLY}) ==
           "SUPPLY PURPOSE: MATERIAL SOURCE; FUNCTION: SOURCE LOOP MATERIAL; FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; SUPPORTS: WORKPLACE");
    assert(inheritedGadgetSiteMetadataScan(
        registry,
        InspectionTarget{firstBuildingByRole(registry, MicroZoneRole::HOUSING), InspectionTargetType::HOUSING}) ==
           "HOUSING PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY");

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    assert(enterBuildingInterior(registry, player, workplace));

    assert(inheritedGadgetPromptReadout(registry, player, 22.0f) ==
           "SPACE DEBUGGER ON WORKPLACE INTERIOR");
    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "DEBUGGER RESULT: WORKPLACE PURPOSE: PRODUCTION; FUNCTION: CONVERT SUPPLY TO PART; FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; SOURCE: SUPPLY");
}

static void testDependencyReadoutsDoNotMutateLoopState() {
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
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    Entity object = firstCarryableObject(registry);
    assert(worker != MAX_ENTITIES);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    const auto worker_before = registry.get<FixedActorComponent>(worker);
    const WorkplaceBenchState bench_before =
        registry.get<WorkplaceBenchComponent>(workplace).state;
    const bool building_improved_before = buildingImproved(registry);
    const TransformComponent object_before = registry.get<TransformComponent>(object);

    assert(!dependencyInspectionReadout(registry, MicroZoneRole::WORKPLACE).empty());
    assert(!dependencyInspectionReadout(registry, MicroZoneRole::SUPPLY).empty());
    assert(!dependencyScanReadout(registry, MicroZoneRole::WORKPLACE).empty());
    assert(!dependencyScanReadout(registry, MicroZoneRole::SUPPLY).empty());
    assert(!buildingInspectionReadout(registry, workplace).empty());
    assert(!buildingInspectionReadout(registry, supply).empty());

    const auto& worker_after = registry.get<FixedActorComponent>(worker);
    assert(worker_after.path_entity == worker_before.path_entity);
    assert(worker_after.carried_object == worker_before.carried_object);
    assert(closeTo(worker_after.route_t, worker_before.route_t));
    assert(closeTo(worker_after.direction, worker_before.direction));
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == bench_before);
    assert(buildingImproved(registry) == building_improved_before);
    assert(closeTo(registry.get<TransformComponent>(object).x, object_before.x));
    assert(closeTo(registry.get<TransformComponent>(object).y, object_before.y));
}

static void testDependencyDisruptionViaDebuggerTogglesReadouts() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    assert(enterBuildingInterior(registry, player, workplace));

    assert(!dependencyDisrupted(registry));
    assert(inheritedGadgetCanSpoofTarget(registry, playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH DISRUPT DEPENDENCY");
    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(dependencyDisrupted(registry));
    assert(!dependencyRecovered(registry));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: DISRUPTED DEPENDENCY: SUPPLY FLOW CONFUSED");
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH RESTORE DEPENDENCY");

    assert(buildingInspectionReadout(registry, workplace).find("DEPENDENCY: DISRUPTED") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, supply).find("DEPENDENCY: DISRUPTED") !=
           std::string::npos);
    assert(dependencyScanReadout(registry, MicroZoneRole::WORKPLACE).find("FLOW STATUS: CONFUSED") !=
           std::string::npos);
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: DISRUPTED; INTERFERENCE: DEPENDENCY; CONSEQUENCE: SUPPLY FLOW CONFUSED");

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!dependencyDisrupted(registry));
    assert(dependencyRecovered(registry));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: RESTORED DEPENDENCY: SUPPLY FLOW CLEAR");
    assert(buildingInspectionReadout(registry, workplace).find("DEPENDENCY: RESTORED") !=
           std::string::npos);
    assert(dependencyScanReadout(registry, MicroZoneRole::WORKPLACE).find("FLOW STATUS: CLEAR") !=
           std::string::npos);
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");
}

static void testDependencyDisruptionPausesAndRestoresWorkerSupplyFlow() {
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

    assert(toggleDependencyDisruption(registry));
    const float disrupted_t = worker_component.route_t;
    assert(workerCurrentPathHasDisruptedDependency(registry, worker));
    assert(workerBlockedReason(registry, worker) == "DEPENDENCY DISRUPTED");
    assert(workerCarryReadout(registry, worker).find("SOURCE: DISRUPTED DEPENDENCY") !=
           std::string::npos);
    assert(workerCarryReadout(registry, worker).find("WAITING ON SUPPLY FLOW") !=
           std::string::npos);
    assert(!takeSupplyObjectForWorker(registry, worker));
    updateFixedActors(registry, 10.0f);
    assert(closeTo(worker_component.route_t, disrupted_t));

    assert(toggleDependencyDisruption(registry));
    assert(!workerCurrentPathHasDisruptedDependency(registry, worker));
    assert(workerBlockedReason(registry, worker).empty());
    assert(takeSupplyObjectForWorker(registry, worker));
    assert(workerCarryingSupplyObject(registry, worker));
}

static void testDependencyDisruptionBoundariesDoNotSpoofRoutesOrUnrelatedSites() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    Entity market = firstBuildingByRole(registry, MicroZoneRole::MARKET);
    Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    assert(workplace != MAX_ENTITIES);
    assert(market != MAX_ENTITIES);
    assert(clinic != MAX_ENTITIES);
    assert(!anyRouteSignpostSpoofed(registry));

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    assert(enterBuildingInterior(registry, player, workplace));

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(dependencyDisrupted(registry));
    assert(!anyRouteSignpostSpoofed(registry));
    assert(buildingInspectionReadout(registry, market).find("DEPENDENCY: DISRUPTED") ==
           std::string::npos);
    assert(buildingInspectionReadout(registry, clinic).find("DEPENDENCY: DISRUPTED") ==
           std::string::npos);

    exitBuildingInterior(registry, player);
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(clinic);
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH:N/A");
    assert(!useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(dependencyDisrupted(registry));
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
           "PURPOSE: DWELLING; FUNCTION: BUILDING RECOVERY; BUILDING SUPPLY: 0/1; BUILDING: ALREADY COMPLETE; BUILDING IMPROVED: YES");
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
    assert(inheritedGadgetReadout(registry, player) == "DEBUGGER:MOTHER'S DEBUGGER READY");
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: IDLE");

    assert(playerCanTakeNearbyCarryableObject(registry, player, 18.0f));
    assert(takeNearbyCarryableObject(registry, player, 18.0f));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(carryableObjectIsKind(registry, object, ItemKind::SUPPLY));
    assert(inheritedGadgetReadout(registry, player) == "DEBUGGER:MOTHER'S DEBUGGER READY");
}

static void testInheritedGadgetPromptAndNoSignalBehavior() {
    Registry registry;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);

    assert(!playerCanUseInheritedGadget(registry, player));
    assert(inheritedGadgetPromptReadout(registry, player, 18.0f) == "DEBUGGER:NONE");

    registry.assign<InheritedGadgetComponent>(player);

    assert(playerCanUseInheritedGadget(registry, player));
    assert(inheritedGadgetPromptReadout(registry, player, 18.0f) ==
           "SPACE DEBUGGER: NO SIGNAL");
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: IDLE");
    assert(!useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: NO SIGNAL");
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
           "SPACE DEBUGGER ON WORKER");
    assert(useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "DEBUGGER RESULT: WORKER SIGNAL: DEBT WORK; PAY DOCKED IF STALLED; ROUTE QUOTA: 1");

    registry.get<TransformComponent>(player) = TransformComponent{4000.0f, 4000.0f, 12.0f, 12.0f};
    assert(!useInheritedGadget(registry, player, 18.0f));
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: NO SIGNAL");
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
           "DEBUGGER RESULT: WORKER SIGNAL: DEBT WORK; PAY DOCKED IF STALLED; ROUTE QUOTA: 1");
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
           "DEBUGGER RESULT: WORKPLACE PURPOSE: PRODUCTION; FUNCTION: CONVERT SUPPLY TO PART; FLOW: MATERIAL; REQUIRED FOR: BENCH STOCK; SOURCE: SUPPLY");

    exitBuildingInterior(registry, player);
    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    assert(!signposts.empty());
    Entity marker = signposts.front();
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(marker);

    assert(useInheritedGadget(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player).find("SIGNPOST ROUTE:") !=
           std::string::npos);
    assert(inheritedGadgetResultReadout(registry, player).find("EXPECTED CARGO:") !=
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
    assert(inheritedGadgetResultReadout(registry, player) == "DEBUGGER RESULT: NO SIGNAL");
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

    assert(!inheritedGadgetCanSpoofTarget(registry, playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH:N/A");
    assert(!useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: FAILED: SIGNPOST OR DEPENDENCY REQUIRED");
    assert(!anyRouteSignpostSpoofed(registry));

    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(marker);

    assert(inheritedGadgetCanSpoofTarget(registry, playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH SPOOF SIGNPOST");
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
           "INTERFERENCE TORCH RESULT: SPOOFED SIGNPOST: ROUTE SIGNAL CONFUSED");
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH RESTORE SIGNPOST");
    assert(productionLoopSummaryReadout(registry) ==
           "LOOP: SPOOFED; INTERFERENCE: ROUTE; CONSEQUENCE: ROUTE SIGNAL CONFUSED");

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!routeSignpostSpoofed(registry, marker));
    assert(routeSignpostReadout(registry, marker).find("SPOOFED") == std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("RECOVERY: ROUTE SIGNAL CLEAR") !=
           std::string::npos);
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: RESTORED SIGNPOST: ROUTE SIGNAL CLEAR");
    assert(productionLoopSummaryReadout(registry) == "LOOP: RUNNING");
}

static void setupWitnessedMissingPartSuspicion(Registry& registry,
                                               Entity& player,
                                               Entity& worker,
                                               Entity& workplace,
                                               Entity& object) {
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

    workplace = firstWorkplaceBenchBuilding(registry);
    worker = firstFixedWorker(registry);
    object = firstCarryableObject(registry);
    const Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(takeWorkplaceOutput(registry, player));
    assert(localSuspicionActive(registry));
}

static void setupClinicAccessLedgerSuspicion(Registry& registry,
                                             Entity& player,
                                             Entity& worker,
                                             Entity& clinic) {
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    config.fixed_worker_count = 1;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    const Entity workplace = firstWorkplaceBenchBuilding(registry);
    worker = firstFixedWorker(registry);
    const Entity object = firstCarryableObject(registry);
    clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    const Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(clinic != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(takeWorkplaceOutput(registry, player));
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(clinic);
    assert(localSuspicionActive(registry));
}

static void setupWitnessedRouteSuspicion(Registry& registry,
                                         Entity& player,
                                         Entity& worker,
                                         Entity& signpost,
                                         Entity& route_path,
                                         Entity& workplace) {
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);

    worker = firstFixedWorker(registry);
    assert(worker != MAX_ENTITIES);
    signpost = firstRouteSignpostForPath(registry,
        registry.get<FixedActorComponent>(worker).path_entity);
    assert(signpost != MAX_ENTITIES);
    route_path = registry.get<RouteSignpostComponent>(signpost).path_entity;
    workplace = pathEndpointWithRole(registry, route_path, MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);

    registry.get<TransformComponent>(worker) = registry.get<TransformComponent>(signpost);
    registry.get<TransformComponent>(worker).x += 24.0f;

    player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(signpost));

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostSpoofed(registry, signpost));
    assert(localSuspicionActive(registry));
}

static void testWitnessedOutputTheftCreatesLocalSuspicion() {
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

    Entity workplace = firstWorkplaceBenchBuilding(registry);
    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    Entity workplace_supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(workplace_supply_path != MAX_ENTITIES);

    auto& worker_component = registry.get<FixedActorComponent>(worker);
    worker_component.path_entity = workplace_supply_path;
    worker_component.route_t = routeTForPathEndpoint(registry, workplace_supply_path, workplace);
    worker_component.direction = 0.0f;
    registry.get<TransformComponent>(worker) =
        transformOnPath(registry.get<TransformComponent>(workplace_supply_path),
                        worker_component.route_t);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(workerCanTakeWorkplaceOutput(registry, worker));
    assert(takeWorkplaceOutput(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(registry.get<CarryableComponent>(object).kind == ItemKind::PART);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::EMPTY);
    assert(localSuspicionActive(registry));
    assert(registry.has<LocalSuspicionComponent>(worker));
    const auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
    assert(suspicion.active);
    assert(suspicion.cause == LocalSuspicionCause::MISSING_PART);
    assert(suspicion.workplace_entity == workplace);
    assert(suspicion.target_entity == object);
    assert(suspicion.path_entity == MAX_ENTITIES);
    assert(localSuspicionHudReadout(registry) ==
           "LOCAL NOTICE: WORKER SAW MISSING PART");

    const std::string worker_readout = workerCarryReadout(registry, worker);
    assert(worker_readout.find("SUSPICION: MISSING PART") != std::string::npos);
    const std::string workplace_readout = buildingInspectionReadout(registry, workplace);
    assert(workplace_readout.find("SUSPICION: MISSING PART") != std::string::npos);

    const std::string worker_scan =
        inheritedGadgetScanResult(registry, InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(worker_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(worker_scan.find("SUSPICION: MISSING PART") != std::string::npos);
    assert(worker_scan.find("TARGET: WORKPLACE OUTPUT") != std::string::npos);
    const std::string workplace_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{workplace, InspectionTargetType::WORKPLACE});
    assert(workplace_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(workplace_scan.find("SUSPICION: MISSING PART") != std::string::npos);
    assert(workplace_scan.find("TARGET: WORKPLACE OUTPUT") != std::string::npos);
}

static void testUnwitnessedOutputPickupDoesNotCreateLocalSuspicion() {
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

    Entity workplace = firstWorkplaceBenchBuilding(registry);
    Entity worker = firstFixedWorker(registry);
    Entity object = firstCarryableObject(registry);
    assert(workplace != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    registry.get<TransformComponent>(worker) =
        TransformComponent{4000.0f, 4000.0f, 12.0f, 12.0f};
    registry.get<FixedActorComponent>(worker).route_t = 0.5f;
    registry.get<FixedActorComponent>(worker).direction = 0.0f;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    registry.assign<BuildingInteractionComponent>(player);
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
    assert(!workerCanTakeWorkplaceOutput(registry, worker));
    assert(takeWorkplaceOutput(registry, player));

    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(registry.get<CarryableComponent>(object).kind == ItemKind::PART);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state == WorkplaceBenchState::EMPTY);
    assert(!localSuspicionActive(registry));
    assert(localSuspicionHudReadout(registry).empty());
    assert(registry.view<LocalSuspicionComponent>().empty());
    assert(workerCarryReadout(registry, worker).find("SUSPICION:") == std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("SUSPICION:") ==
           std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{worker, InspectionTargetType::WORKER})
               .find("SUSPICION:") == std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{workplace, InspectionTargetType::WORKPLACE})
               .find("SUSPICION:") == std::string::npos);
}

static void testWitnessedRouteTamperingCreatesLocalSuspicionAndRestoreKeepsIt() {
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
    Entity signpost = firstRouteSignpostForPath(registry,
        registry.get<FixedActorComponent>(worker).path_entity);
    assert(signpost != MAX_ENTITIES);

    registry.get<TransformComponent>(worker) = registry.get<TransformComponent>(signpost);
    registry.get<TransformComponent>(worker).x += 24.0f;

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(signpost));

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostSpoofed(registry, signpost));
    assert(localSuspicionActive(registry));
    assert(registry.has<LocalSuspicionComponent>(worker));
    const auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
    assert(suspicion.active);
    assert(suspicion.cause == LocalSuspicionCause::ROUTE_TAMPERING);
    assert(suspicion.target_entity == signpost);
    const Entity route_path = registry.get<RouteSignpostComponent>(signpost).path_entity;
    const Entity workplace = pathEndpointWithRole(registry,
                                                  route_path,
                                                  MicroZoneRole::WORKPLACE);
    assert(workplace != MAX_ENTITIES);
    assert(suspicion.path_entity == route_path);
    assert(localSuspicionHudReadout(registry) ==
           "LOCAL NOTICE: WORKER SAW ROUTE TAMPERING");
    assert(workerCarryReadout(registry, worker).find("SUSPICION: ROUTE TAMPERING") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("SUSPICION: ROUTE TAMPERING") !=
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!routeSignpostSpoofed(registry, signpost));
    assert(!localSuspicionActive(registry));
    assert(registry.get<LocalSuspicionComponent>(worker).cause ==
           LocalSuspicionCause::ROUTE_TAMPERING);
    assert(registry.get<LocalSuspicionComponent>(worker).resolution ==
           LocalSuspicionResolution::CORRECTED_ROUTE);

    const std::string worker_scan =
        inheritedGadgetScanResult(registry, InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(worker_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(worker_scan.find("DE-ESCALATED: CORRECTED ROUTE TAMPERING") != std::string::npos);
    assert(worker_scan.find("TARGET: ROUTE SIGNAL") != std::string::npos);
    const std::string workplace_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{workplace, InspectionTargetType::WORKPLACE});
    assert(workplace_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(workplace_scan.find("DE-ESCALATED: CORRECTED ROUTE TAMPERING") !=
           std::string::npos);
    const std::string signpost_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{signpost, InspectionTargetType::ROUTE_SIGNPOST});
    assert(signpost_scan.find("FLOW: BLOCKED") == std::string::npos);
    assert(signpost_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(signpost_scan.find("DE-ESCALATED: CORRECTED ROUTE TAMPERING") !=
           std::string::npos);
    const std::string path_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{route_path, InspectionTargetType::PEDESTRIAN_PATH});
    assert(path_scan.find("FLOW: BLOCKED") == std::string::npos);
    assert(path_scan.find("LOCAL WITNESS: WORKER") != std::string::npos);
    assert(path_scan.find("DE-ESCALATED: CORRECTED ROUTE TAMPERING") !=
           std::string::npos);
}

static void testUnwitnessedRouteTamperingDoesNotCreateLocalSuspicion() {
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
    Entity signpost = firstRouteSignpostForPath(registry,
        registry.get<FixedActorComponent>(worker).path_entity);
    assert(signpost != MAX_ENTITIES);

    registry.get<TransformComponent>(worker) =
        TransformComponent{4000.0f, 4000.0f, 12.0f, 12.0f};

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(signpost));

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeSignpostSpoofed(registry, signpost));
    assert(!localSuspicionActive(registry));
    assert(localSuspicionHudReadout(registry).empty());
    assert(registry.view<LocalSuspicionComponent>().empty());
    assert(!dependencyDisrupted(registry));
    assert(workerCarryReadout(registry, worker).find("SUSPICION:") == std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{worker, InspectionTargetType::WORKER})
               .find("SUSPICION:") == std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{signpost, InspectionTargetType::ROUTE_SIGNPOST})
               .find("SUSPICION:") == std::string::npos);
    assert(inheritedGadgetScanResult(
               registry,
               InspectionTarget{registry.get<RouteSignpostComponent>(signpost).path_entity,
                                InspectionTargetType::PEDESTRIAN_PATH})
               .find("SUSPICION:") == std::string::npos);
}

static void testReturningSuspiciousOutputDeEscalatesLocalConcern() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);

    assert(playerCanReturnSuspiciousWorkplaceOutput(registry, player));
    assert(returnSuspiciousWorkplaceOutput(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(registry.get<WorkplaceBenchComponent>(workplace).state ==
           WorkplaceBenchState::OUTPUT_READY);
    assert(!localSuspicionActive(registry));
    assert(workerCarryReadout(registry, worker).find("SUSPICION QUIET: RETURNED MISSING PART") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace)
               .find("SUSPICION QUIET: RETURNED MISSING PART") != std::string::npos);

    Registry delivery_registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.carryable_object_count = 1;
    buildWorld(delivery_registry, config);
    deriveInfrastructure(delivery_registry, config);
    Entity delivery_player = delivery_registry.create();
    delivery_registry.assign<PlayerComponent>(delivery_player);
    delivery_registry.assign<BuildingInteractionComponent>(delivery_player);
    Entity delivery_object = firstCarryableObject(delivery_registry);
    Entity housing = firstBuildingByRole(delivery_registry, MicroZoneRole::HOUSING);
    assert(delivery_object != MAX_ENTITIES);
    assert(housing != MAX_ENTITIES);
    delivery_registry.get<CarryableComponent>(delivery_object).kind = ItemKind::PART;
    hideCarryableObject(delivery_registry, delivery_object);
    delivery_registry.get<PlayerComponent>(delivery_player).carried_object = delivery_object;
    delivery_registry.assign<TransformComponent>(delivery_player,
                                                 delivery_registry.get<TransformComponent>(housing));
    assert(enterBuildingInterior(delivery_registry, delivery_player, housing));
    assert(playerCanImproveBuilding(delivery_registry, delivery_player));
    assert(improveBuilding(delivery_registry, delivery_player));
}

static void testHidingSuspiciousItemInHousingKeepsInspectableConcern() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));

    assert(playerCanHideSuspiciousItemInHousing(registry, player));
    assert(hideSuspiciousItemInHousing(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == MAX_ENTITIES);
    assert(!localSuspicionActive(registry));
    assert(workerCarryReadout(registry, worker).find("SUSPICION HIDDEN: MISSING PART") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, housing).find("SUSPICION HIDDEN: MISSING PART") !=
           std::string::npos);

    const std::string housing_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{housing, InspectionTargetType::HOUSING});
    assert(housing_scan.find("DE-ESCALATED: HIDDEN MISSING PART") != std::string::npos);
}

static void testCorrectingSuspiciousRouteDeEscalatesOnlyMatchingSignpost() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity signpost = MAX_ENTITIES;
    Entity route_path = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    setupWitnessedRouteSuspicion(registry, player, worker, signpost, route_path, workplace);

    Entity unrelated = MAX_ENTITIES;
    auto signposts = registry.view<RouteSignpostComponent, TransformComponent>();
    for (Entity marker : signposts) {
        if (marker != signpost) {
            unrelated = marker;
            break;
        }
    }
    assert(unrelated != MAX_ENTITIES);
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(unrelated);
    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(localSuspicionActive(registry));
    assert(workerCarryReadout(registry, worker).find("SUSPICION: ROUTE TAMPERING") !=
           std::string::npos);

    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(signpost);
    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!routeSignpostSpoofed(registry, signpost));
    assert(!localSuspicionActive(registry));
    assert(workerCarryReadout(registry, worker)
               .find("SUSPICION QUIET: CORRECTED ROUTE TAMPERING") != std::string::npos);
    assert(routeSignpostReadout(registry, signpost).find("FLOW: CLEAR") != std::string::npos);
}

static void testTinySaveRoundTripRestoresLocalSuspicionStates() {
    Registry active_registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(active_registry, player, worker, workplace, object);

    const std::string active_save =
        serializeTinySaveState(captureTinySaveState(active_registry, player));
    assert(active_save.find("LOCAL_SUSPICION 0 1 1 MISSING_PART NONE") !=
           std::string::npos);

    TinySaveState active_state;
    assert(deserializeTinySaveState(active_save, active_state) == TinySaveStatus::OK);
    active_registry.remove<LocalSuspicionComponent>(worker);
    assert(applyTinySaveState(active_registry, player, active_state) == TinySaveStatus::OK);
    assert(localSuspicionActive(active_registry));
    assert(workerCarryReadout(active_registry, worker).find("SUSPICION: MISSING PART") !=
           std::string::npos);

    Registry hidden_registry;
    Entity hidden_player = MAX_ENTITIES;
    Entity hidden_worker = MAX_ENTITIES;
    Entity hidden_workplace = MAX_ENTITIES;
    Entity hidden_object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(hidden_registry,
                                       hidden_player,
                                       hidden_worker,
                                       hidden_workplace,
                                       hidden_object);
    Entity housing = firstBuildingByRole(hidden_registry, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    assert(exitBuildingInterior(hidden_registry, hidden_player));
    hidden_registry.get<TransformComponent>(hidden_player) =
        hidden_registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(hidden_registry, hidden_player, housing));
    assert(hideSuspiciousItemInHousing(hidden_registry, hidden_player));
    const std::string hidden_save =
        serializeTinySaveState(captureTinySaveState(hidden_registry, hidden_player));
    assert(hidden_save.find("LOCAL_SUSPICION 0 1 0 MISSING_PART HIDDEN_ITEM") !=
           std::string::npos);
    TinySaveState hidden_state;
    assert(deserializeTinySaveState(hidden_save, hidden_state) == TinySaveStatus::OK);
    hidden_registry.remove<LocalSuspicionComponent>(hidden_worker);
    assert(applyTinySaveState(hidden_registry, hidden_player, hidden_state) == TinySaveStatus::OK);
    assert(!localSuspicionActive(hidden_registry));
    assert(buildingInspectionReadout(hidden_registry, housing).find("SUSPICION HIDDEN") !=
           std::string::npos);

    Registry clear_registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.fixed_worker_count = 1;
    buildWorld(clear_registry, config);
    deriveInfrastructure(clear_registry, config);
    spawnFixedActors(clear_registry, config);
    Entity clear_player = clear_registry.create();
    clear_registry.assign<PlayerComponent>(clear_player);
    clear_registry.assign<BuildingInteractionComponent>(clear_player);
    clear_registry.assign<TransformComponent>(clear_player, TransformComponent{0, 0, 12, 12});
    Entity clear_worker = firstFixedWorker(clear_registry);
    assert(clear_worker != MAX_ENTITIES);
    clear_registry.assign<LocalSuspicionComponent>(clear_worker).active = true;
    const std::string clear_save =
        serializeTinySaveState(captureTinySaveState(clear_registry, clear_player));
    TinySaveState clear_state;
    assert(deserializeTinySaveState(clear_save, clear_state) == TinySaveStatus::OK);
    assert(applyTinySaveState(clear_registry, clear_player, clear_state) == TinySaveStatus::OK);
    assert(!localSuspicionActive(clear_registry));
}

static void testInstitutionalLogFragmentRecoveryAddsAuditTrace() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);
    registry.assign<InheritedGadgetComponent>(player);

    assert(useInheritedGadget(registry, player, 22.0f));
    const std::string result = inheritedGadgetResultReadout(registry, player);
    assert(result.find("WORKPLACE LOG: OUTPUT ANOMALY FILED") != std::string::npos);
    assert(result.find("AUDIT TRACE: LOCAL ONLY") != std::string::npos);
    assert(workerCarryReadout(registry, worker).find("AUDIT TRACE: LOCAL ONLY") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("AUDIT TRACE: LOCAL ONLY") !=
           std::string::npos);

    Registry clear_registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    buildWorld(clear_registry, config);
    deriveInfrastructure(clear_registry, config);
    Entity clear_workplace = firstWorkplaceBenchBuilding(clear_registry);
    Entity clear_player = clear_registry.create();
    clear_registry.assign<PlayerComponent>(clear_player);
    clear_registry.assign<InheritedGadgetComponent>(clear_player);
    clear_registry.assign<TransformComponent>(clear_player,
                                              clear_registry.get<TransformComponent>(clear_workplace));
    clear_registry.assign<BuildingInteractionComponent>(clear_player);
    assert(enterBuildingInterior(clear_registry, clear_player, clear_workplace));
    assert(useInheritedGadget(clear_registry, clear_player, 22.0f));
    assert(inheritedGadgetResultReadout(clear_registry, clear_player)
               .find("WORKPLACE LOG:") == std::string::npos);
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

static void testRoutePurposeReadoutsAndDebuggerScanEnrichment() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity labor_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(labor_path != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);

    RoutePurposeInfo labor = routePurposeForPath(registry, labor_path);
    assert(std::string(labor.purpose) == "LABOR ROUTE");
    assert(std::string(labor.carries) == "LABOR");
    assert(std::string(labor.expected_cargo) == "WORKER");
    assert(std::string(labor.access) == "WORKER ROUTE");

    RoutePurposeInfo supply = routePurposeForPath(registry, supply_path);
    assert(std::string(supply.purpose) == "SUPPLY ROUTE");
    assert(std::string(supply.carries) == "MATERIAL");
    assert(std::string(supply.expected_cargo) == "SUPPLY/PART");
    assert(std::string(supply.access) == "WORKER ONLY");

    InspectionTarget path_target;
    path_target.entity = supply_path;
    path_target.type = InspectionTargetType::PEDESTRIAN_PATH;
    const std::string path_scan = inheritedGadgetScanResult(registry, path_target);
    assert(path_scan.find("PATH ROUTE: SUPPLY ROUTE") != std::string::npos);
    assert(path_scan.find("EXPECTED CARGO: SUPPLY/PART") != std::string::npos);
    assert(path_scan.find("ACCESS: WORKER ONLY") != std::string::npos);

    const Entity signpost = firstRouteSignpostForPath(registry, supply_path);
    assert(signpost != MAX_ENTITIES);
    InspectionTarget signpost_target;
    signpost_target.entity = signpost;
    signpost_target.type = InspectionTargetType::ROUTE_SIGNPOST;
    const std::string signpost_scan = inheritedGadgetScanResult(registry, signpost_target);
    assert(signpost_scan.find("SIGNPOST ROUTE: SUPPLY ROUTE") != std::string::npos);
    assert(signpost_scan.find("EXPECTED CARGO: SUPPLY/PART") != std::string::npos);
    assert(signpost_scan.find("ACCESS: WORKER ONLY") != std::string::npos);
    assert(signpost_scan.find("POINTS TO ") != std::string::npos);
}

static void testSpoofedRouteSignpostCorruptsFlowLabel() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(supply_path != MAX_ENTITIES);
    const Entity signpost = firstRouteSignpostForPath(registry, supply_path);
    assert(signpost != MAX_ENTITIES);

    std::string readout = routeSignpostReadout(registry, signpost);
    assert(readout.find("ROUTE: SUPPLY ROUTE") != std::string::npos);
    assert(readout.find("CARRIES: MATERIAL") != std::string::npos);

    registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    readout = routeSignpostReadout(registry, signpost);
    assert(readout.rfind("TO ", 0) == 0);
    assert(readout.find("ROUTE: SUPPLY ROUTE") != std::string::npos);
    assert(readout.find("CARRIES: ???") != std::string::npos);
    assert(readout.find("CARRIES: MATERIAL") == std::string::npos);
    assert(readout.find("SPOOFED: ROUTE SIGNAL CONFUSED") != std::string::npos);

    registry.get<RouteSignpostComponent>(signpost).spoofed = false;
    registry.get<RouteSignpostComponent>(signpost).signal_recovered = true;
    readout = routeSignpostReadout(registry, signpost);
    assert(readout.find("CARRIES: MATERIAL") != std::string::npos);
    assert(readout.find("CARRIES: ???") == std::string::npos);
    assert(readout.find("RECOVERY: ROUTE SIGNAL CLEAR") != std::string::npos);
}

static void testSpoofedSupplyRouteShowsAndClearsWorkplaceFlowConsequence() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    const Entity supply = firstBuildingByRole(registry, MicroZoneRole::SUPPLY);
    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(supply != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);
    const Entity signpost = firstRouteSignpostForPath(registry, supply_path);
    assert(signpost != MAX_ENTITIES);

    assert(!workplaceSupplyFlowDisruptedBySpoofedRoute(registry));
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") ==
           std::string::npos);

    registry.get<RouteSignpostComponent>(signpost).spoofed = true;
    assert(workplaceSupplyFlowDisruptedBySpoofedRoute(registry));
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, supply).find("SUPPLY FLOW: DISRUPTED") ==
           std::string::npos);

    registry.get<RouteSignpostComponent>(signpost).spoofed = false;
    registry.get<RouteSignpostComponent>(signpost).signal_recovered = true;
    assert(!workplaceSupplyFlowDisruptedBySpoofedRoute(registry));
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") ==
           std::string::npos);
}

static void testSpoofedRouteShowsAndClearsLocalFlowBlockageReadouts() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    const Entity labor_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(labor_path != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);

    const Entity labor_signpost = firstRouteSignpostForPath(registry, labor_path);
    const Entity supply_signpost = firstRouteSignpostForPath(registry, supply_path);
    assert(labor_signpost != MAX_ENTITIES);
    assert(supply_signpost != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(supply_signpost));

    assert(!routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: BLOCKED") ==
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") !=
           std::string::npos);
    assert(pathInspectionReadout(registry, labor_path).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, labor_signpost).find("FLOW: BLOCKED") ==
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(routeFlowRecoveredByRestoredSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: CLEAR") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: CLEAR") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") ==
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: CLEAR") !=
           std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(!routeFlowRecoveredByRestoredSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: CLEAR") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, supply_signpost).find("FLOW: CLEAR") ==
           std::string::npos);
}

static void testSpoofedLaborRouteDoesNotDisruptSupplyFlowReadout() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity workplace = firstBuildingByRole(registry, MicroZoneRole::WORKPLACE);
    const Entity labor_path =
        firstPathBetweenRoles(registry, MicroZoneRole::HOUSING, MicroZoneRole::WORKPLACE);
    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(workplace != MAX_ENTITIES);
    assert(labor_path != MAX_ENTITIES);
    assert(supply_path != MAX_ENTITIES);

    const Entity labor_signpost = firstRouteSignpostForPath(registry, labor_path);
    assert(labor_signpost != MAX_ENTITIES);
    registry.get<RouteSignpostComponent>(labor_signpost).spoofed = true;

    assert(routeBetweenRolesHasSpoofedSignpost(registry,
                                               MicroZoneRole::HOUSING,
                                               MicroZoneRole::WORKPLACE));
    assert(!workplaceSupplyFlowDisruptedBySpoofedRoute(registry));
    assert(buildingInspectionReadout(registry, workplace).find("SUPPLY FLOW: DISRUPTED") ==
           std::string::npos);

    const Entity supply_signpost = firstRouteSignpostForPath(registry, supply_path);
    assert(supply_signpost != MAX_ENTITIES);
    registry.get<RouteSignpostComponent>(supply_signpost).spoofed = true;
    assert(workplaceSupplyFlowDisruptedBySpoofedRoute(registry));
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

static void testTinySaveRoundTripRestoresRouteFlowPersistenceBoundary() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);

    const Entity supply_path =
        firstPathBetweenRoles(registry, MicroZoneRole::WORKPLACE, MicroZoneRole::SUPPLY);
    assert(supply_path != MAX_ENTITIES);
    const Entity marker = firstRouteSignpostForPath(registry, supply_path);
    assert(marker != MAX_ENTITIES);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 12.0f, 12.0f);
    registry.assign<BuildingInteractionComponent>(player);

    registry.get<RouteSignpostComponent>(marker).spoofed = true;
    const std::string blocked_save =
        serializeTinySaveState(captureTinySaveState(registry, player));
    assert(blocked_save.find("SPOOFED_SIGNPOSTS 1") != std::string::npos);

    TinySaveState blocked_state;
    assert(deserializeTinySaveState(blocked_save, blocked_state) == TinySaveStatus::OK);
    registry.get<RouteSignpostComponent>(marker).spoofed = false;
    registry.get<RouteSignpostComponent>(marker).signal_recovered = true;

    assert(applyTinySaveState(registry, player, blocked_state) == TinySaveStatus::OK);
    assert(routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("FLOW: BLOCKED") !=
           std::string::npos);

    registry.get<RouteSignpostComponent>(marker).spoofed = false;
    registry.get<RouteSignpostComponent>(marker).signal_recovered = true;
    const std::string clear_save =
        serializeTinySaveState(captureTinySaveState(registry, player));
    assert(clear_save.find("SPOOFED_SIGNPOSTS 0") != std::string::npos);

    TinySaveState clear_state;
    assert(deserializeTinySaveState(clear_save, clear_state) == TinySaveStatus::OK);
    registry.get<RouteSignpostComponent>(marker).spoofed = true;
    registry.get<RouteSignpostComponent>(marker).signal_recovered = false;

    assert(applyTinySaveState(registry, player, clear_state) == TinySaveStatus::OK);
    assert(!routeFlowBlockedBySpoofedSignpost(registry, supply_path));
    assert(!routeFlowRecoveredByRestoredSignpost(registry, supply_path));
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(pathInspectionReadout(registry, supply_path).find("FLOW: CLEAR") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("FLOW: CLEAR") ==
           std::string::npos);
    assert(routeSignpostReadout(registry, marker).find("CARRIES: MATERIAL") !=
           std::string::npos);
}

static void testWorkerScanShowsWageImpactWhenSuspicionRecordExists() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);

    const std::string scan_with_record =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_with_record.find("WORKER SIGNAL: DEBT WORK") != std::string::npos);
    assert(scan_with_record.find("WAGE IMPACT: INCIDENT LOGGED") != std::string::npos);
    assert(scan_with_record.find("DOCK RISK: ACTIVE") != std::string::npos);

    registry.remove<LocalSuspicionComponent>(worker);
    const std::string scan_no_record =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_no_record.find("WORKER SIGNAL: DEBT WORK") != std::string::npos);
    assert(scan_no_record.find("WAGE IMPACT") == std::string::npos);
    assert(scan_no_record.find("DOCK RISK") == std::string::npos);
}

static void testWorkerWageRecordSpoofRequiresSuspicionRecord() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);
    registry.assign<InheritedGadgetComponent>(player);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(worker);

    assert(inheritedGadgetCanSpoofTarget(registry,
                                         playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH SPOOF WAGE RECORD");

    registry.remove<LocalSuspicionComponent>(worker);
    assert(!inheritedGadgetCanSpoofTarget(registry,
                                          playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH:N/A");
}

static void testWorkerWageRecordSpoofTogglesClearedReadout() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);
    registry.assign<InheritedGadgetComponent>(player);

    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(worker);

    const std::string before_spoof =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(before_spoof.find("DOCK RISK: ACTIVE") != std::string::npos);
    assert(before_spoof.find("DOCK RISK: CLEARED") == std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: SPOOFED WAGE RECORD: INCIDENT CLEARED");
    assert(registry.get<FixedActorComponent>(worker).wage_record_spoofed);
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH RESTORE WAGE RECORD");

    const std::string after_spoof =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(after_spoof.find("DOCK RISK: ACTIVE") == std::string::npos);
    assert(after_spoof.find("DOCK RISK: CLEARED") != std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: RESTORED WAGE RECORD: INCIDENT ACTIVE");
    assert(!registry.get<FixedActorComponent>(worker).wage_record_spoofed);

    const std::string after_restore =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(after_restore.find("DOCK RISK: ACTIVE") != std::string::npos);
    assert(after_restore.find("DOCK RISK: CLEARED") == std::string::npos);
}

static void testTinySaveRoundTripRestoresWageRecordSpoofed() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);

    registry.get<FixedActorComponent>(worker).wage_record_spoofed = true;
    const std::string save_text = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(save_text.find("NEON_TINY_SAVE_V12") != std::string::npos);

    TinySaveState state;
    assert(deserializeTinySaveState(save_text, state) == TinySaveStatus::OK);
    assert(state.worker_wage_record_spoofed);

    registry.get<FixedActorComponent>(worker).wage_record_spoofed = false;
    assert(applyTinySaveState(registry, player, state) == TinySaveStatus::OK);
    assert(registry.get<FixedActorComponent>(worker).wage_record_spoofed);

    registry.get<FixedActorComponent>(worker).wage_record_spoofed = false;
    const std::string save_text2 = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState state2;
    assert(deserializeTinySaveState(save_text2, state2) == TinySaveStatus::OK);
    assert(!state2.worker_wage_record_spoofed);
    assert(applyTinySaveState(registry, player, state2) == TinySaveStatus::OK);
    assert(!registry.get<FixedActorComponent>(worker).wage_record_spoofed);
}

static void testClinicAccessLedgerRequiresWorkerRecord() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    buildWorld(registry, config);

    const Entity clinic = firstBuildingByRole(registry, MicroZoneRole::CLINIC);
    assert(clinic != MAX_ENTITIES);
    assert(registry.has<ClinicAccessLedgerComponent>(clinic));
    assert(clinicAccessLedgerReadout(registry, clinic).empty());
    assert(buildingInspectionReadout(registry, clinic).find("CLINIC LEDGER") ==
           std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{clinic, InspectionTargetType::CLINIC})
               .find("CLINIC LEDGER") == std::string::npos);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(clinic));
    registry.assign<BuildingInteractionComponent>(player);

    assert(!inheritedGadgetCanSpoofTarget(registry,
                                          playerInspectionTarget(registry, player, 22.0f)));
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH:N/A");
    assert(!useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);
}

static void testClinicAccessLedgerShowsFlaggedWorkerRecord() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity clinic = MAX_ENTITIES;
    setupClinicAccessLedgerSuspicion(registry, player, worker, clinic);

    const std::string ordinary_readout = buildingInspectionReadout(registry, clinic);
    assert(ordinary_readout.find("CLINIC LEDGER: WORK RECORD FLAGGED") !=
           std::string::npos);
    assert(ordinary_readout.find("GHOST CLEARANCE") == std::string::npos);

    const std::string scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{clinic, InspectionTargetType::CLINIC});
    assert(scan.find("CLINIC PURPOSE: PUBLIC HEALTH") != std::string::npos);
    assert(scan.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(scan.find("GHOST CLEARANCE") == std::string::npos);
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH SPOOF CLINIC ACCESS");
    assert(localSuspicionWorkerForWorker(registry, worker) == worker);
}

static void testClinicAccessLedgerSpoofTogglesReadouts() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity clinic = MAX_ENTITIES;
    setupClinicAccessLedgerSuspicion(registry, player, worker, clinic);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: SPOOFED CLINIC ACCESS: GHOST CLEARANCE");
    assert(registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);
    assert(inheritedGadgetSpoofPromptReadout(registry, player, 22.0f) ==
           "G INTERFERENCE TORCH RESTORE CLINIC ACCESS");
    assert(buildingInspectionReadout(registry, clinic).find(
               "CLINIC ACCESS: GHOST CLEARANCE") != std::string::npos);
    assert(inheritedGadgetScanResult(registry,
                                     InspectionTarget{clinic, InspectionTargetType::CLINIC})
               .find("CLINIC ACCESS: GHOST CLEARANCE") != std::string::npos);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(inheritedGadgetResultReadout(registry, player) ==
           "INTERFERENCE TORCH RESULT: RESTORED CLINIC ACCESS: WORK RECORD FLAGGED");
    assert(!registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);
    const std::string restored_scan =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{clinic, InspectionTargetType::CLINIC});
    assert(restored_scan.find("CLINIC LEDGER: WORK RECORD FLAGGED") != std::string::npos);
    assert(restored_scan.find("GHOST CLEARANCE") == std::string::npos);
    assert(localSuspicionWorkerForWorker(registry, worker) == worker);
}

static void testWorkerScanShowsClinicAccessMismatchWhenClinicSpoofed() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity clinic = MAX_ENTITIES;
    setupClinicAccessLedgerSuspicion(registry, player, worker, clinic);

    const std::string scan_before =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_before.find("DOCK RISK: ACTIVE") != std::string::npos);
    assert(scan_before.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") ==
           std::string::npos);

    registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed = true;

    const std::string scan_spoofed =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_spoofed.find("WAGE IMPACT: INCIDENT LOGGED") != std::string::npos);
    assert(scan_spoofed.find("DOCK RISK: ACTIVE") != std::string::npos);
    assert(scan_spoofed.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") !=
           std::string::npos);
    assert(scan_spoofed.find("LOCAL WITNESS") != std::string::npos);

    registry.get<FixedActorComponent>(worker).wage_record_spoofed = true;
    const std::string scan_wage_spoofed =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_wage_spoofed.find("WAGE IMPACT: RECORD ALTERED") != std::string::npos);
    assert(scan_wage_spoofed.find("DOCK RISK: CLEARED") != std::string::npos);
    assert(scan_wage_spoofed.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") !=
           std::string::npos);

    registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed = false;
    const std::string scan_restored =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_restored.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") ==
           std::string::npos);
    assert(scan_restored.find("DOCK RISK: CLEARED") != std::string::npos);

    registry.remove<LocalSuspicionComponent>(worker);
    registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed = true;
    const std::string scan_no_record =
        inheritedGadgetScanResult(registry,
                                  InspectionTarget{worker, InspectionTargetType::WORKER});
    assert(scan_no_record.find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") ==
           std::string::npos);
    assert(scan_no_record.find("DOCK RISK") == std::string::npos);
}

static void testClinicAccessSpoofPreservesWorkerSuspicionAndWageState() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity clinic = MAX_ENTITIES;
    setupClinicAccessLedgerSuspicion(registry, player, worker, clinic);

    const auto& suspicion_before = registry.get<LocalSuspicionComponent>(worker);
    const bool record_before = localSuspicionRecordExists(suspicion_before);
    const bool wage_before = registry.get<FixedActorComponent>(worker).wage_record_spoofed;
    const bool log_recovered_before = suspicion_before.institutional_log_recovered;
    const bool dependency_before = dependencyDisrupted(registry);
    assert(record_before);
    assert(!wage_before);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);

    const auto& suspicion_after_spoof = registry.get<LocalSuspicionComponent>(worker);
    assert(localSuspicionRecordExists(suspicion_after_spoof) == record_before);
    assert(suspicion_after_spoof.cause == suspicion_before.cause);
    assert(suspicion_after_spoof.resolution == suspicion_before.resolution);
    assert(suspicion_after_spoof.institutional_log_recovered == log_recovered_before);
    assert(registry.get<FixedActorComponent>(worker).wage_record_spoofed == wage_before);
    assert(dependencyDisrupted(registry) == dependency_before);

    assert(useInheritedGadgetSpoof(registry, player, 22.0f));
    assert(!registry.get<ClinicAccessLedgerComponent>(clinic).access_spoofed);

    const auto& suspicion_after_restore = registry.get<LocalSuspicionComponent>(worker);
    assert(localSuspicionRecordExists(suspicion_after_restore) == record_before);
    assert(suspicion_after_restore.cause == suspicion_before.cause);
    assert(suspicion_after_restore.resolution == suspicion_before.resolution);
    assert(suspicion_after_restore.institutional_log_recovered == log_recovered_before);
    assert(registry.get<FixedActorComponent>(worker).wage_record_spoofed == wage_before);
    assert(dependencyDisrupted(registry) == dependency_before);
}

static void setWorldPhaseForTest(Registry& registry,
                                 WorldPhase phase,
                                 float elapsed_seconds = 0.0f) {
    const Entity phase_entity = worldPhaseEntity(registry);
    assert(phase_entity != MAX_ENTITIES);
    auto& component = registry.get<WorldPhaseComponent>(phase_entity);
    component.phase = phase;
    component.elapsed_seconds = elapsed_seconds;
    component.interval_seconds = 8.0f;
}

static void setupTheftWitnessRangeFixture(Registry& registry,
                                          Entity& player,
                                          Entity& worker,
                                          Entity& workplace,
                                          Entity& object,
                                          int worker_count,
                                          WorldPhase phase) {
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = worker_count;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);
    setWorldPhaseForTest(registry, phase);

    workplace = firstWorkplaceBenchBuilding(registry);
    worker = firstFixedWorker(registry);
    object = firstCarryableObject(registry);
    assert(workplace != MAX_ENTITIES);
    assert(worker != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, registry.get<TransformComponent>(workplace));
    assert(enterBuildingInterior(registry, player, workplace));

    hideCarryableObject(registry, object);
    registry.get<WorkplaceBenchComponent>(workplace).state = WorkplaceBenchState::OUTPUT_READY;
}

static void placeWorkerAtAabbDistance(Registry& registry,
                                      Entity worker,
                                      const TransformComponent& target,
                                      float distance_wu) {
    TransformComponent worker_transform = target;
    worker_transform.width = 10.0f;
    worker_transform.height = 10.0f;
    worker_transform.x = target.x + distance_wu + (target.width + worker_transform.width) * 0.5f;
    registry.get<TransformComponent>(worker) = worker_transform;
}

static void testWorldPhaseModulatesOutputTheftWitnessRange() {
    Registry day_registry;
    Entity day_player = MAX_ENTITIES;
    Entity day_worker = MAX_ENTITIES;
    Entity day_workplace = MAX_ENTITIES;
    Entity day_object = MAX_ENTITIES;
    setupTheftWitnessRangeFixture(day_registry,
                                  day_player,
                                  day_worker,
                                  day_workplace,
                                  day_object,
                                  1,
                                  WorldPhase::DAY);
    placeWorkerAtAabbDistance(day_registry,
                              day_worker,
                              day_registry.get<TransformComponent>(day_workplace),
                              16.0f);
    assert(takeWorkplaceOutput(day_registry, day_player));
    assert(localSuspicionActive(day_registry));
    assert(day_registry.get<PlayerComponent>(day_player).carried_object == day_object);

    Registry night_registry;
    Entity night_player = MAX_ENTITIES;
    Entity night_worker = MAX_ENTITIES;
    Entity night_workplace = MAX_ENTITIES;
    Entity night_object = MAX_ENTITIES;
    setupTheftWitnessRangeFixture(night_registry,
                                  night_player,
                                  night_worker,
                                  night_workplace,
                                  night_object,
                                  1,
                                  WorldPhase::NIGHT);
    placeWorkerAtAabbDistance(night_registry,
                              night_worker,
                              night_registry.get<TransformComponent>(night_workplace),
                              16.0f);
    assert(takeWorkplaceOutput(night_registry, night_player));
    assert(!localSuspicionActive(night_registry));
    assert(night_registry.view<LocalSuspicionComponent>().empty());
    assert(worldPhaseReadout(night_registry).find("PHASE: NIGHT") != std::string::npos);
}

static void testWorldPhaseUsesElapsedTimeNotFrames() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    buildWorld(registry, config);

    const Entity phase_entity = worldPhaseEntity(registry);
    assert(phase_entity != MAX_ENTITIES);
    assert(currentWorldPhase(registry) == WorldPhase::DAY);
    assert(closeTo(registry.get<WorldPhaseComponent>(phase_entity).interval_seconds, 240.0f));

    for (int i = 0; i < 120; ++i) {
        advanceWorldPhase(registry, 1.0f / 60.0f);
    }
    assert(currentWorldPhase(registry) == WorldPhase::DAY);

    advanceWorldPhase(registry, 238.0f);
    assert(currentWorldPhase(registry) == WorldPhase::NIGHT);
}

static void testTwoWorkersCreateCrowdCamouflageAndDistinctRoutes() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity first_worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupTheftWitnessRangeFixture(registry,
                                  player,
                                  first_worker,
                                  workplace,
                                  object,
                                  2,
                                  WorldPhase::DAY);

    const auto workers = fixedWorkersInSaveOrder(registry);
    assert(workers.size() == 2);
    assert(registry.get<FixedActorComponent>(workers[0]).path_entity !=
           registry.get<FixedActorComponent>(workers[1]).path_entity);

    const TransformComponent event_transform = registry.get<TransformComponent>(workplace);
    placeWorkerAtAabbDistance(registry, workers[0], event_transform, 16.0f);
    placeWorkerAtAabbDistance(registry, workers[1], event_transform, 16.0f);
    assert(effectiveLocalWitnessRange(registry, event_transform, kLocalWitnessRangeWu) ==
           kLocalWitnessRangeWu * 0.5f);
    assert(takeWorkplaceOutput(registry, player));
    assert(!localSuspicionActive(registry));
}

static void testTinySaveRoundTripRestoresWorldPhaseAndTwoWorkers() {
    Registry registry;
    WorldConfig config = makeSandboxConfig();
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.fixed_worker_count = 2;
    config.carryable_object_count = 1;
    buildWorld(registry, config);
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);
    setWorldPhaseForTest(registry, WorldPhase::NIGHT, 3);

    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, TransformComponent{0, 0, 12, 12});

    auto workers = fixedWorkersInSaveOrder(registry);
    assert(workers.size() == 2);
    registry.get<FixedActorComponent>(workers[0]).route_t = 0.25f;
    registry.get<FixedActorComponent>(workers[0]).direction = 1.0f;
    registry.get<FixedActorComponent>(workers[0]).wage_record_spoofed = true;
    registry.get<FixedActorComponent>(workers[1]).route_t = 0.75f;
    registry.get<FixedActorComponent>(workers[1]).direction = -1.0f;
    registry.get<FixedActorComponent>(workers[1]).acknowledged = true;

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("NEON_TINY_SAVE_V12") != std::string::npos);
    assert(serialized.find("WORLD_PHASE NIGHT 3") != std::string::npos);
    assert(serialized.find("WORKERS 2") != std::string::npos);

    TinySaveState state;
    assert(deserializeTinySaveState(serialized, state) == TinySaveStatus::OK);
    registry.get<WorldPhaseComponent>(worldPhaseEntity(registry)).phase = WorldPhase::DAY;
    registry.get<FixedActorComponent>(workers[0]).route_t = 0.0f;
    registry.get<FixedActorComponent>(workers[0]).wage_record_spoofed = false;
    registry.get<FixedActorComponent>(workers[1]).route_t = 0.0f;
    registry.get<FixedActorComponent>(workers[1]).acknowledged = false;
    assert(applyTinySaveState(registry, player, state) == TinySaveStatus::OK);
    workers = fixedWorkersInSaveOrder(registry);
    assert(currentWorldPhase(registry) == WorldPhase::NIGHT);
    assert(closeTo(registry.get<WorldPhaseComponent>(worldPhaseEntity(registry)).elapsed_seconds,
                   3.0f));
    assert(closeTo(registry.get<FixedActorComponent>(workers[0]).route_t, 0.25f));
    assert(registry.get<FixedActorComponent>(workers[0]).wage_record_spoofed);
    assert(closeTo(registry.get<FixedActorComponent>(workers[1]).route_t, 0.75f));
    assert(registry.get<FixedActorComponent>(workers[1]).acknowledged);
}

static void testLayLowConsumesSupplyAndPreservesRecord() {
    Registry registry;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(registry, player, worker, workplace, object);

    Entity housing = firstBuildingByRole(registry, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    assert(exitBuildingInterior(registry, player));
    registry.get<TransformComponent>(player) = registry.get<TransformComponent>(housing);
    assert(enterBuildingInterior(registry, player, housing));
    registry.assign<InheritedGadgetComponent>(player);
    registry.get<ShelterStockComponent>(housing).current_supply = 1;

    assert(playerCanLayLowInHousing(registry, player));
    assert(useLayLowInHousing(registry, player));
    assert(registry.get<ShelterStockComponent>(housing).current_supply == 0);
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(!localSuspicionActive(registry));
    const auto& suspicion = registry.get<LocalSuspicionComponent>(worker);
    assert(suspicion.resolution == LocalSuspicionResolution::LAID_LOW);
    assert(workerCarryReadout(registry, worker).find("SUSPICION QUIET: LAID LOW MISSING PART") !=
           std::string::npos);
    assert(localSuspicionHudReadout(registry).empty());
    assert(inheritedGadgetResultReadout(registry, player) ==
           "ACTION RESULT: LAY LOW: LOCAL NOTICE QUIETED");

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("LAID_LOW") != std::string::npos);
    TinySaveState state;
    assert(deserializeTinySaveState(serialized, state) == TinySaveStatus::OK);
    registry.remove<LocalSuspicionComponent>(worker);
    registry.get<ShelterStockComponent>(housing).current_supply = 1;
    assert(applyTinySaveState(registry, player, state) == TinySaveStatus::OK);
    assert(registry.get<ShelterStockComponent>(housing).current_supply == 0);
    assert(registry.get<LocalSuspicionComponent>(worker).resolution ==
           LocalSuspicionResolution::LAID_LOW);
}

static void testLayLowFailureCasesAndNightPhase() {
    Registry no_supply;
    Entity player = MAX_ENTITIES;
    Entity worker = MAX_ENTITIES;
    Entity workplace = MAX_ENTITIES;
    Entity object = MAX_ENTITIES;
    setupWitnessedMissingPartSuspicion(no_supply, player, worker, workplace, object);
    Entity housing = firstBuildingByRole(no_supply, MicroZoneRole::HOUSING);
    assert(housing != MAX_ENTITIES);
    assert(exitBuildingInterior(no_supply, player));
    no_supply.get<TransformComponent>(player) = no_supply.get<TransformComponent>(housing);
    assert(enterBuildingInterior(no_supply, player, housing));
    no_supply.assign<InheritedGadgetComponent>(player);
    no_supply.get<ShelterStockComponent>(housing).current_supply = 0;
    assert(!useLayLowInHousing(no_supply, player));
    assert(localSuspicionActive(no_supply));
    assert(inheritedGadgetResultReadout(no_supply, player) ==
           "ACTION RESULT: LAY LOW FAILED: SHELTER SUPPLY REQUIRED");

    Registry no_suspicion;
    WorldConfig config = makeSandboxConfig();
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    buildWorld(no_suspicion, config);
    deriveInfrastructure(no_suspicion, config);
    setWorldPhaseForTest(no_suspicion, WorldPhase::NIGHT, 2);
    Entity no_suspicion_housing = firstBuildingByRole(no_suspicion, MicroZoneRole::HOUSING);
    Entity no_suspicion_player = no_suspicion.create();
    no_suspicion.assign<PlayerComponent>(no_suspicion_player);
    no_suspicion.assign<InheritedGadgetComponent>(no_suspicion_player);
    no_suspicion.assign<BuildingInteractionComponent>(no_suspicion_player);
    no_suspicion.assign<TransformComponent>(
        no_suspicion_player,
        no_suspicion.get<TransformComponent>(no_suspicion_housing));
    no_suspicion.get<ShelterStockComponent>(no_suspicion_housing).current_supply = 1;
    assert(enterBuildingInterior(no_suspicion, no_suspicion_player, no_suspicion_housing));
    assert(!useLayLowInHousing(no_suspicion, no_suspicion_player));
    assert(no_suspicion.get<ShelterStockComponent>(no_suspicion_housing).current_supply == 1);
    assert(currentWorldPhase(no_suspicion) == WorldPhase::NIGHT);
    assert(inheritedGadgetResultReadout(no_suspicion, no_suspicion_player) ==
           "ACTION RESULT: LAY LOW FAILED: NO ACTIVE LOCAL SUSPICION");
}

static WorldConfig makeTransitTestConfig(int macro_count_x = 2) {
    WorldConfig config = makeSandboxConfig();
    config.macro_count_x = macro_count_x;
    config.workplace_micro_zone_count = 1;
    config.workplace_building_count = 1;
    config.supply_micro_zone_count = 1;
    config.supply_building_count = 1;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    config.clinic_micro_zone_count = 1;
    config.clinic_building_count = 1;
    config.fixed_worker_count = 2;
    config.carryable_object_count = 1;
    config.transit_enabled = macro_count_x >= 2;
    config.transit_ride_seconds = 1.0f;
    return config;
}

static Entity buildTransitWorld(Registry& registry, const WorldConfig& config) {
    buildWorld(registry, config);
    assert(validateWorld(registry, config));
    deriveInfrastructure(registry, config);
    spawnFixedActors(registry, config);
    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<InheritedGadgetComponent>(player);
    registry.assign<BuildingInteractionComponent>(player);
    registry.assign<TransformComponent>(player, TransformComponent{0, 0, 12, 12});
    return player;
}

static std::vector<Entity> stationsInOrder(Registry& registry) {
    auto stations = registry.view<StationComponent>();
    std::vector<Entity> ordered(stations.begin(), stations.end());
    std::sort(ordered.begin(), ordered.end());
    return ordered;
}

static Entity firstBuildingByRoleInDistrict(Registry& registry,
                                            MicroZoneRole role,
                                            uint32_t district_id) {
    auto buildings = registry.view<BuildingComponent, BuildingUseComponent, TransformComponent>();
    for (Entity building : buildings) {
        if (registry.get<BuildingUseComponent>(building).role == role &&
            districtIdForEntity(registry, building) == district_id) {
            return building;
        }
    }
    return MAX_ENTITIES;
}

static Entity firstPathBetweenRolesInDistrict(Registry& registry,
                                              MicroZoneRole from_role,
                                              MicroZoneRole to_role,
                                              uint32_t district_id) {
    auto paths = registry.view<PathComponent>();
    for (Entity path_entity : paths) {
        const auto& path = registry.get<PathComponent>(path_entity);
        if (path.kind != PathKind::PEDESTRIAN ||
            districtIdForEntity(registry, path_entity) != district_id ||
            !registry.alive(path.from) ||
            !registry.alive(path.to) ||
            !registry.has<BuildingUseComponent>(path.from) ||
            !registry.has<BuildingUseComponent>(path.to)) {
            continue;
        }
        const MicroZoneRole from = registry.get<BuildingUseComponent>(path.from).role;
        const MicroZoneRole to = registry.get<BuildingUseComponent>(path.to).role;
        if ((from == from_role && to == to_role) ||
            (from == to_role && to == from_role)) {
            return path_entity;
        }
    }
    return MAX_ENTITIES;
}

static Entity firstWorkerInDistrict(Registry& registry, uint32_t district_id) {
    const std::vector<Entity> workers = fixedWorkersInSaveOrder(registry);
    for (Entity worker : workers) {
        if (districtIdForEntity(registry, worker) == district_id) {
            return worker;
        }
    }
    return MAX_ENTITIES;
}

static void testTransitStationsRequireTwoDistrictConfig() {
    Registry one_district;
    WorldConfig one = makeTransitTestConfig(1);
    Entity one_player = buildTransitWorld(one_district, one);
    assert(stationsInOrder(one_district).empty());
    assert(!playerCanBoardTransit(one_district, one_player, 18.0f));

    Registry two_districts;
    WorldConfig two = makeTransitTestConfig(2);
    buildTransitWorld(two_districts, two);
    const auto stations = stationsInOrder(two_districts);
    assert(stations.size() == 2);
    const auto& a = two_districts.get<StationComponent>(stations[0]);
    const auto& b = two_districts.get<StationComponent>(stations[1]);
    assert(a.linked_station == stations[1]);
    assert(b.linked_station == stations[0]);
    assert(a.district_id != b.district_id);
    assert(stationReadout(two_districts, stations[0]).find("DESTINATION: DISTRICT B") !=
           std::string::npos);
}

static void testTransitRideLookOutWindowCarriesObjectToDestination() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    Entity player = buildTransitWorld(registry, config);
    const auto stations = stationsInOrder(registry);
    assert(stations.size() == 2);
    Entity object = firstCarryableObject(registry);
    assert(object != MAX_ENTITIES);
    registry.get<PlayerComponent>(player).carried_object = object;
    hideCarryableObject(registry, object);
    registry.get<TransformComponent>(player) = stationExitTransform(registry, stations[0]);

    assert(playerCanBoardTransit(registry, player, 18.0f));
    assert(enterTransitRide(registry, player, 18.0f, config.transit_ride_seconds));
    assert(playerInsideTransitInterior(registry, player));
    assert(playerLocationState(registry, player, 18.0f) == PlayerLocationState::INSIDE_TRANSIT);
    auto& ride = registry.get<TransitRideComponent>(player);
    const float old_x = ride.interior_position.x;
    ride.interior_position = movedTransitInteriorPosition(ride.interior_position,
                                                          1.0f,
                                                          0.0f,
                                                          90.0f,
                                                          0.2f);
    assert(ride.interior_position.x > old_x);

    assert(finishTransitRide(registry, player, true));
    assert(!playerInsideTransitInterior(registry, player));
    assert(registry.get<PlayerComponent>(player).carried_object == object);
    assert(playerCurrentDistrictId(registry, player) ==
           registry.get<StationComponent>(stations[1]).district_id);
    assert(inheritedGadgetResultReadout(registry, player) ==
           "ACTION RESULT: LOOKED OUT WINDOW: DESTINATION PLATFORM");
}

static void testTransitRideCanWaitForDoorsThenExit() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    Entity player = buildTransitWorld(registry, config);
    const auto stations = stationsInOrder(registry);
    registry.get<TransformComponent>(player) = stationExitTransform(registry, stations[0]);

    assert(enterTransitRide(registry, player, 18.0f, config.transit_ride_seconds));
    advanceTransitRide(registry, player, 0.5f);
    assert(!registry.get<TransitRideComponent>(player).doors_open);
    advanceTransitRide(registry, player, 0.6f);
    assert(registry.get<TransitRideComponent>(player).doors_open);
    assert(finishTransitRide(registry, player, false));
    assert(playerCurrentDistrictId(registry, player) ==
           registry.get<StationComponent>(stations[1]).district_id);
    assert(inheritedGadgetResultReadout(registry, player) ==
           "ACTION RESULT: TRANSIT ARRIVED: DOORS OPEN");
}

static void testTinySaveRoundTripRestoresTransitInterior() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    Entity player = buildTransitWorld(registry, config);
    const auto stations = stationsInOrder(registry);
    registry.get<TransformComponent>(player) = stationExitTransform(registry, stations[0]);
    assert(enterTransitRide(registry, player, 18.0f, config.transit_ride_seconds));
    advanceTransitRide(registry, player, 0.75f);
    registry.get<TransitRideComponent>(player).interior_position.x = 18.0f;

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("NEON_TINY_SAVE_V12") != std::string::npos);
    assert(serialized.find("TRANSIT 1") != std::string::npos);

    TinySaveState state;
    assert(deserializeTinySaveState(serialized, state) == TinySaveStatus::OK);
    registry.remove<TransitRideComponent>(player);
    assert(applyTinySaveState(registry, player, state) == TinySaveStatus::OK);
    assert(playerInsideTransitInterior(registry, player));
    const auto& ride = registry.get<TransitRideComponent>(player);
    assert(closeTo(ride.elapsed_seconds, 0.75f));
    assert(closeTo(ride.interior_position.x, 18.0f));
    assert(ride.destination_station == stations[1]);
}

static void testPerDistrictWorkersSpawnOnLocalLaborRoutes() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    buildTransitWorld(registry, config);

    const std::vector<Entity> workers = fixedWorkersInSaveOrder(registry);
    assert(workers.size() == 2);
    assert(firstWorkerInDistrict(registry, 0) != MAX_ENTITIES);
    assert(firstWorkerInDistrict(registry, 1) != MAX_ENTITIES);

    for (Entity worker : workers) {
        const uint32_t district_id = districtIdForEntity(registry, worker);
        const auto& actor = registry.get<FixedActorComponent>(worker);
        assert(pathConnectsRoles(registry,
                                 actor.path_entity,
                                 MicroZoneRole::HOUSING,
                                 MicroZoneRole::WORKPLACE));
        assert(districtIdForEntity(registry, actor.path_entity) == district_id);
        assert(workerCarryReadout(registry, worker).find(
                   districtLabel(district_id) + ":WORKER") != std::string::npos);
    }

    for (int i = 0; i < 120; ++i) {
        updateFixedActors(registry, 0.1f);
    }
    for (Entity worker : workers) {
        assert(districtIdForEntity(registry,
                                   registry.get<FixedActorComponent>(worker).path_entity) ==
               districtIdForEntity(registry, worker));
    }
}

static void testDistrictTagsAppearOnInspectionLabels() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    buildTransitWorld(registry, config);

    const Entity workplace_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 0);
    const Entity path_a = firstPathBetweenRolesInDistrict(registry,
                                                         MicroZoneRole::HOUSING,
                                                         MicroZoneRole::WORKPLACE,
                                                         0);
    const Entity signpost_a = firstRouteSignpostForPath(registry, path_a);
    const Entity worker_a = firstWorkerInDistrict(registry, 0);
    assert(workplace_a != MAX_ENTITIES);
    assert(path_a != MAX_ENTITIES);
    assert(signpost_a != MAX_ENTITIES);
    assert(worker_a != MAX_ENTITIES);

    assert(buildingInspectionReadout(registry, workplace_a).find("A:WORKPLACE") !=
           std::string::npos);
    assert(pathInspectionReadout(registry, path_a).find("A:ROUTE: LABOR ROUTE") !=
           std::string::npos);
    assert(routeSignpostReadout(registry, signpost_a).find("A:ROUTE: LABOR ROUTE") !=
           std::string::npos);
    assert(inheritedGadgetWorkerScan(registry, worker_a).find("A:WORKER SIGNAL") !=
           std::string::npos);
}

static void testDistrictSuspicionAndWageReadoutsDoNotLeak() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    buildTransitWorld(registry, config);

    const Entity worker_a = firstWorkerInDistrict(registry, 0);
    const Entity worker_b = firstWorkerInDistrict(registry, 1);
    const Entity workplace_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 0);
    const Entity workplace_b = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 1);
    const Entity clinic_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::CLINIC, 0);
    const Entity clinic_b = firstBuildingByRoleInDistrict(registry, MicroZoneRole::CLINIC, 1);
    const Entity object = firstCarryableObject(registry);
    assert(worker_a != MAX_ENTITIES);
    assert(worker_b != MAX_ENTITIES);
    assert(workplace_a != MAX_ENTITIES);
    assert(workplace_b != MAX_ENTITIES);
    assert(clinic_a != MAX_ENTITIES);
    assert(clinic_b != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    recordLocalSuspicion(registry,
                         worker_a,
                         LocalSuspicionCause::MISSING_PART,
                         workplace_a,
                         object,
                         MAX_ENTITIES);

    assert(localSuspicionHudReadout(registry).find("LOCAL NOTICE: A:WORKER SAW MISSING PART") !=
           std::string::npos);
    assert(workerCarryReadout(registry, worker_a).find("SUSPICION: MISSING PART") !=
           std::string::npos);
    assert(workerCarryReadout(registry, worker_b).find("SUSPICION") == std::string::npos);
    assert(buildingInspectionReadout(registry, workplace_a).find("SUSPICION: MISSING PART") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace_b).find("SUSPICION") ==
           std::string::npos);

    const std::string scan_a = inheritedGadgetWorkerScan(registry, worker_a);
    const std::string scan_b = inheritedGadgetWorkerScan(registry, worker_b);
    assert(scan_a.find("A:WAGE IMPACT: INCIDENT LOGGED") != std::string::npos);
    assert(scan_a.find("LOCAL WITNESS: A:WORKER") != std::string::npos);
    assert(scan_b.find("WAGE IMPACT") == std::string::npos);
    assert(scan_b.find("LOCAL WITNESS") == std::string::npos);

    assert(!clinicAccessLedgerReadout(registry, clinic_a).empty());
    assert(clinicAccessLedgerReadout(registry, clinic_b).empty());
    assert(clinicAccessCanSpoof(registry, clinic_a));
    assert(!clinicAccessCanSpoof(registry, clinic_b));

    registry.get<ClinicAccessLedgerComponent>(clinic_a).access_spoofed = true;
    assert(inheritedGadgetWorkerScan(registry, worker_a)
               .find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") != std::string::npos);
    assert(inheritedGadgetWorkerScan(registry, worker_b)
               .find("CLINIC ACCESS: GHOST CLEARANCE MISMATCH") == std::string::npos);

    registry.get<FixedActorComponent>(worker_a).wage_record_spoofed = true;
    assert(inheritedGadgetWorkerScan(registry, worker_a)
               .find("A:WAGE IMPACT: RECORD ALTERED") != std::string::npos);
    assert(inheritedGadgetWorkerScan(registry, worker_b).find("WAGE IMPACT") ==
           std::string::npos);
}

static void testDistrictDependencyDisruptionDoesNotLeak() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    buildTransitWorld(registry, config);

    const Entity workplace_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 0);
    const Entity workplace_b = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 1);
    const Entity supply_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::SUPPLY, 0);
    const Entity supply_b = firstBuildingByRoleInDistrict(registry, MicroZoneRole::SUPPLY, 1);
    assert(workplace_a != MAX_ENTITIES);
    assert(workplace_b != MAX_ENTITIES);
    assert(supply_a != MAX_ENTITIES);
    assert(supply_b != MAX_ENTITIES);

    assert(toggleDependencyDisruption(registry, kWorkplaceDependsOnSupply, 0));
    assert(dependencyDisrupted(registry, kWorkplaceDependsOnSupply, 0));
    assert(!dependencyDisrupted(registry, kWorkplaceDependsOnSupply, 1));
    assert(buildingInspectionReadout(registry, workplace_a).find("DEPENDENCY: DISRUPTED") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, supply_a).find("DEPENDENCY: DISRUPTED") !=
           std::string::npos);
    assert(buildingInspectionReadout(registry, workplace_b).find("DEPENDENCY: DISRUPTED") ==
           std::string::npos);
    assert(buildingInspectionReadout(registry, supply_b).find("DEPENDENCY: DISRUPTED") ==
           std::string::npos);
    assert(dependencyScanReadout(registry,
                                 MicroZoneRole::WORKPLACE,
                                 kWorkplaceDependsOnSupply,
                                 1)
               .find("FLOW STATUS: CONFUSED") == std::string::npos);
}

static void testTinySaveRoundTripRestoresTwoDistrictWorkersAndRecords() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    Entity player = buildTransitWorld(registry, config);

    const Entity worker_a = firstWorkerInDistrict(registry, 0);
    const Entity worker_b = firstWorkerInDistrict(registry, 1);
    const Entity workplace_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 0);
    const Entity object = firstCarryableObject(registry);
    assert(worker_a != MAX_ENTITIES);
    assert(worker_b != MAX_ENTITIES);
    assert(workplace_a != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);

    registry.get<FixedActorComponent>(worker_a).wage_record_spoofed = true;
    registry.get<FixedActorComponent>(worker_b).acknowledged = true;
    recordLocalSuspicion(registry,
                         worker_a,
                         LocalSuspicionCause::MISSING_PART,
                         workplace_a,
                         object,
                         MAX_ENTITIES);

    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    TinySaveState state;
    assert(deserializeTinySaveState(serialized, state) == TinySaveStatus::OK);

    Registry loaded;
    Entity loaded_player = buildTransitWorld(loaded, config);
    assert(applyTinySaveState(loaded, loaded_player, state) == TinySaveStatus::OK);

    const Entity loaded_worker_a = firstWorkerInDistrict(loaded, 0);
    const Entity loaded_worker_b = firstWorkerInDistrict(loaded, 1);
    assert(loaded_worker_a != MAX_ENTITIES);
    assert(loaded_worker_b != MAX_ENTITIES);
    assert(loaded.get<FixedActorComponent>(loaded_worker_a).wage_record_spoofed);
    assert(loaded.get<FixedActorComponent>(loaded_worker_b).acknowledged);
    assert(loaded.has<LocalSuspicionComponent>(loaded_worker_a));
    assert(localSuspicionRecordExists(loaded.get<LocalSuspicionComponent>(loaded_worker_a)));
    assert(!loaded.has<LocalSuspicionComponent>(loaded_worker_b));
}

static void testTinySaveRoundTripRestoresPerDistrictInterferenceBoundary() {
    Registry registry;
    WorldConfig config = makeTransitTestConfig(2);
    Entity player = buildTransitWorld(registry, config);

    const auto stations = stationsInOrder(registry);
    assert(stations.size() == 2);
    const Entity worker_a = firstWorkerInDistrict(registry, 0);
    const Entity worker_b = firstWorkerInDistrict(registry, 1);
    const Entity workplace_a = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 0);
    const Entity workplace_b = firstBuildingByRoleInDistrict(registry, MicroZoneRole::WORKPLACE, 1);
    const Entity object = firstCarryableObject(registry);
    const Entity supply_path_a = firstPathBetweenRolesInDistrict(registry,
                                                                 MicroZoneRole::WORKPLACE,
                                                                 MicroZoneRole::SUPPLY,
                                                                 0);
    const Entity supply_path_b = firstPathBetweenRolesInDistrict(registry,
                                                                 MicroZoneRole::WORKPLACE,
                                                                 MicroZoneRole::SUPPLY,
                                                                 1);
    const Entity signpost_a = firstRouteSignpostForPath(registry, supply_path_a);
    const Entity signpost_b = firstRouteSignpostForPath(registry, supply_path_b);
    assert(worker_a != MAX_ENTITIES);
    assert(worker_b != MAX_ENTITIES);
    assert(workplace_a != MAX_ENTITIES);
    assert(workplace_b != MAX_ENTITIES);
    assert(object != MAX_ENTITIES);
    assert(signpost_a != MAX_ENTITIES);
    assert(signpost_b != MAX_ENTITIES);

    registry.get<RouteSignpostComponent>(signpost_a).spoofed = true;
    assert(toggleDependencyDisruption(registry, kWorkplaceDependsOnSupply, 0));
    registry.get<FixedActorComponent>(worker_a).wage_record_spoofed = true;
    recordLocalSuspicion(registry,
                         worker_a,
                         LocalSuspicionCause::MISSING_PART,
                         workplace_a,
                         object,
                         supply_path_a);
    assert(deEscalateLocalSuspicion(registry,
                                    worker_a,
                                    LocalSuspicionResolution::LAID_LOW,
                                    firstBuildingByRoleInDistrict(registry,
                                                                  MicroZoneRole::HOUSING,
                                                                  0)));

    assert(pathInspectionReadout(registry, supply_path_a).find("A:ROUTE: SUPPLY ROUTE") !=
           std::string::npos);
    assert(pathInspectionReadout(registry, supply_path_a).find("FLOW: BLOCKED") !=
           std::string::npos);
    assert(pathInspectionReadout(registry, supply_path_b).find("FLOW: BLOCKED") ==
           std::string::npos);
    assert(dependencyScanReadout(registry,
                                 MicroZoneRole::WORKPLACE,
                                 kWorkplaceDependsOnSupply,
                                 0)
               .find("DISTRICT: A; FLOW: MATERIAL") != std::string::npos);
    assert(localSuspicionDebuggerReadoutForWorker(registry, worker_a)
               .find("LOCAL WITNESS: A:WORKER") != std::string::npos);
    assert(localSuspicionDebuggerReadoutForWorker(registry, worker_a)
               .find("DE-ESCALATED: LAID LOW MISSING PART") != std::string::npos);
    assert(workerCarryReadout(registry, worker_b).find("SUSPICION") == std::string::npos);

    registry.get<TransformComponent>(player) = stationExitTransform(registry, stations[1]);
    assert(playerCurrentDistrictId(registry, player) == 1);
    const std::string serialized = serializeTinySaveState(captureTinySaveState(registry, player));
    assert(serialized.find("DEPENDENCY_DISRUPTIONS 1") != std::string::npos);
    assert(serialized.find("SPOOFED_SIGNPOSTS 1") != std::string::npos);
    assert(serialized.find("LAID_LOW") != std::string::npos);

    TinySaveState state;
    assert(deserializeTinySaveState(serialized, state) == TinySaveStatus::OK);

    Registry loaded;
    Entity loaded_player = buildTransitWorld(loaded, config);
    assert(applyTinySaveState(loaded, loaded_player, state) == TinySaveStatus::OK);
    assert(playerCurrentDistrictId(loaded, loaded_player) == 1);

    const Entity loaded_worker_a = firstWorkerInDistrict(loaded, 0);
    const Entity loaded_worker_b = firstWorkerInDistrict(loaded, 1);
    const Entity loaded_supply_path_a = firstPathBetweenRolesInDistrict(loaded,
                                                                       MicroZoneRole::WORKPLACE,
                                                                       MicroZoneRole::SUPPLY,
                                                                       0);
    const Entity loaded_supply_path_b = firstPathBetweenRolesInDistrict(loaded,
                                                                       MicroZoneRole::WORKPLACE,
                                                                       MicroZoneRole::SUPPLY,
                                                                       1);
    const Entity loaded_signpost_a = firstRouteSignpostForPath(loaded, loaded_supply_path_a);
    const Entity loaded_signpost_b = firstRouteSignpostForPath(loaded, loaded_supply_path_b);
    assert(routeSignpostSpoofed(loaded, loaded_signpost_a));
    assert(!routeSignpostSpoofed(loaded, loaded_signpost_b));
    assert(dependencyDisrupted(loaded, kWorkplaceDependsOnSupply, 0));
    assert(!dependencyDisrupted(loaded, kWorkplaceDependsOnSupply, 1));
    assert(loaded.get<FixedActorComponent>(loaded_worker_a).wage_record_spoofed);
    assert(!loaded.get<FixedActorComponent>(loaded_worker_b).wage_record_spoofed);
    assert(loaded.has<LocalSuspicionComponent>(loaded_worker_a));
    assert(loaded.get<LocalSuspicionComponent>(loaded_worker_a).resolution ==
           LocalSuspicionResolution::LAID_LOW);
    assert(!loaded.has<LocalSuspicionComponent>(loaded_worker_b));

    assert(playerCanBoardTransit(loaded, loaded_player, 18.0f));
    assert(enterTransitRide(loaded, loaded_player, 18.0f, config.transit_ride_seconds));
    assert(finishTransitRide(loaded, loaded_player, true));
    assert(playerCurrentDistrictId(loaded, loaded_player) == 0);
    assert(routeSignpostSpoofed(loaded, loaded_signpost_a));
    assert(dependencyDisrupted(loaded, kWorkplaceDependsOnSupply, 0));
}

int main() {
    testBuildWorldCreatesOnlyHousingBaseline();
    testConfiguredHousingCountCreatesNonOverlappingBuildings();
    testConfiguredWorkplaceCreatesSecondBuildingType();
    testConfiguredSupplyCreatesThirdPurposeBuilding();
    testThreeRoleLayoutKeepsUsableBuildingFootprints();
    testCommercialSiteRolePlacementAndInspection();
    testFiveRoleLayoutKeepsWorkerPathsConnectedAndMarketClear();
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
    testBuildingPurposeForCurrentRoles();
    testBuildingInspectionReadoutsIncludePurposeForCurrentRoles();
    testCommercialSiteDebuggerScanMetadataAndNoTarget();
    testCommercialSiteObservationOnlyBoundary();
    testPublicSitePlacementAndInspection();
    testSiteContextTagsInInspectionReadout();
    testPublicSiteDebuggerScanMetadata();
    testPublicSiteBoundaryDoesNotBreakLoop();
    testBuildingPurposeReadoutsDoNotMutateLoopState();
    testDependencyEdgeInspectionReadouts();
    testDependencyEdgeMissingTargetReadouts();
    testDependencyDebuggerScanEnrichment();
    testDependencyReadoutsDoNotMutateLoopState();
    testDependencyDisruptionViaDebuggerTogglesReadouts();
    testDependencyDisruptionPausesAndRestoresWorkerSupplyFlow();
    testDependencyDisruptionBoundariesDoNotSpoofRoutesOrUnrelatedSites();
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
    testWitnessedOutputTheftCreatesLocalSuspicion();
    testUnwitnessedOutputPickupDoesNotCreateLocalSuspicion();
    testWitnessedRouteTamperingCreatesLocalSuspicionAndRestoreKeepsIt();
    testUnwitnessedRouteTamperingDoesNotCreateLocalSuspicion();
    testReturningSuspiciousOutputDeEscalatesLocalConcern();
    testHidingSuspiciousItemInHousingKeepsInspectableConcern();
    testCorrectingSuspiciousRouteDeEscalatesOnlyMatchingSignpost();
    testTinySaveRoundTripRestoresLocalSuspicionStates();
    testInstitutionalLogFragmentRecoveryAddsAuditTrace();
    testRouteSignpostRecoveryReadoutAppearsOnlyAfterRestore();
    testRouteSignpostConsequenceReadoutCoversLocalStates();
    testProductionLoopSummaryReadoutCoversRouteInterferenceStates();
    testRoutePurposeReadoutsAndDebuggerScanEnrichment();
    testSpoofedRouteSignpostCorruptsFlowLabel();
    testSpoofedSupplyRouteShowsAndClearsWorkplaceFlowConsequence();
    testSpoofedRouteShowsAndClearsLocalFlowBlockageReadouts();
    testSpoofedLaborRouteDoesNotDisruptSupplyFlowReadout();
    testInheritedGadgetSpoofLeavesWorkerAndLoopStateUnchanged();
    testSpoofedRouteSignalAffectsOnlyRelevantWorkerPathReadout();
    testSpoofedRouteSignalSourceLabelAppearsAndClears();
    testSpoofedRouteSignalPausesAndRestoresWorkerPathMovement();
    testSpoofedRouteSignalPausesAndRestoresSupplyDeliveryMovement();
    testSpoofedRouteSignalConsequenceDoesNotMutateProductionState();
    testTinySaveRoundTripRestoresSpoofedSignpostState();
    testTinySaveRoundTripRestoresRouteFlowPersistenceBoundary();
    testWorkerScanShowsWageImpactWhenSuspicionRecordExists();
    testWorkerWageRecordSpoofRequiresSuspicionRecord();
    testWorkerWageRecordSpoofTogglesClearedReadout();
    testTinySaveRoundTripRestoresWageRecordSpoofed();
    testClinicAccessLedgerRequiresWorkerRecord();
    testClinicAccessLedgerShowsFlaggedWorkerRecord();
    testClinicAccessLedgerSpoofTogglesReadouts();
    testWorkerScanShowsClinicAccessMismatchWhenClinicSpoofed();
    testClinicAccessSpoofPreservesWorkerSuspicionAndWageState();
    testWorldPhaseModulatesOutputTheftWitnessRange();
    testWorldPhaseUsesElapsedTimeNotFrames();
    testTwoWorkersCreateCrowdCamouflageAndDistinctRoutes();
    testTinySaveRoundTripRestoresWorldPhaseAndTwoWorkers();
    testLayLowConsumesSupplyAndPreservesRecord();
    testLayLowFailureCasesAndNightPhase();
    testTransitStationsRequireTwoDistrictConfig();
    testTransitRideLookOutWindowCarriesObjectToDestination();
    testTransitRideCanWaitForDoorsThenExit();
    testTinySaveRoundTripRestoresTransitInterior();
    testPerDistrictWorkersSpawnOnLocalLaborRoutes();
    testDistrictTagsAppearOnInspectionLabels();
    testDistrictSuspicionAndWageReadoutsDoNotLeak();
    testDistrictDependencyDisruptionDoesNotLeak();
    testTinySaveRoundTripRestoresTwoDistrictWorkersAndRecords();
    testTinySaveRoundTripRestoresPerDistrictInterferenceBoundary();
    return 0;
}
