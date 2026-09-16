#ifndef ENEMY_COMMON_H
#define ENEMY_COMMON_H
#include "src/player/player.h"
#include <SDL3/SDL.h>
#include <math.h>

static inline bool Enemy_Animate(float *timer, int *frame, float dt,
                                const uint32_t *durations, int count, bool loop)
{
    *timer += dt;
    while (*timer >= durations[*frame] / 1000.0f) {
        *timer -= durations[*frame] / 1000.0f;
        if (*frame + 1 == count) {
            if (!loop) { *timer = 0; return true; }
            *frame = 0;
        } else ++*frame;
    }
    return false;
}

static inline PlayerDirection Enemy_Face(float dx, float dy)
{
    return fabsf(dx) > fabsf(dy) ? (dx < 0 ? PLAYER_LEFT : PLAYER_RIGHT)
                                : (dy < 0 ? PLAYER_UP : PLAYER_DOWN);
}

static inline void Enemy_HurtPlayer(Player *p, int damage, float x, float y)
{
    if (p->hp <= 0 || damage <= 0 || p->invincibilityTimer > 0) return;
    p->invincibilityTimer = PLAYER_INVINCIBILITY_TIME;
    p->hp = p->hp > damage ? p->hp - damage : 0;
    p->hitFlashTimer = PLAYER_HIT_FLASH_TIME;
    p->collisionGraceTimer = PLAYER_COLLISION_GRACE_TIME;
    float dx = p->x + ACTOR_HALF_SIZE - x, dy = p->y + ACTOR_HALF_SIZE - y;
    float length = sqrtf(dx * dx + dy * dy);
    if (length > 0.001f) {
        p->x += dx / length * PLAYER_HIT_KNOCKBACK;
        p->y += dy / length * PLAYER_HIT_KNOCKBACK;
    }
}
#endif
