#ifndef EXPLOSION_ANIMATION_H
#define EXPLOSION_ANIMATION_H
#include "assets/sprites/effects/arrow-explosion.h"
#include "assets/sprites/effects/eye_parasite_flesh-burst.h"

static inline float Explosion_Duration(const uint32_t *durations, int count)
{
    float total = 0;
    for (int i = 0; i < count; ++i) total += durations[i] / 1000.0f;
    return total;
}

// Existing combat timers count down; play every exported frame once.
static inline int Explosion_Frame(float remaining, const uint32_t *durations, int count)
{
    if (remaining <= 0) return -1;
    float elapsed = Explosion_Duration(durations, count) - remaining;
    for (int i = 0; i < count; ++i) {
        float duration = durations[i] / 1000.0f;
        if (elapsed < duration - 0.00001f) return i;
        elapsed -= duration;
    }
    return count - 1;
}
#define ARROW_EXPLOSION_TIME Explosion_Duration(arrow_explosion_8frames_dark_frames_duration_ms, ARROW_EXPLOSION_8FRAMES_DARK_FRAMES_COUNT)
#define EYE_EXPLOSION_TIME Explosion_Duration(flesh_burst_frames_duration_ms, FLESH_BURST_FRAMES_COUNT)
#endif
