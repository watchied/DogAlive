#ifndef RUNNING_EFFECT_H
#define RUNNING_EFFECT_H

#include "src/player/player.h"
#include "assets/sprites/effects/running.h"

#define RUNNING_EFFECT_CAPACITY 2
#define RUNNING_EFFECT_INTERVAL 0.15f

typedef struct {
    float x, y, timer;
    int frame;
    bool active;
    bool flipX;
    bool vertical;
    bool diagonal;
    bool flipY;
} RunningPuff;

typedef struct {
    RunningPuff puffs[RUNNING_EFFECT_CAPACITY];
    float spawnTimer;
    int nextPuff;
} RunningEffect;

static inline void RunningEffect_Update(RunningEffect *effect, const Player *p, float dt)
{
    for (int i = 0; i < RUNNING_EFFECT_CAPACITY; ++i) {
        RunningPuff *puff = &effect->puffs[i];
        if (!puff->active) continue;
        const uint32_t *durations = puff->diagonal
            ? running_ort_running_duration_ms : puff->vertical
            ? running_up_running_duration_ms : running_side_running_duration_ms;
        int frameCount = puff->diagonal
            ? RUNNING_ORT_RUNNING_COUNT : puff->vertical
            ? RUNNING_UP_RUNNING_COUNT : RUNNING_SIDE_RUNNING_COUNT;
        puff->timer += dt;
        while (puff->timer >= durations[puff->frame] / 1000.0f) {
            puff->timer -= durations[puff->frame] / 1000.0f;
            if (++puff->frame >= frameCount) {
                puff->active = false;
                break;
            }
        }
    }
    if (!p->isSprinting || !p->isMoving || p->hp <= 0 || p->isAttacking || p->isShooting) {
        effect->spawnTimer = 0.0f;
        return;
    }
    effect->spawnTimer -= dt;
    if (effect->spawnTimer > 0.0f) return;
    effect->spawnTimer = RUNNING_EFFECT_INTERVAL;
    float dx = (float)p->isMovingRight - (float)p->isMovingLeft;
    float dy = (float)p->isMovingDown - (float)p->isMovingUp;
    float scale = dx != 0 && dy != 0 ? 0.70710678f : 1.0f;
    // The diagonal source is drawn for up-right movement.
    bool diagonal = dx != 0 && dy != 0;
    bool vertical = !diagonal && dy != 0;
    {
        // Recycle the oldest slot to keep a continuous trail of two puffs.
        int i = effect->nextPuff;
        effect->nextPuff = (effect->nextPuff + 1) % RUNNING_EFFECT_CAPACITY;
        // Anchor the bottom of the sprite at the feet, slightly behind motion.
        effect->puffs[i] = (RunningPuff){
            .x = p->x + ACTOR_HALF_SIZE - RUNNING_WIDTH / 2.0f - dx * scale * 5.0f,
            .y = p->y + ACTOR_SIZE - RUNNING_HEIGHT - dy * scale * 5.0f,
            .active = true, .flipX = !vertical && dx < 0,
            .vertical = vertical, .diagonal = diagonal,
            .flipY = diagonal && dy > 0
        };
    }
}

static inline const uint16_t *RunningEffect_GetSprite(const RunningPuff *puff)
{
    if (puff->diagonal)
        return running_ort_running[puff->frame];
    return puff->vertical ? running_up_running[puff->frame]
                          : running_side_running[puff->frame];
}

#endif
