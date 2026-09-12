#ifndef GHOUL_H
#define GHOUL_H
#define GHOUL_MAX_HP 100
#define GHOUL_ATTACK_HIT_FRAME 4 // Final slash pose (zero-based)
#define GHOUL_ATTACK_WINDUP 0.4f // Extra pause before the slash animation
#include <math.h>
#include <stdbool.h>
#include "player.h"
#include "bloodless_ghoul_sprites.h"
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
        .speed = 65.0f,
        .direction = PLAYER_LEFT,
        .state = GHOUL_WALK,
        .attackDamage = 10,
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
        return (SDL_FRect){g->x, g->y - 32, 48, 32};

    case PLAYER_DOWN:
        return (SDL_FRect){g->x, g->y + 48, 48, 32};

    case PLAYER_LEFT:
        return (SDL_FRect){g->x - 32, g->y, 32, 48};

    default:
        return (SDL_FRect){g->x + 48, g->y, 32, 48};
    }
}

static inline bool Ghoul_Overlaps(SDL_FRect a, SDL_FRect b)
{
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}
static inline SDL_FRect Player_AttackBox(const Player *p)
{
    switch (p->direction)
    {
    case PLAYER_UP:
        return (SDL_FRect){p->x, p->y - 32, 48, 32};

    case PLAYER_DOWN:
        return (SDL_FRect){p->x, p->y + 48, 48, 32};

    case PLAYER_LEFT:
        return (SDL_FRect){p->x - 32, p->y, 32, 48};

    default:
        return (SDL_FRect){p->x + 48, p->y, 32, 48};
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

    SDL_FRect ghoulBody = {g->x, g->y, 48, 48};

    if (!Ghoul_Overlaps(Player_AttackBox(p), ghoulBody))
        return;

    g->hp -= p->attackDamage;
    g->hitFlashTimer = 0.08f;
    float knockback = 18.0f;

    switch (p->direction)
    {
    case PLAYER_UP:
        g->y -= knockback;
        break;

    case PLAYER_DOWN:
        g->y += knockback;
        break;

    case PLAYER_LEFT:
        g->x -= knockback;
        break;

    case PLAYER_RIGHT:
        g->x += knockback;
        break;
    }

    // ขัดจังหวะการโจมตี และให้ศัตรูชะงัก
    g->state = GHOUL_REST;
    g->timer = 0.0f;
    g->frame = 0;
    if (g->hp <= 0)
    {
        g->hp = 0;
        g->state = GHOUL_DEAD;
        g->frame = 0;
        g->timer = 0.0f;
    }
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

    // ผู้เล่นและ Ghoul แสดงผลขนาด 48×48
    SDL_FRect playerBox = {player->x, player->y, 48, 48};
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

            SDL_FRect bodyBox = {g->x, g->y, 48, 48};
            if (Ghoul_Overlaps(Ghoul_AttackBox(g), playerBox) ||
                Ghoul_Overlaps(bodyBox, playerBox))
            {
                player->hp -= g->attackDamage;

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

    SDL_FRect bodyBox = {g->x, g->y, 48, 48};

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
