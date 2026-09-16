#ifndef ENEMY_COMBAT_H
#define ENEMY_COMBAT_H
#include "src/combat/projectile.h"
#include "src/enemies/flesh_slime.h"
#include "src/enemies/eye_parasite.h"

// Run before melee consumes attackHasHit, on the same damage beat as the sword.
static inline void SlimeShots_CheckPlayerAttack(SlimeShot *shots, const Player *p)
{
    if (p->hp <= 0 || !p->isAttacking || p->attackHasHit ||
        p->currentFrame < PLAYER_ATTACK_HIT_FRAME) return;
    SDL_FRect attack = Player_AttackBox(p);
    for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
        SlimeShot *s = &shots[i];
        SDL_FRect body = {s->x - 2, s->y - 2, 4, 4};
        if (!s->active || s->reflected || !SDL_HasRectIntersectionFloat(&attack, &body)) continue;
        s->vx = -s->vx;
        s->vy = -s->vy;
        s->damage = p->arrowDamage;
        s->reflected = true;
        s->hitFlashTimer = 0.12f;
        s->lifetime = 5.0f;
    }
}

static inline void SlimeShots_UpdateCombat(SlimeShot *shots, Player *p,
                                          Ghoul *g, FleshSlime *f, EyeParasite *e, float dt)
{
    for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
        SlimeShot *s = &shots[i];
        if (!s->active || !s->reflected || p->hp <= 0) continue;
        float step = fminf(dt, s->lifetime);
        SDL_FRect bodies[] = {{g->x - 2, g->y - 2, ACTOR_SIZE + 4, ACTOR_SIZE + 4},
            {f->x - 2, f->y - 2, ACTOR_SIZE + 4, ACTOR_SIZE + 4},
            {e->x - 2, e->y - 2, ACTOR_SIZE + 4, ACTOR_SIZE + 4}};
        int hp[] = {g->hp, f->hp, e->hp};
        int target = -1;
        float nearest = INFINITY;
        for (int j = 0; j < 3; ++j) {
            float x1 = s->x, y1 = s->y, x2 = s->x + s->vx * step, y2 = s->y + s->vy * step;
            if (hp[j] <= 0 || !SDL_GetRectAndLineIntersectionFloat(&bodies[j], &x1, &y1, &x2, &y2)) continue;
            float d = (x1 - s->x) * (x1 - s->x) + (y1 - s->y) * (y1 - s->y);
            if (d < nearest) { nearest = d; target = j; }
        }
        if (target < 0) continue;
        float speed = sqrtf(s->vx * s->vx + s->vy * s->vy);
        float px = speed > 0 ? s->vx / speed * (10.0f / 3.0f) : 0;
        float py = speed > 0 ? s->vy / speed * (10.0f / 3.0f) : 0;
        if (target == 0) Ghoul_TakeDamage(g, s->damage, px, py);
        if (target == 1) FleshSlime_TakeDamage(f, s->damage, px, py);
        if (target == 2) EyeParasite_TakeDamage(e, s->damage, px, py);
        s->active = false;
    }
    SlimeShots_Update(shots, p, dt);
}

static inline void Enemies_CheckPlayerAttack(Ghoul *g, FleshSlime *f, EyeParasite *e, Player *p)
{
    bool hit = p->hp > 0 && p->isAttacking &&
        p->currentFrame >= PLAYER_ATTACK_HIT_FRAME && !p->attackHasHit;
    Ghoul_CheckPlayerAttack(g, p);
    if (!hit) return;
    p->attackHasHit = true;
    SDL_FRect attack = Player_AttackBox(p);
    SDL_FRect slime = {f->x, f->y, ACTOR_SIZE, ACTOR_SIZE};
    SDL_FRect eye = {e->x, e->y, ACTOR_SIZE, ACTOR_SIZE};
    float px = 6.0f * ((p->direction == PLAYER_RIGHT) - (p->direction == PLAYER_LEFT));
    float py = 6.0f * ((p->direction == PLAYER_DOWN) - (p->direction == PLAYER_UP));
    if (SDL_HasRectIntersectionFloat(&attack, &slime)) FleshSlime_TakeDamage(f, p->attackDamage, px, py);
    if (SDL_HasRectIntersectionFloat(&attack, &eye)) EyeParasite_TakeDamage(e, p->attackDamage, px, py);
}

// Check additional targets before the original arrow update, without moving arrows twice.
static inline void Enemies_UpdateArrows(Projectile *shots, Ghoul *g, FleshSlime *f, EyeParasite *e, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        Projectile *b = &shots[i];
        if (!b->active) continue;
        float speed = sqrtf(b->vx * b->vx + b->vy * b->vy);
        float ux = speed > 0 ? b->vx / speed : 0, uy = speed > 0 ? b->vy / speed : 0;
        float startX = b->x + ux * 5.5f, startY = b->y + uy * 5.5f;
        float step = fminf(dt, b->lifetime);
        SDL_FRect bodies[] = {{g->x, g->y, ACTOR_SIZE, ACTOR_SIZE},
            {f->x, f->y, ACTOR_SIZE, ACTOR_SIZE}, {e->x, e->y, ACTOR_SIZE, ACTOR_SIZE}};
        int hp[] = {g->hp, f->hp, e->hp};
        int target = -1;
        float nearest = INFINITY;
        for (int j = 0; j < 3; ++j) {
            float x1 = startX, y1 = startY;
            float x2 = startX + b->vx * step, y2 = startY + b->vy * step;
            if (hp[j] <= 0 || !SDL_GetRectAndLineIntersectionFloat(&bodies[j], &x1, &y1, &x2, &y2)) continue;
            float distance = (x1 - startX) * (x1 - startX) + (y1 - startY) * (y1 - startY);
            if (distance < nearest) { nearest = distance; target = j; }
        }
        if (target == 1) FleshSlime_TakeDamage(f, b->damage, ux * (10.0f / 3.0f), uy * (10.0f / 3.0f));
        if (target == 2) EyeParasite_TakeDamage(e, b->damage, ux * (10.0f / 3.0f), uy * (10.0f / 3.0f));
        if (target > 0) b->active = false;
    }
    Projectiles_Update(shots, g, dt);
}
#endif
