#include <cassert>
#include "simulation_systems.h"

static void testFeedbackCueBoundsAndExpiry() {
    FeedbackState feedback(2);
    feedback.push(FeedbackCue::SCAN, 1.0f);
    feedback.push(FeedbackCue::PICKUP, 1.0f);
    feedback.push(FeedbackCue::DENIED, 1.0f);

    assert(feedback.size() == 2);
    assert(!feedback.hasRecent(FeedbackCue::SCAN));
    assert(feedback.hasRecent(FeedbackCue::PICKUP));
    assert(feedback.hasRecent(FeedbackCue::DENIED));

    feedback.update(1.1f);
    assert(feedback.recent(4).empty());
}

static void testWarningTriggersFlash() {
    FeedbackState feedback;
    feedback.push(FeedbackCue::WARNING);

    assert(feedback.flashActive());
    SDL_Color flash = feedback.currentFlashColor();
    assert(flash.a > 0);

    feedback.update(1.0f);
    assert(!feedback.flashActive());
}

static void testImpactTriggersShakeAndDecays() {
    FeedbackState feedback;
    feedback.push(FeedbackCue::IMPACT);

    assert(feedback.shakeActive());
    int initialY = feedback.shakeOffsetY();
    assert(initialY != 0);

    feedback.update(0.2f);
    assert(!feedback.shakeActive());
    assert(feedback.shakeOffsetX() == 0);
    assert(feedback.shakeOffsetY() == 0);
}

static void testHazardDamageRecordsCueAndStrongFlash() {
    FeedbackState feedback;
    feedback.push(FeedbackCue::HAZARD_DAMAGE);

    assert(feedback.hasRecent(FeedbackCue::HAZARD_DAMAGE));
    assert(feedback.shakeActive());
    assert(feedback.flashActive());
    SDL_Color flash = feedback.currentFlashColor();
    assert(flash.r > flash.g);
    assert(flash.a >= 100);
}

int main() {
    testFeedbackCueBoundsAndExpiry();
    testWarningTriggersFlash();
    testImpactTriggersShakeAndDecays();
    testHazardDamageRecordsCueAndStrongFlash();
    return 0;
}
