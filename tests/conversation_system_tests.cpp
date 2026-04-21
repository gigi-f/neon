#include <cassert>
#include <iostream>
#include <string>

#include "simulation_systems.h"

static Entity makeCitizen(Registry& registry, float x, float y) {
    Entity e = registry.create();
    registry.assign<CitizenComponent>(e);
    registry.assign<TransformComponent>(e, x, y, 8.0f, 8.0f);
    registry.assign<CognitiveComponent>(e);
    registry.assign<RelationshipComponent>(e);
    registry.assign<ConversationComponent>(e);
    registry.assign<BiologyComponent>(e);
    registry.assign<EconomicComponent>(e);
    registry.assign<ScheduleComponent>(e);
    return e;
}

static void setAffinity(Registry& registry, Entity from, Entity to, float affinity) {
    auto& rel = registry.get<RelationshipComponent>(from);
    int idx = rel.find(to);
    if (idx >= 0) {
        rel.entries[idx].affinity = affinity;
        return;
    }
    assert(rel.count < RelationshipComponent::CAP);
    rel.entries[rel.count++] = {to, affinity};
}

static void testConversationCooldownBlocksImmediateRepeat() {
    Registry registry;
    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 500.0f, 500.0f, 16.0f, 16.0f);

    Entity a = makeCitizen(registry, 0.0f, 0.0f);
    Entity b = makeCitizen(registry, 10.0f, 0.0f);
    setAffinity(registry, a, b, 0.4f);
    setAffinity(registry, b, a, 0.4f);

    ConversationSystem system;
    system.update(registry, player, 0.5f, 9.0f, nullptr);

    const auto& cogA1 = registry.get<CognitiveComponent>(a);
    const auto& cogB1 = registry.get<CognitiveComponent>(b);
    assert(cogA1.mem_size == 1);
    assert(cogB1.mem_size == 1);
    assert(registry.has<SpeechBubbleComponent>(a));
    assert(registry.has<SpeechBubbleComponent>(b));
    assert(registry.get<SpeechBubbleComponent>(a).ttl > 0.0f);
    assert(!registry.get<SpeechBubbleComponent>(a).text.empty());
    assert(registry.get<SpeechBubbleComponent>(a).text.find("TALK:") == std::string::npos);
    assert(registry.get<SpeechBubbleComponent>(a).text.find("EAVESDROP:") == std::string::npos);
    assert(cogA1.memory[(cogA1.mem_head + CognitiveComponent::MEM_CAP - 1) % CognitiveComponent::MEM_CAP].event == MemoryEventType::HEARD_RUMOR);
    assert(cogB1.memory[(cogB1.mem_head + CognitiveComponent::MEM_CAP - 1) % CognitiveComponent::MEM_CAP].event == MemoryEventType::HEARD_RUMOR);

    system.update(registry, player, 0.5f, 9.1f, nullptr);
    const auto& cogA2 = registry.get<CognitiveComponent>(a);
    const auto& cogB2 = registry.get<CognitiveComponent>(b);
    assert(cogA2.mem_size == 1);
    assert(cogB2.mem_size == 1);

    for (int i = 0; i < 8; ++i) {
        system.update(registry, player, 0.5f, 9.2f + i * 0.5f, nullptr);
    }
    assert(registry.get<SpeechBubbleComponent>(a).ttl == 0.0f);
    assert(registry.get<SpeechBubbleComponent>(a).text.empty());
    assert(registry.get<SpeechBubbleComponent>(b).ttl == 0.0f);
    assert(registry.get<SpeechBubbleComponent>(b).text.empty());
}

static void testNearbyPlayerReceivesIntelAlert() {
    Registry registry;
    Entity player = registry.create();
    registry.assign<PlayerComponent>(player);
    registry.assign<TransformComponent>(player, 0.0f, 0.0f, 16.0f, 16.0f);

    Entity a = makeCitizen(registry, 5.0f, 0.0f);
    Entity b = makeCitizen(registry, 12.0f, 0.0f);
    setAffinity(registry, a, b, 0.4f);
    setAffinity(registry, b, a, 0.4f);

    SimulationAlertStack alerts(8);
    ConversationSystem system;
    system.update(registry, player, 0.5f, 10.0f, &alerts);

    auto visible = alerts.visible(4);
    assert(!visible.empty());
    assert(visible[0].category == SimulationAlertCategory::INTEL);
    assert(visible[0].message.find("EAVESDROP:") != std::string::npos);
}

int main() {
    testConversationCooldownBlocksImmediateRepeat();
    testNearbyPlayerReceivesIntelAlert();
    std::cout << "conversation_system_tests passed\n";
    return 0;
}
