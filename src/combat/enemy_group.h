#ifndef ENEMY_GROUP_H
#define ENEMY_GROUP_H
#include "src/combat/enemy_combat.h"

typedef struct { int damageLeft, ticksLeft; float timer; } EnemyBurn;
typedef struct { float x, y, timer; } ArrowExplosion;
typedef struct {
    Ghoul ghouls[ENEMY_TYPE_CAPACITY];
    FleshSlime slimes[ENEMY_TYPE_CAPACITY];
    EyeParasite eyes[ENEMY_TYPE_CAPACITY];
    int ghoulCount, slimeCount, eyeCount;
    EnemyBurn burns[3][ENEMY_TYPE_CAPACITY];
    ArrowExplosion explosions[MAX_PROJECTILES];
} EnemyGroup;

static inline void EnemyGroup_InitCounts(EnemyGroup *group, int ghouls, int slimes, int eyes)
{
    *group = (EnemyGroup){.ghoulCount = ghouls < 0 ? 0 : ghouls > ENEMY_TYPE_CAPACITY ? ENEMY_TYPE_CAPACITY : ghouls,
        .slimeCount = slimes < 0 ? 0 : slimes > ENEMY_TYPE_CAPACITY ? ENEMY_TYPE_CAPACITY : slimes,
        .eyeCount = eyes < 0 ? 0 : eyes > ENEMY_TYPE_CAPACITY ? ENEMY_TYPE_CAPACITY : eyes};
    int slot = 0;
    // Separate spawn cells on the right side of the arena, away from the player.
    for (int i = 0; i < group->ghoulCount; ++i, ++slot)
        Ghoul_Init(&group->ghouls[i], 172 + slot % 5 * 28, 36 + slot / 5 * 30);
    for (int i = 0; i < group->slimeCount; ++i, ++slot)
        FleshSlime_Init(&group->slimes[i], 172 + slot % 5 * 28, 36 + slot / 5 * 30);
    for (int i = 0; i < group->eyeCount; ++i, ++slot)
        EyeParasite_Init(&group->eyes[i], 172 + slot % 5 * 28, 36 + slot / 5 * 30);
}

static inline void EnemyGroup_Init(EnemyGroup *group)
{
    EnemyGroup_InitCounts(group, GHOUL_COUNT, FLESH_SLIME_COUNT, EYE_PARASITE_COUNT);
}

typedef struct { SDL_FRect body; int hp, type, index; } EnemyTarget;
static inline int EnemyGroup_Targets(EnemyGroup *g, EnemyTarget *targets)
{
    int n = 0;
    for (int i = 0; i < g->ghoulCount; ++i)
        targets[n++] = (EnemyTarget){{g->ghouls[i].x, g->ghouls[i].y, ACTOR_SIZE, ACTOR_SIZE}, g->ghouls[i].hp, 0, i};
    for (int i = 0; i < g->slimeCount; ++i)
        targets[n++] = (EnemyTarget){{g->slimes[i].x, g->slimes[i].y, ACTOR_SIZE, ACTOR_SIZE}, g->slimes[i].hp, 1, i};
    for (int i = 0; i < g->eyeCount; ++i)
        targets[n++] = (EnemyTarget){{g->eyes[i].x, g->eyes[i].y, ACTOR_SIZE, ACTOR_SIZE}, g->eyes[i].hp, 2, i};
    return n;
}
static inline void EnemyGroup_Damage(EnemyGroup *g, EnemyTarget t, int damage, float px, float py)
{
    if (t.type == 0) Ghoul_TakeDamage(&g->ghouls[t.index], damage, px, py);
    if (t.type == 1) FleshSlime_TakeDamage(&g->slimes[t.index], damage, px, py);
    if (t.type == 2) EyeParasite_TakeDamage(&g->eyes[t.index], damage, px, py);
}
static inline void EnemyGroup_Melee(EnemyGroup *g, Player *p)
{
    if (p->hp <= 0 || !p->isAttacking || p->attackHasHit || p->currentFrame < PLAYER_ATTACK_HIT_FRAME) return;
    p->attackHasHit = true;
    SDL_FRect box = Player_AttackBox(p);
    EnemyTarget targets[ENEMY_TYPE_CAPACITY * 3];
    int n = EnemyGroup_Targets(g, targets);
    float px = 6.0f * ((p->direction == PLAYER_RIGHT) - (p->direction == PLAYER_LEFT));
    float py = 6.0f * ((p->direction == PLAYER_DOWN) - (p->direction == PLAYER_UP));
    for (int i = 0; i < n; ++i)
        if (targets[i].hp > 0 && SDL_HasRectIntersectionFloat(&box, &targets[i].body))
            EnemyGroup_Damage(g, targets[i], p->attackDamage, px, py);
}
// Search every living enemy for the first intersection along this segment.
static inline bool EnemyGroup_FindHit(EnemyGroup *g, float x, float y, float vx, float vy,
    float step, float tip, float radius, EnemyTarget *target, float *hitX, float *hitY)
{
    float speed = sqrtf(vx * vx + vy * vy);
    float ux = speed > 0 ? vx / speed : 0, uy = speed > 0 ? vy / speed : 0;
    x += ux * tip; y += uy * tip;
    EnemyTarget targets[ENEMY_TYPE_CAPACITY * 3];
    int n = EnemyGroup_Targets(g, targets), nearest = -1;
    float best = INFINITY;
    for (int i = 0; i < n; ++i) {
        if (targets[i].hp <= 0) continue;
        SDL_FRect box = targets[i].body;
        box.x -= radius; box.y -= radius; box.w += radius * 2; box.h += radius * 2;
        float x1 = x, y1 = y, x2 = x + vx * step, y2 = y + vy * step;
        if (!SDL_GetRectAndLineIntersectionFloat(&box, &x1, &y1, &x2, &y2)) continue;
        float d = (x1 - x) * (x1 - x) + (y1 - y) * (y1 - y);
        if (d < best) { best = d; nearest = i; *hitX = x1; *hitY = y1; }
    }
    if (nearest < 0) return false;
    *target = targets[nearest];
    return true;
}
static inline bool EnemyGroup_Sweep(EnemyGroup *g, float x, float y, float vx, float vy,
                                     float step, float tip, float radius, int damage)
{
    EnemyTarget target;
    float hitX, hitY;
    if (!EnemyGroup_FindHit(g, x, y, vx, vy, step, tip, radius, &target, &hitX, &hitY)) return false;
    float speed = sqrtf(vx * vx + vy * vy);
    EnemyGroup_Damage(g, target, damage, speed > 0 ? vx / speed * (10.0f / 3.0f) : 0,
        speed > 0 ? vy / speed * (10.0f / 3.0f) : 0);
    return true;
}

static inline void EnemyGroup_ArrowImpact(EnemyGroup *g, EnemyTarget target, const Projectile *s, float x, float y)
{
    // Evaluate blast coverage before impact knockback changes target positions.
    EnemyTarget targets[ENEMY_TYPE_CAPACITY * 3];
    int n = EnemyGroup_Targets(g, targets);
    float speed = sqrtf(s->vx * s->vx + s->vy * s->vy);
    EnemyGroup_Damage(g, target, s->damage, speed > 0 ? s->vx / speed * (10.0f / 3.0f) : 0,
        speed > 0 ? s->vy / speed * (10.0f / 3.0f) : 0);
    if (s->type == ARROW_FIRE) {
        EnemyBurn *burn = &g->burns[target.type][target.index];
        burn->damageLeft += (int)lroundf(s->damage * FIRE_ARROW_BURN_MULTIPLIER);
        burn->ticksLeft = FIRE_ARROW_TICKS;
        burn->timer = FIRE_ARROW_TICK_TIME;
    } else if (s->type == ARROW_EXPLOSIVE) {
        for (int i = 0; i < n; ++i) {
            SDL_FRect b = targets[i].body;
            float dx = x - fmaxf(b.x, fminf(x, b.x + b.w));
            float dy = y - fmaxf(b.y, fminf(y, b.y + b.h));
            if (targets[i].hp > 0 && dx * dx + dy * dy <= EXPLOSIVE_ARROW_RADIUS * EXPLOSIVE_ARROW_RADIUS)
                EnemyGroup_Damage(g, targets[i], (int)lroundf(s->damage * EXPLOSIVE_ARROW_MULTIPLIER), 0, 0);
        }
        for (int i = 0; i < MAX_PROJECTILES; ++i)
            if (g->explosions[i].timer <= 0) {
                g->explosions[i] = (ArrowExplosion){x, y, 0.25f}; break;
            }
    }
}

static inline void EnemyGroup_UpdateArrowEffects(EnemyGroup *g, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i)
        g->explosions[i].timer = fmaxf(0, g->explosions[i].timer - dt);
    EnemyTarget targets[ENEMY_TYPE_CAPACITY * 3];
    int n = EnemyGroup_Targets(g, targets);
    for (int i = 0; i < n; ++i) {
        EnemyTarget t = targets[i];
        EnemyBurn *burn = &g->burns[t.type][t.index];
        if (t.hp <= 0) { *burn = (EnemyBurn){0}; continue; }
        burn->timer -= dt;
        while (burn->ticksLeft > 0 && burn->timer <= 0) {
            int damage = (burn->damageLeft + burn->ticksLeft - 1) / burn->ticksLeft;
            burn->damageLeft -= damage; --burn->ticksLeft;
            burn->timer += FIRE_ARROW_TICK_TIME;
            int *hp = t.type == 0 ? &g->ghouls[t.index].hp : t.type == 1 ? &g->slimes[t.index].hp : &g->eyes[t.index].hp;
            if (damage >= *hp) {
                EnemyGroup_Damage(g, t, damage, 0, 0);
                *burn = (EnemyBurn){0}; break;
            }
            // Burn ticks don't interrupt attacks or add knockback.
            *hp -= damage;
        }
    }
}
static inline void EnemyGroup_Arrows(EnemyGroup *g, Projectile *shots, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        Projectile *s = &shots[i];
        if (!s->active) continue;
        float step = fminf(dt, s->lifetime);
        EnemyTarget target;
        float hitX, hitY;
        if (EnemyGroup_FindHit(g, s->x, s->y, s->vx, s->vy, step, 5.5f, 0, &target, &hitX, &hitY)) {
            EnemyGroup_ArrowImpact(g, target, s, hitX, hitY);
            s->active = false; continue;
        }
        s->x += s->vx * step; s->y += s->vy * step; s->lifetime -= dt;
        if (s->lifetime <= 0) s->active = false;
    }
}
typedef struct { float *x, *y; bool walking; } EnemyBody;

static inline void EnemyGroup_Separate(EnemyGroup *g, float dt)
{
    EnemyBody bodies[ENEMY_TYPE_CAPACITY * 3];
    int count = 0;
    for (int i = 0; i < g->ghoulCount; ++i)
        if (g->ghouls[i].hp > 0) bodies[count++] = (EnemyBody){&g->ghouls[i].x, &g->ghouls[i].y, g->ghouls[i].state == GHOUL_WALK};
    for (int i = 0; i < g->slimeCount; ++i)
        if (g->slimes[i].hp > 0) bodies[count++] = (EnemyBody){&g->slimes[i].x, &g->slimes[i].y,
            g->slimes[i].state == FLEASH_SLIME_WALK || g->slimes[i].state == FLEASH_SLIME_REST};
    for (int i = 0; i < g->eyeCount; ++i)
        if (g->eyes[i].hp > 0) bodies[count++] = (EnemyBody){&g->eyes[i].x, &g->eyes[i].y,
            g->eyes[i].state == EYE_PARASITE_WALK && g->eyes[i].proximityTimer == 0};
    float forceX[ENEMY_TYPE_CAPACITY * 3] = {0}, forceY[ENEMY_TYPE_CAPACITY * 3] = {0};
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            float dx = *bodies[i].x - *bodies[j].x, dy = *bodies[i].y - *bodies[j].y;
            float distance = sqrtf(dx * dx + dy * dy);
            if (distance >= ENEMY_SPACING) continue;
            if (distance < 0.001f) { dx = (i + j) % 2 ? 1 : 0; dy = dx == 0 ? 1 : 0; }
            else { dx /= distance; dy /= distance; }
            float strength = 1 - distance / ENEMY_SPACING;
            forceX[i] += dx * strength; forceY[i] += dy * strength;
            forceX[j] -= dx * strength; forceY[j] -= dy * strength;
        }
    }
    for (int i = 0; i < count; ++i) {
        if (!bodies[i].walking) continue;
        float length = sqrtf(forceX[i] * forceX[i] + forceY[i] * forceY[i]);
        float scale = ENEMY_SEPARATION_SPEED * dt / fmaxf(1, length);
        *bodies[i].x = fmaxf(0, fminf(GAME_WIDTH - ACTOR_SIZE, *bodies[i].x + forceX[i] * scale));
        *bodies[i].y = fmaxf(0, fminf(GAME_HEIGHT - ACTOR_SIZE, *bodies[i].y + forceY[i] * scale));
    }
    // Resolve solid 16x16 bodies after steering. Repeat to settle crowded groups.
    for (int pass = 0; pass < 64; ++pass) {
        bool overlap = false;
        for (int i = 0; i < count; ++i) {
            for (int j = i + 1; j < count; ++j) {
                float dx = *bodies[i].x - *bodies[j].x, dy = *bodies[i].y - *bodies[j].y;
                float ox = ACTOR_SIZE - fabsf(dx), oy = ACTOR_SIZE - fabsf(dy);
                if (ox <= 0.001f || oy <= 0.001f) continue;
                overlap = true;
                bool horizontal = ox < oy;
                float *a = horizontal ? bodies[i].x : bodies[i].y;
                float *b = horizontal ? bodies[j].x : bodies[j].y;
                float limit = (horizontal ? GAME_WIDTH : GAME_HEIGHT) - ACTOR_SIZE;
                float sign = (horizontal ? dx : dy) < 0 ? -1 : 1;
                float amount = (horizontal ? ox : oy) + 0.001f;
                // Prefer moving the walker rather than a monster holding an attack pose.
                float share = bodies[i].walking == bodies[j].walking ? 0.5f : bodies[i].walking ? 1 : 0;
                float oldA = *a, oldB = *b;
                *a = fmaxf(0, fminf(limit, oldA + sign * amount * share));
                *b = fmaxf(0, fminf(limit, oldB - sign * (amount - fabsf(*a - oldA))));
                float remaining = amount - fabsf(*a - oldA) - fabsf(*b - oldB);
                if (remaining > 0) *a = fmaxf(0, fminf(limit, *a + sign * remaining));
            }
        }
        if (!overlap) break;
    }
}

static inline void EnemyGroup_Update(EnemyGroup *g, Player *p, SlimeShot *shots, float dt)
{
    for (int i = 0; i < g->ghoulCount; ++i) {
        Ghoul_Update(&g->ghouls[i], p, dt);
        Ghoul_ResolvePlayerCollision(&g->ghouls[i], p);
    }
    for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
        SlimeShot *s = &shots[i];
        if (s->active && s->reflected && p->hp > 0 &&
            EnemyGroup_Sweep(g, s->x, s->y, s->vx, s->vy, fminf(dt, s->lifetime), 0, 2, s->damage))
            s->active = false;
    }
    SlimeShots_Update(shots, p, dt);
    for (int i = 0; i < g->slimeCount; ++i) FleshSlime_Update(&g->slimes[i], p, shots, dt);
    for (int i = 0; i < g->eyeCount; ++i) EyeParasite_Update(&g->eyes[i], p, dt);
    if (p->hp > 0 && dt > 0) EnemyGroup_Separate(g, dt);
}
#endif
