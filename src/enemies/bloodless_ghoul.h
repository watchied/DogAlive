#ifndef GHOUL_H
#define GHOUL_H
#define GHOUL_MAX_HP 100
#define GHOUL_ATTACK_HIT_FRAME MELEE_HIT_FRAME // Final actor pose, synchronized with effect frame 3.
#define GHOUL_ATTACK_WINDUP 0.35f // Extra pause before the slash animation
#include <math.h>
#include <stdbool.h>
#include "src/player/player.h"
#include "assets/sprites/enemies/bloodless_ghoul_sprites.h"
#include <SDL3/SDL.h>
typedef enum
{
    GHOUL_WALK,
    GHOUL_ATTACK,
    GHOUL_REST,
    GHOUL_DEAD
} GhoulState;

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
    GhoulState state;
    bool hasHit;
} Ghoul;

static inline void Ghoul_Init(Ghoul *g, float x, float y)
{
    *g = (Ghoul){
        .x = x,
        .y = y,
        .speed = 70.0f / 3.0f,
        .direction = PLAYER_LEFT,
        .state = GHOUL_WALK,
        .attackDamage = 20,
        .hp = GHOUL_MAX_HP,
        .maxHP = GHOUL_MAX_HP,

    };
}

// กรอบตีอยู่ด้านหน้าตัว Ghoul
static inline SDL_FRect Ghoul_AttackBox(const Ghoul *g)
{
    switch (g->direction)
    {
    case PLAYER_UP:
        return (SDL_FRect){g->x, g->y - MELEE_REACH, ACTOR_SIZE, MELEE_REACH};

    case PLAYER_DOWN:
        return (SDL_FRect){g->x, g->y + ACTOR_SIZE, ACTOR_SIZE, MELEE_REACH};

    case PLAYER_LEFT:
        return (SDL_FRect){g->x - MELEE_REACH, g->y, MELEE_REACH, ACTOR_SIZE};

    default:
        return (SDL_FRect){g->x + ACTOR_SIZE, g->y, MELEE_REACH, ACTOR_SIZE};
    }
}

static inline bool Ghoul_Overlaps(SDL_FRect a, SDL_FRect b)
{
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}
static inline void Ghoul_TakeDamage(Ghoul *g, int damage, float pushX, float pushY)
{
    if (g->hp <= 0 || damage <= 0)
        return;
    g->hp -= damage;
    g->hitFlashTimer = 0.08f;
    g->x += pushX;
    g->y += pushY;
    g->state = GHOUL_REST;
    g->timer = 0.0f;
    g->frame = 0;
    if (g->hp <= 0) {
        g->hp = 0;
        g->state = GHOUL_DEAD;
    }
}

static inline void Ghoul_CheckPlayerAttack(Ghoul *g, Player *p)
{
    if (g->hp <= 0 || p->hp <= 0)
        return;

    // ตรวจครั้งเดียวตอนถึงเฟรมตีที่ 2 (นับจาก 0)
    if (!p->isAttacking ||
        p->currentFrame < PLAYER_ATTACK_HIT_FRAME ||
        p->attackHasHit)
        return;

    // แม้ตีพลาด ก็ถือว่าใช้จังหวะทำดาเมจของท่านี้แล้ว
    p->attackHasHit = true;

    SDL_FRect ghoulBody = {g->x, g->y, ACTOR_SIZE, ACTOR_SIZE};

    if (!Ghoul_Overlaps(Player_AttackBox(p), ghoulBody))
        return;

    float pushX = 0.0f, pushY = 0.0f;
    switch (p->direction) {
        case PLAYER_UP: pushY = -6.0f; break;
        case PLAYER_DOWN: pushY = 6.0f; break;
        case PLAYER_LEFT: pushX = -6.0f; break;
        case PLAYER_RIGHT: pushX = 6.0f; break;
    }
    Ghoul_TakeDamage(g, p->attackDamage, pushX, pushY);
}

static inline void Ghoul_Update(
    Ghoul *g,
    Player *player,
    float dt)
{
    if (g->hitFlashTimer > 0.0f)
    {
        g->hitFlashTimer -= dt;
        if (g->hitFlashTimer < 0.0f)
            g->hitFlashTimer = 0.0f;
    }

    // Play every death frame once, including the final frame's duration.
    if (g->state == GHOUL_DEAD)
    {
        if (g->deathFinished)
            return;

        g->timer += dt;
        while (g->timer >=
               bloodless_ghoul_melee_enemy_death_duration_ms[g->frame] / 1000.0f)
        {
            g->timer -=
                bloodless_ghoul_melee_enemy_death_duration_ms[g->frame] / 1000.0f;
            if (g->frame + 1 >= BLOODLESS_GHOUL_MELEE_ENEMY_DEATH_COUNT)
            {
                g->deathFinished = true;
                g->timer = 0.0f;
                break;
            }
            g->frame++;
        }
        return;
    }

    // ผู้เล่นและ Ghoul แสดงผลขนาด 16×16
    SDL_FRect playerBox = {player->x, player->y, ACTOR_SIZE, ACTOR_SIZE};
    if (g->hp <= 0)
        return;
    if (player->hp <= 0)
        return;

    if (g->state == GHOUL_ATTACK)
    {
        g->timer += dt;

        // Hold the first pose so the player has time to react.
        float attackTime = g->timer - GHOUL_ATTACK_WINDUP;
        if (attackTime < 0.0f) {
            g->frame = 0;
            return;
        }

        // Use the sprite timings for both the visible pose and the hit.
        float frameTime = attackTime;
        int attackFrame = 0;
        while (attackFrame < BLOODLESS_GHOUL_MELEE_ENEMY_SLASH_COUNT &&
               frameTime >= bloodless_ghoul_melee_enemy_slash_duration_ms[attackFrame] / 1000.0f) {
            frameTime -= bloodless_ghoul_melee_enemy_slash_duration_ms[attackFrame] / 1000.0f;
            attackFrame++;
        }
        g->frame = attackFrame < BLOODLESS_GHOUL_MELEE_ENEMY_SLASH_COUNT
            ? attackFrame : BLOODLESS_GHOUL_MELEE_ENEMY_SLASH_COUNT - 1;

        if (!g->hasHit && g->frame >= GHOUL_ATTACK_HIT_FRAME)
        {
            g->hasHit = true;

            SDL_FRect bodyBox = {g->x, g->y, ACTOR_SIZE, ACTOR_SIZE};
            if (player->invincibilityTimer <= 0 &&
                (Ghoul_Overlaps(Ghoul_AttackBox(g), playerBox) ||
                 Ghoul_Overlaps(bodyBox, playerBox)))
            {
                player->invincibilityTimer = PLAYER_INVINCIBILITY_TIME;
                player->hp -= g->attackDamage;
                player->hitFlashTimer = PLAYER_HIT_FLASH_TIME;
                player->collisionGraceTimer = PLAYER_COLLISION_GRACE_TIME;
                float pushX = player->x - g->x;
                float pushY = player->y - g->y;
                float pushLength = sqrtf(pushX * pushX + pushY * pushY);
                if (pushLength > 0.001f) {
                    pushX /= pushLength;
                    pushY /= pushLength;
                } else {
                    pushX = (g->direction == PLAYER_RIGHT) - (g->direction == PLAYER_LEFT);
                    pushY = (g->direction == PLAYER_DOWN) - (g->direction == PLAYER_UP);
                }
                player->x += pushX * PLAYER_HIT_KNOCKBACK;
                player->y += pushY * PLAYER_HIT_KNOCKBACK;

                if (player->hp < 0)
                    player->hp = 0;
            }
        }

        if (attackFrame >= BLOODLESS_GHOUL_MELEE_ENEMY_SLASH_COUNT)
        {
            g->state = GHOUL_REST;
            g->timer = 0;
            g->frame = 0;
        }

        return;
    }

    if (g->state == GHOUL_REST)
    {
        g->timer += dt;

        if (g->timer >= 0.7f)
        {
            g->state = GHOUL_WALK;
            g->timer = 0;
            g->frame = 0;
        }

        return;
    }

    float dx = player->x - g->x;
    float dy = player->y - g->y;
    float distance = sqrtf(dx * dx + dy * dy);

    // เลือกทิศตามแกนที่ห่างมากกว่า
    if (fabsf(dx) > fabsf(dy))
    {
        g->direction = dx < 0 ? PLAYER_LEFT : PLAYER_RIGHT;
    }
    else if (distance > 0)
    {
        g->direction = dy < 0 ? PLAYER_UP : PLAYER_DOWN;
    }

    SDL_FRect bodyBox = {g->x, g->y, ACTOR_SIZE, ACTOR_SIZE};

    if (Ghoul_Overlaps(Ghoul_AttackBox(g), playerBox) ||
        Ghoul_Overlaps(bodyBox, playerBox))
    {
        g->state = GHOUL_ATTACK;
        g->timer = 0;
        g->frame = 0;
        g->hasHit = false;
        return;
    }

    if (distance > 0)
    {
        float step = g->speed * dt;

        if (step > distance)
            step = distance;

        g->x += dx / distance * step;
        g->y += dy / distance * step;
    }

    g->timer += dt;

    while (g->timer >= 0.1f)
    {
        g->timer -= 0.1f;
        g->frame = (g->frame + 1) %
                   BLOODLESS_GHOUL_MELEE_ENEMY_WALK_COUNT;
    }
}

// Separate living bodies along the smallest overlap. Damage/grace is handled first.
static inline void Ghoul_ResolvePlayerCollision(const Ghoul *g, Player *p)
{
    if (g->hp <= 0 || p->hp <= 0 || p->collisionGraceTimer > 0.0f)
        return;
    float dx = p->x - g->x;
    float dy = p->y - g->y;
    float overlapX = ACTOR_SIZE - fabsf(dx);
    float overlapY = ACTOR_SIZE - fabsf(dy);
    if (overlapX <= 0.0f || overlapY <= 0.0f)
        return;
    if (overlapX < overlapY)
        p->x += dx < 0.0f ? -overlapX : overlapX;
    else
        p->y += dy < 0.0f ? -overlapY : overlapY;
}

static inline const uint16_t *Ghoul_GetSprite(const Ghoul *g)
{
    if (g->state == GHOUL_DEAD)
        return bloodless_ghoul_melee_enemy_death[g->frame];

    if (g->state == GHOUL_ATTACK)
        return bloodless_ghoul_melee_enemy_slash[g->frame];

    if (g->state == GHOUL_REST)
        return bloodless_ghoul_melee_enemy_walk[0];

    return bloodless_ghoul_melee_enemy_walk[g->frame];
}

#endif
