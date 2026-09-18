#ifndef SLIME_KING_DRAW_H
#define SLIME_KING_DRAW_H
#include "src/enemies/slime_king.h"
#include "assets/sprites/projectiles/king_slime_bubble.h"
#include "assets/sprites/projectiles/king_slime_laser.h"
#include "assets/sprites/projectiles/king_slime_beam.h"

static inline int King_VisualFrame(float time, float duration, int count)
{
    int frame = duration > 0 ? (int)(time / duration * count) : count - 1;
    return frame < 0 ? 0 : frame >= count ? count - 1 : frame;
}
static inline const uint16_t *King_Sprite(const SlimeKing *k)
{
    int frame;
    switch (k->state) {
    case KING_INTRO:
        return slime_king_boss_opening1[King_VisualFrame(k->timer, KING_INTRO_TIME, SLIME_KING_BOSS_OPENING1_COUNT)];
    case KING_PHASE_CHANGE:
        return slime_king_boss_opening2[King_VisualFrame(k->timer, KING_PHASE_TRANSITION_TIME, SLIME_KING_BOSS_OPENING2_COUNT)];
    case KING_DEATH: return slime_king_boss_death[k->frame];
    case KING_BUBBLE:
        if (k->phase == 2) return slime_king_boss_sprite_2boss_attack1[King_VisualFrame(k->timer, KING_BUBBLE_WINDUP, SLIME_KING_BOSS_SPRITE_2BOSS_ATTACK1_COUNT)];
        return slime_king_boss_boss_attack1[King_VisualFrame(k->timer, KING_BUBBLE_WINDUP, SLIME_KING_BOSS_BOSS_ATTACK1_COUNT)];
    case KING_DASH_READY: case KING_DASH:
        frame = King_VisualFrame(k->state == KING_DASH ? KING_DASH_WINDUP : k->timer, KING_DASH_WINDUP,
            k->phase == 1 ? SLIME_KING_BOSS_BOSS_ATTACK2_COUNT : SLIME_KING_BOSS_SPRITE_2BOSS_ATTACK2_COUNT);
        return k->phase == 1 ? slime_king_boss_boss_attack2[frame] : slime_king_boss_sprite_2boss_attack2[frame];
    case KING_BURROW:
        return slime_king_boss_boss_attack3_1[King_VisualFrame(k->timer, KING_BURROW_TIME, SLIME_KING_BOSS_BOSS_ATTACK3_1_COUNT)];
    case KING_SLAM_FALL: return slime_king_boss_boss_attack3_2[0];
    case KING_SLAM_LAND:
        return slime_king_boss_boss_attack3_2[King_VisualFrame(k->timer, KING_SLAM_RECOVERY, SLIME_KING_BOSS_BOSS_ATTACK3_2_COUNT)];
    case KING_RADIAL:
        return slime_king_boss_sprite_2boss_attack3[King_VisualFrame(k->timer, KING_RADIAL_WINDUP, SLIME_KING_BOSS_SPRITE_2BOSS_ATTACK3_COUNT)];
    case KING_SPIRAL:
        return slime_king_boss_sprite_2boss_attack4[King_VisualFrame(k->timer, KING_SPIRAL_WINDUP + KING_SPIRAL_COUNT * KING_SPIRAL_INTERVAL, SLIME_KING_BOSS_SPRITE_2BOSS_ATTACK4_COUNT)];
    case KING_BEAM_CHARGE: case KING_BEAM_FIRE:
        return slime_king_boss_sprite_2boss_attack5[King_VisualFrame(k->state == KING_BEAM_FIRE ? KING_BEAM_CHARGE_TIME : k->timer,
            KING_BEAM_CHARGE_TIME, SLIME_KING_BOSS_SPRITE_2BOSS_ATTACK5_COUNT)];
    default: return slime_king_boss_walk[(int)(k->timer / 0.1f) % SLIME_KING_BOSS_WALK_COUNT];
    }
}
// Source art is drawn at native size. Projectile pixels rotate around their center.
static inline void King_DrawPixels(SDL_Renderer *r, float cx, float cy, const uint16_t *pixels,
    int width, int height, float angle, bool flash)
{
    float cosine = cosf(angle), sine = sinf(angle);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uint16_t c = pixels[y * width + x];
            if (c == 0x07E0) continue;
            SDL_SetRenderDrawColor(r, flash ? 255 : ((c >> 11) & 31) * 255 / 31,
                flash ? 255 : ((c >> 5) & 63) * 255 / 63, flash ? 255 : (c & 31) * 255 / 31, 255);
            float dx = x - width / 2.0f + 0.5f, dy = y - height / 2.0f + 0.5f;
            SDL_FRect p = {cx + dx * cosine - dy * sine - 0.5f, cy + dx * sine + dy * cosine - 0.5f, 1, 1};
            SDL_RenderFillRect(r, &p);
        }
}
static inline void King_DrawCircle(SDL_Renderer *r, float x, float y, float radius)
{
    SDL_FPoint points[49];
    for (int i = 0; i <= 48; ++i) {
        float angle = i * 6.28318530718f / 48;
        points[i] = (SDL_FPoint){x + cosf(angle) * radius, y + sinf(angle) * radius};
    }
    SDL_RenderLines(r, points, 49);
}
static inline void King_Draw(SDL_Renderer *r, const SlimeKing *k)
{
    if (!k->active) return;
    SDL_FRect b = King_Body(k);
    float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
    if (k->state == KING_SLAM_MARK || k->state == KING_SLAM_FALL || k->slamFlash > 0) {
        SDL_SetRenderDrawColor(r, 255, k->slamFlash > 0 ? 240 : 80, 40, 255);
        King_DrawCircle(r, k->slamX, k->slamY, KING_SLAM_RADIUS);
        SDL_RenderLine(r, k->slamX - 4, k->slamY, k->slamX + 4, k->slamY);
        SDL_RenderLine(r, k->slamX, k->slamY - 4, k->slamX, k->slamY + 4);
    }
    if (k->state == KING_DASH_READY || k->state == KING_BEAM_CHARGE) {
        SDL_SetRenderDrawColor(r, 255, k->state == KING_BEAM_CHARGE ? 50 : 180, 50, 255);
        SDL_RenderLine(r, cx, cy, cx + k->aimX * GAME_WIDTH, cy + k->aimY * GAME_WIDTH);
    }
    if (k->state == KING_NPC) {
        King_DrawPixels(r, k->x + 24, k->y + 32, slime_king_npc_frames[0], SLIME_KING_NPC_WIDTH, SLIME_KING_NPC_HEIGHT, 0, false);
    } else if (k->state != KING_SLAM_MARK) {
        float lift = k->state == KING_SLAM_FALL ? KING_SLAM_HEIGHT * (1 - fminf(1, k->timer / KING_SLAM_FALL_TIME)) : 0;
        King_DrawPixels(r, k->x + 24, k->y + 24 - lift, King_Sprite(k), SLIME_KING_BOSS_WIDTH, SLIME_KING_BOSS_HEIGHT, 0,
            k->flashTimer > 0);
        if (k->phase == 2 && (k->state == KING_IDLE || k->state == KING_STUN))
            King_DrawPixels(r, k->x + 24, k->y + 24, slime_king_boss_crown[0], SLIME_KING_BOSS_WIDTH, SLIME_KING_BOSS_HEIGHT, 0, k->flashTimer > 0);
    }
    for (int i = 0; i < KING_SHOT_CAPACITY; ++i) {
        const KingShot *s = &k->shots[i];
        if (!s->active) continue;
        const uint16_t *pixels = s->type == KING_SHOT_BUBBLE ? bubble_frames[0] : s->type == KING_SHOT_LASER ? laser_frames[0] : king_slime_beam_frames[0];
        int width = s->type == KING_SHOT_BUBBLE ? BUBBLE_WIDTH : s->type == KING_SHOT_LASER ? LASER_WIDTH : KING_SLIME_BEAM_WIDTH;
        int height = s->type == KING_SHOT_BUBBLE ? BUBBLE_HEIGHT : s->type == KING_SHOT_LASER ? LASER_HEIGHT : KING_SLIME_BEAM_HEIGHT;
        // Laser exports point vertically; rotate that axis into the travel direction.
        King_DrawPixels(r, s->x, s->y, pixels, width, height, s->type == KING_SHOT_BUBBLE ? 0 : atan2f(s->vy, s->vx) + 1.57079632679f, false);
    }
    if (k->state == KING_NPC) return;
    SDL_SetRenderDrawColor(r, 240, 220, 170, 255);
    SDL_RenderDebugTextFormat(r, 72, GAME_HEIGHT - 26, "SLIME KING - PHASE %d", k->phase);
    SDL_FRect bar = {60, GAME_HEIGHT - 14, 200, 5};
    SDL_SetRenderDrawColor(r, 50, 20, 30, 255); SDL_RenderFillRect(r, &bar);
    bar.w *= (float)k->hp / k->maxHP;
    SDL_SetRenderDrawColor(r, 200, 55, 100, 255); SDL_RenderFillRect(r, &bar);
    if (k->state == KING_BEAM_CHARGE) {
        SDL_SetRenderDrawColor(r, 255, 90, 60, 255);
        SDL_RenderDebugText(r, 64, GAME_HEIGHT - 40, "HIT BOSS TO BREAK BEAM!");
    }
}
#endif
