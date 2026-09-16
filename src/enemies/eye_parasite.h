#ifndef EYE_PARASITE_H
#define EYE_PARASITE_H
#define EYE_PARASITE_MAX_HP 50
#include <math.h>
#include <stdbool.h>
#include "src/player/player.h"
#include "assets/sprites/enemies/eye_parasite_sprites.h"
#include <SDL3/SDL.h>
#include "src/enemies/enemy_common.h"
#define EYE_PARASITE_BLAST_RADIUS 32.0f
#define EYE_PARASITE_BLAST_SIZE (EYE_PARASITE_BLAST_RADIUS * 2.0f)
#define EYE_PARASITE_TRIGGER_DISTANCE 25.0f
#define EYE_PARASITE_DECISION_DELAY 0.6f // Continuous time in trigger range before committing.

typedef enum { EYE_PARASITE_WALK, EYE_PARASITE_ATTACK, EYE_PARASITE_DEAD } EyeParasiteState;
typedef struct {
    float x, y, speed, timer, hitFlashTimer, explosionTimer;
    float proximityTimer;
    int hp, maxHP, attackDamage, frame;
    bool deathFinished;
    PlayerDirection direction;
    EyeParasiteState state;
} EyeParasite;

static inline void EyeParasite_Init(EyeParasite *e, float x, float y)
{
    *e = (EyeParasite){.x = x, .y = y, .speed = 38, .hp = EYE_PARASITE_MAX_HP,
        .maxHP = EYE_PARASITE_MAX_HP, .attackDamage = 30, .direction = PLAYER_LEFT};
}

static inline SDL_FRect EyeParasite_BlastBox(const EyeParasite *e)
{
    return (SDL_FRect){e->x + ACTOR_HALF_SIZE - EYE_PARASITE_BLAST_SIZE / 2,
        e->y + ACTOR_HALF_SIZE - EYE_PARASITE_BLAST_SIZE / 2,
        EYE_PARASITE_BLAST_SIZE, EYE_PARASITE_BLAST_SIZE};
}

static inline void EyeParasite_TakeDamage(EyeParasite *e, int damage, float px, float py)
{
    if (e->hp <= 0 || damage <= 0) return;
    e->hp = e->hp > damage ? e->hp - damage : 0;
    e->hitFlashTimer = 0.08f;
    e->x += px; e->y += py;
    if (!e->hp) {
        e->state = EYE_PARASITE_DEAD; e->timer = 0; e->frame = 0;
    }
}

static inline bool EyeParasite_BlastHitsPlayer(const EyeParasite *e, const Player *p)
{
    float cx = e->x + ACTOR_HALF_SIZE, cy = e->y + ACTOR_HALF_SIZE;
    float nearestX = fmaxf(p->x, fminf(cx, p->x + ACTOR_SIZE));
    float nearestY = fmaxf(p->y, fminf(cy, p->y + ACTOR_SIZE));
    float dx = cx - nearestX, dy = cy - nearestY;
    return dx * dx + dy * dy <= EYE_PARASITE_BLAST_RADIUS * EYE_PARASITE_BLAST_RADIUS;
}

static inline void EyeParasite_Update(EyeParasite *e, Player *p, float dt)
{
    e->hitFlashTimer = fmaxf(0, e->hitFlashTimer - dt);
    e->explosionTimer = fmaxf(0, e->explosionTimer - dt);
    if (e->state == EYE_PARASITE_DEAD) {
        if (!e->deathFinished)
            e->deathFinished = Enemy_Animate(&e->timer, &e->frame, dt,
                eye_parasite_bomb_enemy_dead_duration_ms, EYE_PARASITE_BOMB_ENEMY_DEAD_COUNT, false);
        return;
    }
    if (p->hp <= 0) return;
    if (e->state == EYE_PARASITE_ATTACK) {
        if (Enemy_Animate(&e->timer, &e->frame, dt, eye_parasite_bomb_enemy_attack_duration_ms,
                          EYE_PARASITE_BOMB_ENEMY_ATTACK_COUNT, false)) {
            if (EyeParasite_BlastHitsPlayer(e, p))
                Enemy_HurtPlayer(p, e->attackDamage, e->x + ACTOR_HALF_SIZE, e->y + ACTOR_HALF_SIZE);
            e->hp = 0; e->state = EYE_PARASITE_DEAD; e->frame = 0; e->timer = 0;
            e->explosionTimer = 0.25f;
        }
        return;
    }
    float dx = p->x - e->x, dy = p->y - e->y;
    float distance = sqrtf(dx * dx + dy * dy);
    e->direction = Enemy_Face(dx, dy);
    if (distance <= EYE_PARASITE_TRIGGER_DISTANCE) {
        e->proximityTimer += dt;
        if (e->proximityTimer >= EYE_PARASITE_DECISION_DELAY) {
            // Once committed, finish the normal attack animation even if the player escapes.
            e->state = EYE_PARASITE_ATTACK;
            e->timer = 0;
            e->frame = 0;
        }
        return;
    }
    e->proximityTimer = 0; // Leaving the range cancels the pending decision.
    float step = fminf(e->speed * dt, distance);
    e->x += dx / distance * step; e->y += dy / distance * step;
    Enemy_Animate(&e->timer, &e->frame, dt, eye_parasite_bomb_enemy_walk_duration_ms,
                  EYE_PARASITE_BOMB_ENEMY_WALK_COUNT, true);
}

static inline const uint16_t *EyeParasite_GetSprite(const EyeParasite *e)
{
    if (e->state == EYE_PARASITE_DEAD) return eye_parasite_bomb_enemy_dead[e->frame];
    if (e->state == EYE_PARASITE_ATTACK) return eye_parasite_bomb_enemy_attack[e->frame];
    return eye_parasite_bomb_enemy_walk[e->frame];
}
#endif // EYE_PARASITE_H
