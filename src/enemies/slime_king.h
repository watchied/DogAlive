#ifndef SLIME_KING_H
#define SLIME_KING_H
#include "src/enemies/slime_king_config.h"
#include "src/enemies/enemy_common.h"
#include "assets/sprites/enemies/slime_king(boss)_sprites.h"
#include "assets/sprites/npc/slime_king(npc)_sprites.h"
#include <string.h>

typedef enum {
    KING_INTRO, KING_IDLE, KING_BUBBLE, KING_DASH_READY, KING_DASH,
    KING_BURROW, KING_SLAM_MARK, KING_SLAM_FALL, KING_SLAM_LAND,
    KING_RADIAL, KING_SPIRAL, KING_BEAM_CHARGE, KING_BEAM_FIRE,
    KING_STUN, KING_PHASE_CHANGE, KING_DEATH, KING_NPC
} KingState;
typedef enum { KING_SHOT_BUBBLE, KING_SHOT_LASER, KING_SHOT_BEAM } KingShotType;
typedef struct {
    float x, y, vx, vy, radius, lifetime;
    int damage, bounces, bounceLimit;
    KingShotType type;
    bool active;
} KingShot;
typedef struct {
    bool active;
    float x, y, timer, animTimer, flashTimer, vx, vy, aimX, aimY;
    float slamX, slamY, slamFlash, spiralAngle;
    int hp, maxHP, phase, frame, slamsLeft, shotsFired, interruptDamage, lastSkill;
    bool dashHit;
    uint32_t rng;
    KingState state;
    KingShot shots[KING_SHOT_CAPACITY];
} SlimeKing;

static inline uint32_t King_Random(SlimeKing *k)
{
    k->rng = k->rng * 1664525u + 1013904223u;
    return k->rng;
}
static inline void King_Enter(SlimeKing *k, KingState state)
{
    k->state = state; k->timer = 0; k->animTimer = 0; k->frame = 0;
}
static inline SDL_FRect King_Body(const SlimeKing *k)
{
    return (SDL_FRect){k->x + KING_BODY_OFFSET_X, k->y + KING_BODY_OFFSET_Y,
        KING_BODY_WIDTH, KING_BODY_HEIGHT};
}
static inline bool King_Targetable(const SlimeKing *k)
{
    return k->active && k->hp > 0 && k->state != KING_INTRO && k->state != KING_PHASE_CHANGE;
}
static inline void King_Clamp(SlimeKing *k)
{
    k->x = fmaxf(KING_WALL - KING_BODY_OFFSET_X,
        fminf(GAME_WIDTH - KING_WALL - KING_BODY_OFFSET_X - KING_BODY_WIDTH, k->x));
    k->y = fmaxf(KING_WALL - KING_BODY_OFFSET_Y,
        fminf(GAME_HEIGHT - KING_WALL - KING_BODY_OFFSET_Y - KING_BODY_HEIGHT, k->y));
}
static inline void King_Init(SlimeKing *k, float x, float y)
{
    *k = (SlimeKing){.active = true, .x = x, .y = y, .hp = KING_MAX_HP,
        .maxHP = KING_MAX_HP, .phase = 1, .rng = 0xB055u, .lastSkill = -1, .aimX = -1};
    King_Clamp(k);
    King_Enter(k, KING_INTRO);
}
static inline void King_TakeDamage(SlimeKing *k, int damage, bool direct)
{
    if (!King_Targetable(k) || damage <= 0) return;
    k->hp = k->hp > damage ? k->hp - damage : 0;
    k->flashTimer = 0.08f;
    if (k->hp == 0) {
        memset(k->shots, 0, sizeof(k->shots)); k->slamFlash = 0;
        King_Enter(k, KING_DEATH); return;
    }
    if (k->phase == 1 && k->hp <= k->maxHP * KING_PHASE_TWO_RATIO) {
        k->phase = 2; k->slamFlash = 0;
        memset(k->shots, 0, sizeof(k->shots));
        King_Enter(k, KING_PHASE_CHANGE); return;
    }
    if (direct && k->state == KING_BEAM_CHARGE) {
        k->interruptDamage += damage;
        if (k->interruptDamage >= KING_BEAM_INTERRUPT_DAMAGE) King_Enter(k, KING_STUN);
    }
}
static inline void King_Aim(SlimeKing *k, const Player *p)
{
    SDL_FRect b = King_Body(k);
    float dx = p->x + ACTOR_HALF_SIZE - (b.x + b.w / 2);
    float dy = p->y + ACTOR_HALF_SIZE - (b.y + b.h / 2);
    float len = sqrtf(dx * dx + dy * dy);
    k->aimX = len > 0.001f ? dx / len : 1;
    k->aimY = len > 0.001f ? dy / len : 0;
}
static inline void King_SpawnShot(SlimeKing *k, KingShotType type, float ux, float uy, int bounces)
{
    SDL_FRect body = King_Body(k);
    for (int i = 0; i < KING_SHOT_CAPACITY; ++i) {
        if (k->shots[i].active) continue;
        float speed = type == KING_SHOT_BUBBLE ? KING_BUBBLE_SPEED : type == KING_SHOT_BEAM ? KING_BEAM_SPEED : KING_LASER_SPEED;
        k->shots[i] = (KingShot){.x = body.x + body.w / 2, .y = body.y + body.h / 2,
            .vx = ux * speed, .vy = uy * speed, .bounceLimit = bounces,
            .radius = type == KING_SHOT_BUBBLE ? KING_BUBBLE_RADIUS : type == KING_SHOT_BEAM ? KING_BEAM_RADIUS : KING_LASER_RADIUS,
            .damage = type == KING_SHOT_BUBBLE ? KING_BUBBLE_DAMAGE : type == KING_SHOT_BEAM ? KING_BEAM_DAMAGE : KING_LASER_DAMAGE,
            .lifetime = type == KING_SHOT_BEAM ? KING_BEAM_LIFETIME : KING_SHOT_LIFETIME,
            .type = type, .active = true};
        return;
    }
}
// Sweep each straight segment up to the next wall, then reflect the remaining time.
static inline void King_UpdateShot(KingShot *s, Player *p, float dt)
{
    if (!s->active) return;
    float remaining = fminf(dt, s->lifetime);
    s->lifetime -= dt;
    float minX = KING_WALL + s->radius, maxX = GAME_WIDTH - KING_WALL - s->radius;
    float minY = KING_WALL + s->radius, maxY = GAME_HEIGHT - KING_WALL - s->radius;
    s->x = fmaxf(minX, fminf(maxX, s->x)); s->y = fmaxf(minY, fminf(maxY, s->y));
    while (remaining > 0.000001f && s->active) {
        float tx = s->vx > 0 ? (maxX - s->x) / s->vx : s->vx < 0 ? (minX - s->x) / s->vx : INFINITY;
        float ty = s->vy > 0 ? (maxY - s->y) / s->vy : s->vy < 0 ? (minY - s->y) / s->vy : INFINITY;
        float wallTime = fminf(tx, ty);
        float step = fmaxf(0, fminf(remaining, wallTime));
        float nx = s->x + s->vx * step, ny = s->y + s->vy * step;
        float x1 = s->x, y1 = s->y, x2 = nx, y2 = ny;
        SDL_FRect box = {p->x - s->radius, p->y - s->radius, ACTOR_SIZE + s->radius * 2, ACTOR_SIZE + s->radius * 2};
        if (p->hp > 0 && SDL_GetRectAndLineIntersectionFloat(&box, &x1, &y1, &x2, &y2)) {
            Enemy_HurtPlayer(p, s->damage, s->x - s->vx * 0.01f, s->y - s->vy * 0.01f);
            s->active = false;
        }
        s->x = nx; s->y = ny; remaining -= step;
        if (wallTime <= step + 0.000001f && s->active) {
            ++s->bounces;
            if (s->bounces >= s->bounceLimit) { s->active = false; break; }
            if (tx <= wallTime + 0.000001f) s->vx = -s->vx;
            if (ty <= wallTime + 0.000001f) s->vy = -s->vy;
        }
    }
    if (s->lifetime <= 0) s->active = false;
}
static inline void King_StartSkill(SlimeKing *k, const Player *p, int skill)
{
    k->lastSkill = skill; k->shotsFired = 0; k->interruptDamage = 0; k->dashHit = false;
    King_Aim(k, p);
    if (skill == 0) King_Enter(k, KING_BUBBLE);
    else if (skill == 1) King_Enter(k, KING_DASH_READY);
    else if (k->phase == 1) {
        k->slamsLeft = KING_SLAM_MIN + King_Random(k) % (KING_SLAM_MAX - KING_SLAM_MIN + 1);
        King_Enter(k, KING_BURROW);
    } else if (skill == 2) King_Enter(k, KING_RADIAL);
    else if (skill == 3) { k->spiralAngle = atan2f(k->aimY, k->aimX); King_Enter(k, KING_SPIRAL); }
    else King_Enter(k, KING_BEAM_CHARGE);
}
static inline void King_SlamDamage(SlimeKing *k, Player *p)
{
    float dx = k->slamX - fmaxf(p->x, fminf(k->slamX, p->x + ACTOR_SIZE));
    float dy = k->slamY - fmaxf(p->y, fminf(k->slamY, p->y + ACTOR_SIZE));
    if (dx * dx + dy * dy <= KING_SLAM_RADIUS * KING_SLAM_RADIUS)
        Enemy_HurtPlayer(p, KING_SLAM_DAMAGE, k->slamX, k->slamY);
    k->slamFlash = 0.2f;
}
static inline void King_ResolvePlayerCollision(const SlimeKing *k, Player *p)
{
    if (!King_Targetable(k) || p->hp <= 0 || p->collisionGraceTimer > 0) return;
    // The slam remains damageable, but its underground/airborne body must not push the player.
    if (k->state == KING_BURROW || k->state == KING_SLAM_MARK || k->state == KING_SLAM_FALL) return;
    SDL_FRect b = King_Body(k);
    float dx = p->x + ACTOR_HALF_SIZE - (b.x + b.w / 2);
    float dy = p->y + ACTOR_HALF_SIZE - (b.y + b.h / 2);
    float ox = (ACTOR_SIZE + b.w) / 2 - fabsf(dx);
    float oy = (ACTOR_SIZE + b.h) / 2 - fabsf(dy);
    if (ox <= 0 || oy <= 0) return;
    if (ox < oy) p->x += dx < 0 ? -ox : ox;
    else p->y += dy < 0 ? -oy : oy;
}
static inline void King_Update(SlimeKing *k, Player *p, float dt)
{
    if (!k->active) return;
    k->flashTimer = fmaxf(0, k->flashTimer - dt);
    k->slamFlash = fmaxf(0, k->slamFlash - dt);
    if (k->state == KING_DEATH) {
        k->timer += dt;
        if (k->timer >= KING_DEATH_TIME) {
            King_Enter(k, KING_NPC);
        } else {
            k->frame = (int)(k->timer / KING_DEATH_TIME * SLIME_KING_BOSS_DEATH_COUNT);
            if (k->frame >= SLIME_KING_BOSS_DEATH_COUNT) k->frame = SLIME_KING_BOSS_DEATH_COUNT - 1;
        }
        return;
    }
    if (k->state == KING_NPC) {
        k->frame = 0;
        k->animTimer = 0;
        return;
    }
    if (p->hp <= 0) return;
    for (int i = 0; i < KING_SHOT_CAPACITY; ++i) King_UpdateShot(&k->shots[i], p, dt);
    k->timer += dt;
    switch (k->state) {
    case KING_INTRO: if (k->timer >= KING_INTRO_TIME) King_Enter(k, KING_IDLE); break;
    case KING_PHASE_CHANGE: if (k->timer >= KING_PHASE_TRANSITION_TIME) King_Enter(k, KING_IDLE); break;
    case KING_STUN: if (k->timer >= KING_BEAM_INTERRUPT_STUN) King_Enter(k, KING_IDLE); break;
    case KING_IDLE:
        King_Aim(k, p);
        k->x += k->aimX * KING_MOVE_SPEED * dt; k->y += k->aimY * KING_MOVE_SPEED * dt; King_Clamp(k);
        if (k->timer >= (k->phase == 1 ? KING_ATTACK_REST : KING_PHASE_TWO_REST)) {
            int count = k->phase == 1 ? 3 : 5;
            int skill = (int)(King_Random(k) % count);
            if (skill == k->lastSkill) skill = (skill + 1) % count;
            King_StartSkill(k, p, skill);
        }
        break;
    case KING_BUBBLE:
        King_Aim(k, p);
        if (k->timer >= KING_BUBBLE_WINDUP) {
            King_SpawnShot(k, KING_SHOT_BUBBLE, k->aimX, k->aimY, KING_BUBBLE_BOUNCES); King_Enter(k, KING_IDLE);
        }
        break;
    case KING_DASH_READY:
        if (k->timer >= KING_DASH_WINDUP) {
            k->vx = k->aimX * KING_DASH_SPEED; k->vy = k->aimY * KING_DASH_SPEED; King_Enter(k, KING_DASH);
        }
        break;
    case KING_DASH: {
        SDL_FRect b = King_Body(k);
        float x1 = b.x + b.w / 2, y1 = b.y + b.h / 2;
        k->x += k->vx * dt; k->y += k->vy * dt; King_Clamp(k);
        SDL_FRect next = King_Body(k);
        float x2 = next.x + next.w / 2, y2 = next.y + next.h / 2;
        SDL_FRect box = {p->x - b.w / 2, p->y - b.h / 2, ACTOR_SIZE + b.w, ACTOR_SIZE + b.h};
        if (!k->dashHit && SDL_GetRectAndLineIntersectionFloat(&box, &x1, &y1, &x2, &y2)) {
            Enemy_HurtPlayer(p, KING_DASH_DAMAGE, b.x, b.y); k->dashHit = true;
        }
        if (k->timer >= KING_DASH_TIME) King_Enter(k, KING_IDLE);
        break;
    }
    case KING_BURROW:
        if (k->timer >= KING_BURROW_TIME) {
            k->x = p->x + ACTOR_HALF_SIZE - KING_BODY_OFFSET_X - KING_BODY_WIDTH / 2;
            k->y = p->y + ACTOR_HALF_SIZE - KING_BODY_OFFSET_Y - KING_BODY_HEIGHT / 2;
            King_Clamp(k);
            SDL_FRect b = King_Body(k);
            k->slamX = b.x + b.w / 2; k->slamY = b.y + b.h / 2;
            King_Enter(k, KING_SLAM_MARK);
        }
        break;
    case KING_SLAM_MARK: if (k->timer >= KING_SLAM_WARNING) King_Enter(k, KING_SLAM_FALL); break;
    case KING_SLAM_FALL:
        if (k->timer >= KING_SLAM_FALL_TIME) { King_SlamDamage(k, p); King_Enter(k, KING_SLAM_LAND); }
        break;
    case KING_SLAM_LAND:
        if (k->timer >= KING_SLAM_RECOVERY) King_Enter(k, --k->slamsLeft > 0 ? KING_BURROW : KING_IDLE);
        break;
    case KING_RADIAL:
        if (k->timer >= KING_RADIAL_WINDUP) {
            for (int i = 0; i < KING_RADIAL_COUNT; ++i) {
                float angle = i * 6.28318530718f / KING_RADIAL_COUNT;
                King_SpawnShot(k, KING_SHOT_LASER, cosf(angle), sinf(angle), KING_LASER_BOUNCES);
            }
            King_Enter(k, KING_IDLE);
        }
        break;
    case KING_SPIRAL:
        while (k->shotsFired < KING_SPIRAL_COUNT && k->timer >= KING_SPIRAL_WINDUP + k->shotsFired * KING_SPIRAL_INTERVAL) {
            float angle = k->spiralAngle + k->shotsFired * 6.28318530718f / KING_SPIRAL_COUNT;
            King_SpawnShot(k, KING_SHOT_LASER, cosf(angle), sinf(angle), KING_SPIRAL_BOUNCES); ++k->shotsFired;
        }
        if (k->shotsFired == KING_SPIRAL_COUNT) King_Enter(k, KING_IDLE);
        break;
    case KING_BEAM_CHARGE:
        King_Aim(k, p);
        if (k->timer >= KING_BEAM_CHARGE_TIME) {
            King_SpawnShot(k, KING_SHOT_BEAM, k->aimX, k->aimY, 0); King_Enter(k, KING_BEAM_FIRE);
        }
        break;
    case KING_BEAM_FIRE: if (k->timer >= KING_BEAM_RECOVERY) King_Enter(k, KING_IDLE); break;
    default: break;
    }
    King_ResolvePlayerCollision(k, p);
}
#endif
