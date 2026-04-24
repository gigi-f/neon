#pragma once

#include <cstdint>
#include "components.h"

struct AabbRect {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

inline AabbRect aabbFromTransform(const TransformComponent& t) {
    return {t.x - t.width * 0.5f,
            t.y - t.height * 0.5f,
            t.x + t.width * 0.5f,
            t.y + t.height * 0.5f};
}

inline bool aabbOverlap(const AabbRect& a, const AabbRect& b) {
    return a.left < b.right && a.right > b.left &&
           a.top < b.bottom && a.bottom > b.top;
}

inline uint32_t deterministicStep(uint32_t state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}
