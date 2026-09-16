#ifndef FLEASH_SLIME_H
#define FLEASH_SLIME_H
#define FLEASH_SLIME_MAX_HP 80
#define FLEASH_SLIME_ATTACK_HIT_FRAME 4 // Release the projectile on the final attack pose.
#define FLEASH_SLIME_ATTACK_WINDUP 0.9f
#define FLEASH_SLIME_ATTACK_COOLDOWN 1.5f // Reposition before preparing the next shot.
#include <math.h>
#include <stdbool.h>
#include "src/player/player.h"
#include "assets/sprites/enemies/flesh_slime_sprites.h"
#include <SDL3/SDL.h>
#include "src/enemies/enemy_common.h"
#include "assets/sprites/projectiles/slime_bullet.h"
#define SLIME_SHOT_CAPACITY (16 * ENEMY_TYPE_CAPACITY)
#define SLIME_PROJECTILE_SPEED 50.0f // World pixels per second.
#define FLESH_SLIME_PREFERRED_DISTANCE 100.0f

typedef struct {
    float x, y, vx, vy, lifetime;
    int damage;
    bool active;
    bool reflected;
    float hitFlashTimer;
} SlimeShot;
typedef enum
{
    FLEASH_SLIME_WALK,
    FLEASH_SLIME_ATTACK,
    FLEASH_SLIME_REST,
    FLEASH_SLIME_DEAD
} FleshSlimeState;

typedef struct
{
    float x, y;
    float speed;
    float timer;
    float hitFlashTimer;
    bool deathFinished;
    int frame;
    int attackDamage;
    int hp;
    int maxHP;
    PlayerDirection direction;
    FleshSlimeState state;
    bool hasHit;
} FleshSlime;

static inline void FleshSlime_Init(FleshSlime *f, float x, float y)
{
    *f = (FleshSlime){
        .x = x,
        .y = y,
        .speed = 65.0f / 3.0f,
        .direction = PLAYER_LEFT,
        .state = FLEASH_SLIME_WALK,
        .attackDamage = 20,
        .hp = FLEASH_SLIME_MAX_HP,
        .maxHP = FLEASH_SLIME_MAX_HP,

    };
}
static inline void FleshSlime_TakeDamage(FleshSlime *f, int damage, float px, float py)
{
    if (f->hp <= 0 || damage <= 0) return;
    f->hp = f->hp > damage ? f->hp - damage : 0;
    f->x += px; f->y += py;
    f->hitFlashTimer = 0.08f;
    f->state = f->hp ? FLEASH_SLIME_REST : FLEASH_SLIME_DEAD;
    f->timer = 0; f->frame = 0;
}

static inline void SlimeShots_Update(SlimeShot *shots, Player *p, float dt)
{
    for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
        SlimeShot *s = &shots[i];
        if (!s->active) continue;
        s->hitFlashTimer = fmaxf(0, s->hitFlashTimer - dt);
        if (p->hp <= 0) { s->active = false; continue; }
        float step = fminf(dt, s->lifetime);
        float nx = s->x + s->vx * step, ny = s->y + s->vy * step;
        float x1 = s->x, y1 = s->y, x2 = nx, y2 = ny;
        SDL_FRect body = {p->x - 2, p->y - 2, ACTOR_SIZE + 4, ACTOR_SIZE + 4};
        if (!s->reflected && SDL_GetRectAndLineIntersectionFloat(&body, &x1, &y1, &x2, &y2)) {
            Enemy_HurtPlayer(p, s->damage, s->x, s->y);
            s->active = false;
        }
        s->x = nx; s->y = ny; s->lifetime -= dt;
        if (s->lifetime <= 0) s->active = false;
    }
}

static inline void FleshSlime_Update(FleshSlime *f, Player *p, SlimeShot *shots, float dt)
{
    f->hitFlashTimer = fmaxf(0, f->hitFlashTimer - dt);
    if (f->state == FLEASH_SLIME_DEAD) {
        if (!f->deathFinished)
            f->deathFinished = Enemy_Animate(&f->timer, &f->frame, dt,
                flesh_slime_range_enemy_dead_duration_ms, FLESH_SLIME_RANGE_ENEMY_DEAD_COUNT, false);
        return;
    }
    if (p->hp <= 0) return;
    float dx = p->x - f->x, dy = p->y - f->y;
    float distance = sqrtf(dx * dx + dy * dy);
    f->direction = Enemy_Face(dx, dy);
    if (f->state == FLEASH_SLIME_ATTACK) {
        // Hold the first pose for the windup, then play the exported attack timings.
        f->timer += dt;
        float elapsed = fmaxf(0, f->timer - FLEASH_SLIME_ATTACK_WINDUP);
        float time = elapsed;
        int frame = 0;
        while (frame < FLESH_SLIME_RANGE_ENEMY_ATTACK_COUNT &&
               time >= flesh_slime_range_enemy_attack_duration_ms[frame] / 1000.0f) {
            time -= flesh_slime_range_enemy_attack_duration_ms[frame] / 1000.0f;
            ++frame;
        }
        f->frame = frame < FLESH_SLIME_RANGE_ENEMY_ATTACK_COUNT ? frame : FLESH_SLIME_RANGE_ENEMY_ATTACK_COUNT - 1;
        if (!f->hasHit && f->frame >= FLEASH_SLIME_ATTACK_HIT_FRAME) {
            f->hasHit = true;
            for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
                if (shots[i].active) continue;
                float ux = distance > 0.001f ? dx / distance : 1;
                float uy = distance > 0.001f ? dy / distance : 0;
                shots[i] = (SlimeShot){.x = f->x + ACTOR_HALF_SIZE, .y = f->y + ACTOR_HALF_SIZE,
                    .vx = ux * SLIME_PROJECTILE_SPEED, .vy = uy * SLIME_PROJECTILE_SPEED,
                    .lifetime = 5.0f, .damage = f->attackDamage, .active = true};
                break;
            }
        }
        if (frame == FLESH_SLIME_RANGE_ENEMY_ATTACK_COUNT) {
            f->state = FLEASH_SLIME_REST; f->timer = 0; f->frame = 0;
        }
        return;
    }
    // Reposition between shots, even when the player is outside melee range.
    if (distance > 0.001f && fabsf(distance - FLESH_SLIME_PREFERRED_DISTANCE) > 5) {
        float sign = distance > FLESH_SLIME_PREFERRED_DISTANCE ? 1.0f : -1.0f;
        float step = fminf(f->speed * dt, fabsf(distance - FLESH_SLIME_PREFERRED_DISTANCE));
        f->x = fmaxf(0, fminf(GAME_WIDTH - ACTOR_SIZE, f->x + dx / distance * step * sign));
        f->y = fmaxf(0, fminf(GAME_HEIGHT - ACTOR_SIZE, f->y + dy / distance * step * sign));
    }
    f->timer += dt;
    f->frame = (int)(f->timer / 0.1f) % FLESH_SLIME_RANGE_ENEMY_WALK_COUNT;
    if (f->timer >= FLEASH_SLIME_ATTACK_COOLDOWN) {
        f->state = FLEASH_SLIME_ATTACK; f->timer = 0; f->frame = 0; f->hasHit = false;
    }
}

static inline const uint16_t *FleshSlime_GetSprite(const FleshSlime *f)
{
    if (f->state == FLEASH_SLIME_DEAD) return flesh_slime_range_enemy_dead[f->frame];
    if (f->state == FLEASH_SLIME_ATTACK) return flesh_slime_range_enemy_attack[f->frame];
    return flesh_slime_range_enemy_walk[f->frame];
}
#endif // FLEASH_SLIME_H
