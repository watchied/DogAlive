#ifndef MELEE_SLASH_H
#define MELEE_SLASH_H
#include "src/enemies/bloodless_ghoul.h"
#include "assets/sprites/effects/sword-slash.h"
#include "assets/sprites/effects/monster-slash.h"

static inline int MeleeSlash_Frame(float attackTime, const uint32_t *durations, int count)
{
    float time = attackTime - MELEE_SLASH_START_TIME;
    if (time < -0.00001f) return -1;
    for (int i = 0; i < count; ++i) {
        // First two poses: 50ms each; peak and fade: 25ms each.
        // The complete effect fits the final 200ms of the actor's swing.
        float duration = durations[i] / 1000.0f * (i < 2 ? 5.0f / 6.0f : 5.0f / 12.0f);
        if (time + 0.00001f < duration) return i;
        time -= duration;
    }
    return -1;
}

static inline void MeleeSlash_Draw(SDL_Renderer *renderer, float x, float y,
    PlayerDirection direction, const uint16_t *sprite, int width, int height)
{
    float ux = (direction == PLAYER_RIGHT) - (direction == PLAYER_LEFT);
    float uy = (direction == PLAYER_DOWN) - (direction == PLAYER_UP);
    float cx = x + ACTOR_HALF_SIZE + ux * ACTOR_HALF_SIZE;
    float cy = y + ACTOR_HALF_SIZE + uy * ACTOR_HALF_SIZE;
    for (int py = 0; py < height; ++py)
        for (int px = 0; px < width; ++px) {
            uint16_t c = sprite[py * width + px];
            if (c == 0x07E0) continue;
            float dx = px - width / 2.0f + 0.5f, dy = py - height / 2.0f + 0.5f;
            SDL_FRect pixel = {cx + ux * dx - uy * dy - 0.5f,
                cy + uy * dx + ux * dy - 0.5f, 1, 1};
            SDL_SetRenderDrawColor(renderer, ((c >> 11) & 31) * 255 / 31,
                ((c >> 5) & 63) * 255 / 63, (c & 31) * 255 / 31, 255);
            SDL_RenderFillRect(renderer, &pixel);
        }
}
static inline void MeleeSlash_Player(SDL_Renderer *r, const Player *p)
{
    if (p->hp <= 0 || !p->isAttacking) return;
    int frame = MeleeSlash_Frame(p->currentFrame * MELEE_ACTOR_FRAME_TIME + p->animTimer,
        sword_slash_frames_duration_ms, SWORD_SLASH_FRAMES_COUNT);
    if (frame >= 0) MeleeSlash_Draw(r, p->x, p->y, p->direction,
        sword_slash_frames[frame], SWORD_SLASH_WIDTH, SWORD_SLASH_HEIGHT);
}
static inline void MeleeSlash_Ghoul(SDL_Renderer *r, const Ghoul *g)
{
    if (g->hp <= 0 || g->state != GHOUL_ATTACK) return;
    int frame = MeleeSlash_Frame(g->timer - GHOUL_ATTACK_WINDUP,
        monster_slash_frames_duration_ms, MONSTER_SLASH_FRAMES_COUNT);
    if (frame >= 0) MeleeSlash_Draw(r, g->x, g->y, g->direction,
        monster_slash_frames[frame], MONSTER_SLASH_WIDTH, MONSTER_SLASH_HEIGHT);
}
#endif
