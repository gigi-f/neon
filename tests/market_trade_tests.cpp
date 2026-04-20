#include <cassert>
#include <cmath>
#include "ecs.h"
#include "components.h"
#include "simulation_systems.h"

static Entity makePlayer(Registry& registry) {
    Entity player = registry.create();
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 8.0f, 8.0f);
    registry.assign<EconomicComponent>(player);
    registry.assign<SurvivalInventoryComponent>(player);
    registry.assign<DiscreteInventoryComponent>(player);
    return player;
}

static Entity makeMarket(Registry& registry) {
    Entity market = registry.create();
    registry.assign<TransformComponent>(market, 10.0f, 0.0f, 16.0f, 16.0f);
    auto& mc = registry.assign<MarketComponent>(market);
    mc.food_stock = 10.0f;
    mc.water_stock = 8.0f;
    mc.medical_stock = 4.0f;
    mc.max_stock = 20.0f;
    mc.base_price = 5.0f;
    return market;
}

static void testPlayerBuyAddsInventoryAndConsumesStock() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);

    auto preview = previewPlayerMarketBuy(registry, player, ItemComponent::FOOD);
    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(preview.available);
    assert(traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 90.0f) < 0.001f);
    assert(std::fabs(registry.get<MarketComponent>(market).food_stock - 9.0f) < 0.001f);
    assert(registry.get<SurvivalInventoryComponent>(player).food_count == 1);

    auto inspected = inspectSelectedInventoryItem(registry.get<DiscreteInventoryComponent>(player));
    assert(inspected.present);
    assert(inspected.type == ItemComponent::FOOD);
}

static void testPlayerBuyFailsWhenInventoryFull() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    for (auto& slot : inventory.slots) {
        slot.occupied = true;
        slot.type = ItemComponent::FOOD;
    }

    auto preview = previewPlayerMarketBuy(registry, player, ItemComponent::WATER);
    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(!preview.available);
    assert(!traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 100.0f) < 0.001f);
    assert(std::fabs(registry.get<MarketComponent>(market).water_stock - 8.0f) < 0.001f);
}

static void testPlayerSellRemovesInventoryAndPaysCredits() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    assert(storeInventoryItem(inventory, ItemComponent::WATER, 40.0f));
    registry.get<SurvivalInventoryComponent>(player).water_count = 1;

    auto preview = previewPlayerMarketSell(registry, player);
    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(preview.available);
    assert(traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 106.25f) < 0.001f);
    assert(std::fabs(registry.get<MarketComponent>(market).water_stock - 9.0f) < 0.001f);
    assert(registry.get<SurvivalInventoryComponent>(player).water_count == 0);
    assert(!inspectSelectedInventoryItem(inventory).present);
}

static void testPlayerBuyPriceIncludesGreedAndReputation() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);
    registry.get<MarketComponent>(market).greed_margin = 0.2f;
    registry.get<EconomicComponent>(player).market_reputation = 1.0f;

    auto preview = previewPlayerMarketBuy(registry, player, ItemComponent::FOOD);
    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(preview.available);
    assert(std::fabs(preview.base_price - 10.0f) < 0.001f);
    assert(std::fabs(preview.price - 10.2f) < 0.001f);
    assert(traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 89.8f) < 0.001f);
}

static void testPlayerSellPayoutIncludesGreedAndReputation() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);
    registry.get<MarketComponent>(market).greed_margin = 0.2f;
    registry.get<EconomicComponent>(player).market_reputation = 1.0f;
    auto& inventory = registry.get<DiscreteInventoryComponent>(player);
    assert(storeInventoryItem(inventory, ItemComponent::WATER, 40.0f));
    registry.get<SurvivalInventoryComponent>(player).water_count = 1;

    auto preview = previewPlayerMarketSell(registry, player);
    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(preview.available);
    assert(std::fabs(preview.base_price - 12.5f) < 0.001f);
    assert(std::fabs(preview.price - 6.46875f) < 0.001f);
    assert(traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 106.46875f) < 0.001f);
}

static void testConfirmRepricesCurrentMarketTerms() {
    Registry registry;
    Entity player = makePlayer(registry);
    Entity market = makeMarket(registry);

    auto preview = previewPlayerMarketBuy(registry, player, ItemComponent::FOOD);
    registry.get<MarketComponent>(market).greed_margin = 0.5f;
    registry.get<EconomicComponent>(player).credits = 12.0f;

    bool traded = executePlayerMarketTrade(registry, player, preview);

    assert(preview.available);
    assert(!traded);
    assert(std::fabs(registry.get<EconomicComponent>(player).credits - 12.0f) < 0.001f);
    assert(std::fabs(registry.get<MarketComponent>(market).food_stock - 10.0f) < 0.001f);
}

int main() {
    testPlayerBuyAddsInventoryAndConsumesStock();
    testPlayerBuyFailsWhenInventoryFull();
    testPlayerSellRemovesInventoryAndPaysCredits();
    testPlayerBuyPriceIncludesGreedAndReputation();
    testPlayerSellPayoutIncludesGreedAndReputation();
    testConfirmRepricesCurrentMarketTerms();
    return 0;
}
