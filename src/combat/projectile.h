#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <math.h>
#include <string.h>
#include "src/enemies/bloodless_ghoul.h"

#define MAX_PROJECTILES 32

typedef struct {
    float x, y;
    float vx, vy;
    float lifetime;
    int damage;
    bool active;
    ArrowType type;
} Projectile;

static inline void Projectiles_Recharge(Player *p, float dt)
{
    if (p->hp <= 0) return;
    if (p->bowCharges >= PLAYER_BOW_MAX_CHARGES) { p->shootTimer = 0; return; }
    if (p->shootCooldown <= 0) { p->bowCharges = PLAYER_BOW_MAX_CHARGES; p->shootTimer = 0; return; }
    p->shootTimer -= dt;
    while (p->shootTimer <= 0 && p->bowCharges < PLAYER_BOW_MAX_CHARGES) {
        ++p->bowCharges;
        if (p->bowCharges < PLAYER_BOW_MAX_CHARGES) p->shootTimer += p->shootCooldown;
        else p->shootTimer = 0;
    }
}

static inline void Projectiles_Reset(Projectile *shots)
{
    memset(shots, 0, sizeof(Projectile) * MAX_PROJECTILES);
}

// Start the bow animation; the arrow is created only on the middle frame.
static inline bool Projectiles_Shoot(Projectile *shots, Player *p)
{
    if (p->hp <= 0 || p->isAttacking || p->isShooting || p->bowCharges < Arrow_ChargeCost(p->arrowType))
        return false;
    float dx = p->aimX, dy = p->aimY;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f)
        return false;
    bool available = false;
    for (int i = 0; i < MAX_PROJECTILES; ++i)
        if (!shots[i].active) available = true;
    if (!available)
        return false;
    p->aimX = dx / length;
    p->aimY = dy / length;
    p->direction = fabsf(dx) >= fabsf(dy)
        ? (dx < 0 ? PLAYER_LEFT : PLAYER_RIGHT)
        : (dy < 0 ? PLAYER_UP : PLAYER_DOWN);
    p->facingLeft = p->direction == PLAYER_LEFT;
    p->isShooting = true;
    p->isSprinting = false;
    p->arrowReleased = false;
    p->currentFrame = 0;
    p->animTimer = 0.0f;
    if (p->bowCharges == PLAYER_BOW_MAX_CHARGES) p->shootTimer = p->shootCooldown;
    p->shootingArrowType = p->arrowType;
    p->bowCharges -= Arrow_ChargeCost(p->arrowType);
    p->isMoving = false;
    p->isMovingLeft = p->isMovingRight = false;
    p->isMovingUp = p->isMovingDown = false;
    return true;
}

static inline void Projectiles_UpdateShooting(Projectile *shots, Player *p, float dt)
{
    if (!p->isShooting || p->hp <= 0)
        return;
    p->animTimer += dt;
    while (p->animTimer >= PLAYER_SHOOT_FRAME_TIME) {
        p->animTimer -= PLAYER_SHOOT_FRAME_TIME;
        p->currentFrame++;
        if (!p->arrowReleased && p->currentFrame >= PLAYER_SHOOT_RELEASE_FRAME) {
            p->arrowReleased = true;
            for (int i = 0; i < MAX_PROJECTILES; ++i) {
                if (shots[i].active) continue;
                shots[i] = (Projectile){
                    .x = p->x + ACTOR_HALF_SIZE, .y = p->y + ACTOR_HALF_SIZE,
                    .vx = p->aimX * p->arrowSpeed,
                    .vy = p->aimY * p->arrowSpeed,
                    .lifetime = p->arrowLifetime,
                    .damage = p->arrowDamage, .active = true, .type = p->shootingArrowType
                };
                break;
            }
        }
        if (p->currentFrame >= PLAYER_SHOOT_FRAME_COUNT) {
            p->isShooting = false;
            p->currentFrame = 0;
            p->animTimer = 0.0f;
            break;
        }
    }
}

static inline void Projectiles_Update(Projectile *shots, Ghoul *g, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        Projectile *b = &shots[i];
        if (!b->active)
            continue;
        float step = fminf(dt, b->lifetime);
        float nextX = b->x + b->vx * step;
        float nextY = b->y + b->vy * step;
        // Sweep the arrow's tip so fast arrows cannot skip through the target.
        float speed = sqrtf(b->vx * b->vx + b->vy * b->vy);
        float ux = speed > 0 ? b->vx / speed : 0;
        float uy = speed > 0 ? b->vy / speed : 0;
        float x1 = b->x + ux * 5.5f, y1 = b->y + uy * 5.5f;
        float x2 = nextX + ux * 5.5f, y2 = nextY + uy * 5.5f;
        SDL_FRect body = {g->x, g->y, ACTOR_SIZE, ACTOR_SIZE};
        if (g->hp > 0 && SDL_GetRectAndLineIntersectionFloat(&body, &x1, &y1, &x2, &y2)) {
            Ghoul_TakeDamage(g, b->damage, ux * (10.0f / 3.0f), uy * (10.0f / 3.0f));
            b->active = false;
            continue;
        }
        b->x = nextX;
        b->y = nextY;
        b->lifetime -= dt;
        if (b->lifetime <= 0.0f)
            b->active = false;
    }
}

#endif
