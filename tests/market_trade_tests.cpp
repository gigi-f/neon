#include <cassert>
#include <cstdio>
#include <string>

#include "ai_playtest.h"

// ---------------------------------------------------------------------------
// Phase 99: Market category labels
// ---------------------------------------------------------------------------

static WorldConfig makeMarketCategoryTestConfig(int macro_count_x) {
    WorldConfig config = makeSandboxConfig();
    config.macro_count_x = macro_count_x;
    config.market_micro_zone_count = 1;
    config.market_building_count = 1;
    return config;
}

static void assertMarketCategoriesMatchDistricts(int macro_count_x) {
    Registry registry;
    WorldConfig config = makeMarketCategoryTestConfig(macro_count_x);
    buildWorld(registry, config);
    assert(validateWorld(registry, config));

    int found_market_count = 0;
    auto buildings = registry.view<BuildingUseComponent, MarketLedgerComponent>();
    for (Entity e : buildings) {
        if (registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            const uint32_t district_id = districtIdForEntity(registry, e);
            const std::string expected = marketCategoryForDistrict(district_id);
            assert(std::string(registry.get<MarketLedgerComponent>(e).category) == expected &&
                   "Market category must be derived from its district");
            ++found_market_count;
        }
    }
    assert(found_market_count == macro_count_x &&
           "Each configured district must expose one authored market category");
}

static void testMarketBuildingHasMarketLedgerComponent() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    bool found = false;
    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            found = true;
        }
    }
    assert(found && "MARKET building must have MarketLedgerComponent");
}

static void testMarketCategoryAssignedFromDistrict() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            const auto& ledger = session.registry.get<MarketLedgerComponent>(e);
            assert(ledger.category != nullptr && ledger.category[0] != '\0');
            const std::string cat = ledger.category;
            assert((cat == "RATION DEPOT" || cat == "LOCAL GOODS" || cat == "LUXURY PREVIEW") &&
                   "Category must be one of the three known labels");
        }
    }
}

static void testMarketCategoryForDistrictCyclesThroughThreeLabels() {
    assert(std::string(marketCategoryForDistrict(0)) == "RATION DEPOT");
    assert(std::string(marketCategoryForDistrict(1)) == "LOCAL GOODS");
    assert(std::string(marketCategoryForDistrict(2)) == "LUXURY PREVIEW");
    assert(std::string(marketCategoryForDistrict(3)) == "RATION DEPOT");
    assert(std::string(marketCategoryForDistrict(4)) == "LOCAL GOODS");
}

static void testSingleDistrictConfigGivesRationDepotCategory() {
    assertMarketCategoriesMatchDistricts(1);
}

static void testTwoDistrictConfigGivesStableMarketCategories() {
    assertMarketCategoriesMatchDistricts(2);
}

static void testMarketInspectionReadoutShowsCategory() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string warp_result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &warp_result));

    // Determine the expected category from the warped-to market entity
    const Entity market = nearestMarketBuildingInRange(
        session.registry,
        session.registry.get<TransformComponent>(session.player),
        kAiPlaytestInteractionRangeWu);
    assert(market != MAX_ENTITIES);
    const std::string expected_category =
        session.registry.get<MarketLedgerComponent>(market).category;

    applyAiPlaytestKey(session, "SPACE");
    const std::string snapshot = aiPlaytestSnapshot(session);

    assert(snapshot.find("CATEGORY:") != std::string::npos);
    assert(snapshot.find(expected_category) != std::string::npos);
    assert(snapshot.find("ACCESS PRESSURE: LOCAL ONLY") != std::string::npos);
    assert(snapshot.find("CUSTOMERS: DISTRICT RESIDENTS") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Phase 100: Market exchange verb
// ---------------------------------------------------------------------------

static void testPlayerCanExchangeAtMarketWhenNearMarket() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(playerCanExchangeAtMarket(session.registry, session.player,
                                     kAiPlaytestInteractionRangeWu));
}

static void testActionLineShowsExchangeAtMarketPrompt() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));

    const std::string action = aiPlaytestActionLine(session.registry, session.player);
    assert(action.find("E EXCHANGE AT MARKET") != std::string::npos);
}

static void testExchangeNoItemRecordsDeferredClaim() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));

    // Player carries nothing by default after warp
    auto& player_comp = session.registry.get<PlayerComponent>(session.player);
    assert(player_comp.carried_object == MAX_ENTITIES);

    assert(applyAiPlaytestKey(session, "E", &result));
    assert(result == "KEY E OK");

    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    bool found_result = false;
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            const auto& ledger = session.registry.get<MarketLedgerComponent>(e);
            if (ledger.last_exchange_result == "NO ITEM: CLAIM DEFERRED") {
                found_result = true;
            }
        }
    }
    assert(found_result && "Exchange with no item must record CLAIM DEFERRED");
}

static void testExchangeWithPartConvertsToSupply() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;

    // Give player a PART
    Entity item = session.registry.create();
    session.registry.assign<TransformComponent>(item, 0.0f, 0.0f, 8.0f, 8.0f);
    session.registry.assign<CarryableComponent>(item);
    session.registry.get<CarryableComponent>(item).kind = ItemKind::PART;
    session.registry.assign<GlyphComponent>(item, std::string("*"),
        static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255),
        static_cast<uint8_t>(255), 0.8f, true, false);
    session.registry.get<PlayerComponent>(session.player).carried_object = item;

    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    // Item kind must now be SUPPLY
    assert(carryableObjectIsKind(session.registry, item, ItemKind::SUPPLY));

    // Ledger must record the result
    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    bool found_result = false;
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            if (session.registry.get<MarketLedgerComponent>(e).last_exchange_result == "PART -> SUPPLY") {
                found_result = true;
            }
        }
    }
    assert(found_result && "Exchange with PART must record PART -> SUPPLY");
}

static void testExchangeWithSupplyMarksRationClaimed() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;

    // Give player a SUPPLY item
    Entity item = session.registry.create();
    session.registry.assign<TransformComponent>(item, 0.0f, 0.0f, 8.0f, 8.0f);
    session.registry.assign<CarryableComponent>(item);
    session.registry.get<CarryableComponent>(item).kind = ItemKind::SUPPLY;
    session.registry.assign<GlyphComponent>(item, std::string("*"),
        static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255),
        static_cast<uint8_t>(255), 0.8f, true, false);
    session.registry.get<PlayerComponent>(session.player).carried_object = item;

    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    bool claimed = false;
    bool has_result = false;
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            const auto& ledger = session.registry.get<MarketLedgerComponent>(e);
            if (ledger.exchange_claimed) claimed = true;
            if (ledger.last_exchange_result == "SUPPLY: RATION CLAIMED") has_result = true;
        }
    }
    assert(claimed && "Exchange with SUPPLY must set exchange_claimed");
    assert(has_result && "Exchange with SUPPLY must record RATION CLAIMED");
}

static void testExchangeWithNonCarryableObjectReportsWrongItem() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;

    Entity object = session.registry.create();
    session.registry.assign<TransformComponent>(object, 0.0f, 0.0f, 8.0f, 8.0f);
    session.registry.get<PlayerComponent>(session.player).carried_object = object;

    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    bool has_result = false;
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET &&
            session.registry.get<MarketLedgerComponent>(e).last_exchange_result ==
                "WRONG ITEM: MARKET ACCEPTS PART OR SUPPLY") {
            has_result = true;
        }
    }
    assert(has_result && "Exchange with a non-carryable object must say why it failed");
}

static void testExchangeRecordsInspectionReadoutAfterExchange() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    // Now inspect the market
    applyAiPlaytestKey(session, "SPACE");
    const std::string snapshot = aiPlaytestSnapshot(session);

    assert(snapshot.find("LAST EXCHANGE:") != std::string::npos);
    assert(snapshot.find("(VOLATILE)") != std::string::npos);
}

static void testGadgetResultAfterExchange() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    const std::string snapshot = aiPlaytestSnapshot(session);
    assert(snapshot.find("ACTION RESULT: ON MARKET") != std::string::npos);
    assert(snapshot.find("MARKET EXCHANGE:") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Phase 101: Persistence boundary — exchange result is NOT saved
// ---------------------------------------------------------------------------

static void testExchangeResultIsNotPersistedAcrossSaveLoad() {
    AiPlaytestSession session;
    assert(buildAiPlaytestSession(session));

    std::string result;
    assert(warpAiPlaytestPlayer(session, "MARKET", &result));
    assert(applyAiPlaytestKey(session, "E", &result));

    // Confirm result was recorded in memory
    auto buildings = session.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    bool has_result_before_save = false;
    for (Entity e : buildings) {
        if (session.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            if (!session.registry.get<MarketLedgerComponent>(e).last_exchange_result.empty()) {
                has_result_before_save = true;
            }
        }
    }
    assert(has_result_before_save);

    // Save and reload via the AI playtest session helpers
    const std::string tmp_path = "/tmp/neon_market_trade_test.sav";
    assert(saveAiPlaytestSession(session, tmp_path));

    AiPlaytestSession session2;
    std::string load_error;
    assert(loadAiPlaytestSession(session2, tmp_path, &load_error));

    // Exchange result must be absent after reload (it is volatile, not part of TinySaveState)
    auto buildings2 = session2.registry.view<BuildingUseComponent, MarketLedgerComponent>();
    for (Entity e : buildings2) {
        if (session2.registry.get<BuildingUseComponent>(e).role == MicroZoneRole::MARKET) {
            assert(session2.registry.get<MarketLedgerComponent>(e).last_exchange_result.empty() &&
                   "Exchange result must not survive save/load");
        }
    }
}

// ---------------------------------------------------------------------------
// Runner
// ---------------------------------------------------------------------------

int main() {
    testMarketBuildingHasMarketLedgerComponent();
    testMarketCategoryAssignedFromDistrict();
    testMarketCategoryForDistrictCyclesThroughThreeLabels();
    testSingleDistrictConfigGivesRationDepotCategory();
    testTwoDistrictConfigGivesStableMarketCategories();
    testMarketInspectionReadoutShowsCategory();
    testPlayerCanExchangeAtMarketWhenNearMarket();
    testActionLineShowsExchangeAtMarketPrompt();
    testExchangeNoItemRecordsDeferredClaim();
    testExchangeWithPartConvertsToSupply();
    testExchangeWithSupplyMarksRationClaimed();
    testExchangeWithNonCarryableObjectReportsWrongItem();
    testExchangeRecordsInspectionReadoutAfterExchange();
    testGadgetResultAfterExchange();
    testExchangeResultIsNotPersistedAcrossSaveLoad();

    printf("All market trade tests passed.\n");
    return 0;
}
