#ifndef MELEE_SLASH_H
#define MELEE_SLASH_H
#include "src/enemies/bloodless_ghoul.h"
#include "assets/sprites/effects/sword-slash.h"
#include "assets/sprites/effects/charge-slash.h"
#include "assets/sprites/effects/monster-slash.h"

// Visual size only; damage and parry ranges remain configured on each actor.
#define CHARGE_SLASH_VISUAL_SCALE 0.75f

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
    PlayerDirection direction, const uint16_t *sprite, int width, int height, float scaleX, float scaleY, float offset)
{
    float ux = (direction == PLAYER_RIGHT) - (direction == PLAYER_LEFT);
    float uy = (direction == PLAYER_DOWN) - (direction == PLAYER_UP);
    float cx = x + ACTOR_HALF_SIZE + ux * offset;
    float cy = y + ACTOR_HALF_SIZE + uy * offset;
    // Sample at the on-screen resolution, including large exported effect sheets.
    int drawWidth = (int)ceilf(width * scaleX), drawHeight = (int)ceilf(height * scaleY);
    for (int py = 0; py < drawHeight; ++py)
        for (int px = 0; px < drawWidth; ++px) {
            uint16_t c = sprite[(py * height / drawHeight) * width + px * width / drawWidth];
            if (c == 0x07E0) continue;
            float dx = px - drawWidth / 2.0f + 0.5f, dy = py - drawHeight / 2.0f + 0.5f;
            float pw = 1, ph = 1;
            SDL_FRect pixel = {cx + ux * dx - uy * dy - pw / 2,
                cy + uy * dx + ux * dy - ph / 2, pw, ph};
            SDL_SetRenderDrawColor(renderer, ((c >> 11) & 31) * 255 / 31,
                ((c >> 5) & 63) * 255 / 63, (c & 31) * 255 / 31, 255);
            SDL_RenderFillRect(renderer, &pixel);
        }
}
static inline void MeleeSlash_Player(SDL_Renderer *r, const Player *p)
{
    if (p->hp <= 0 || !p->isAttacking) return;
    int frame = MeleeSlash_Frame(p->currentFrame * MELEE_ACTOR_FRAME_TIME + p->animTimer,
        p->chargedAttack ? fire_slash_frames_duration_ms : sword_slash_frames_duration_ms,
        p->chargedAttack ? FIRE_SLASH_FRAMES_COUNT : SWORD_SLASH_FRAMES_COUNT);
    if (frame >= 0) MeleeSlash_Draw(r, p->x, p->y, p->direction,
        p->chargedAttack ? fire_slash_frames[frame] : sword_slash_frames[frame],
        p->chargedAttack ? FIRE_SLASH_WIDTH : SWORD_SLASH_WIDTH,
        p->chargedAttack ? FIRE_SLASH_HEIGHT : SWORD_SLASH_HEIGHT,
        p->chargedAttack ? PLAYER_CHARGE_REACH * CHARGE_SLASH_VISUAL_SCALE / FIRE_SLASH_WIDTH : 1,
        p->chargedAttack ? PLAYER_CHARGE_WIDTH * CHARGE_SLASH_VISUAL_SCALE / FIRE_SLASH_HEIGHT : 1,
        p->chargedAttack ? PLAYER_CHARGE_REACH / 2 : ACTOR_HALF_SIZE);
}
static inline void MeleeSlash_Ghoul(SDL_Renderer *r, const Ghoul *g)
{
    if (g->hp <= 0 || g->state != GHOUL_ATTACK) return;
    int frame = MeleeSlash_Frame(g->timer - GHOUL_ATTACK_WINDUP,
        monster_slash_frames_duration_ms, MONSTER_SLASH_FRAMES_COUNT);
    if (frame >= 0) MeleeSlash_Draw(r, g->x, g->y, g->direction,
        monster_slash_frames[frame], MONSTER_SLASH_WIDTH, MONSTER_SLASH_HEIGHT, 1, 1, ACTOR_HALF_SIZE);
}
#endif
