#include <assert.h>
#include <stdio.h>
#include "src/combat/enemy_group.h"

int main(void)
{
    EnemyGroup g;
    EnemyGroup_Init(&g);
    assert(g.ghoulCount == GHOUL_COUNT && g.slimeCount == FLESH_SLIME_COUNT && g.eyeCount == EYE_PARASITE_COUNT);
    Player p = {.hp = 100, .arrowDamage = 30, .attackDamage = 20,
        .isAttacking = true, .currentFrame = PLAYER_ATTACK_HIT_FRAME, .direction = PLAYER_RIGHT};
    g.ghoulCount = g.slimeCount = g.eyeCount = 2;
    for (int i = 0; i < 2; ++i) {
        Ghoul_Init(&g.ghouls[i], 17 + i, 0);
        FleshSlime_Init(&g.slimes[i], 17 + i, 0);
        EyeParasite_Init(&g.eyes[i], 17 + i, 0);
    }
    EnemyGroup_Melee(&g, &p);
    EnemyGroup_Melee(&g, &p);
    for (int i = 0; i < 2; ++i) {
        assert(g.ghouls[i].hp == GHOUL_MAX_HP - 20);
        assert(g.slimes[i].hp == FLEASH_SLIME_MAX_HP - 20);
        assert(g.eyes[i].hp == EYE_PARASITE_MAX_HP - 20);
    }
    // The second slot is closer; arrows must not favor array order.
    g.ghoulCount = g.eyeCount = 0;
    FleshSlime_Init(&g.slimes[0], 90, 0);
    FleshSlime_Init(&g.slimes[1], 40, 0);
    Projectile shots[MAX_PROJECTILES] = {0};
    shots[0] = (Projectile){.x = 0, .y = 8, .vx = 1000, .lifetime = 1, .damage = 80, .active = true};
    shots[1] = shots[0];
    EnemyGroup_Arrows(&g, shots, 0.2f);
    assert(g.slimes[0].hp == 0 && g.slimes[1].hp == 0);
    assert(!shots[0].active && !shots[1].active);
    FleshSlime_Init(&g.slimes[0], 90, 0);
    FleshSlime_Init(&g.slimes[1], 40, 0);
    SlimeShot slimeShots[SLIME_SHOT_CAPACITY] = {0};
    slimeShots[0] = (SlimeShot){.x = 0, .y = 8, .vx = 1000, .lifetime = 1,
        .damage = 30, .active = true, .reflected = true};
    EnemyGroup_Update(&g, &p, slimeShots, 0.1f);
    assert(g.slimes[0].hp == 80 && g.slimes[1].hp == 50 && p.hp == 100);
    g.slimeCount = 0;
    shots[0] = (Projectile){.x = 0, .vx = 20, .lifetime = 1, .active = true};
    EnemyGroup_Arrows(&g, shots, 0.1f);
    assert(shots[0].active && shots[0].x == 2);
    EnemyGroup_Update(&g, &p, slimeShots, 0.1f);
    EnemyGroup_Init(&g);
    for (int i = 0; i < g.slimeCount; ++i) assert(g.slimes[i].hp == 80 && !g.slimes[i].deathFinished);
    // Living enemies of different types must separate even at identical positions.
    g.ghoulCount = g.slimeCount = g.eyeCount = 1;
    Ghoul_Init(&g.ghouls[0], 0, 0);
    FleshSlime_Init(&g.slimes[0], 0, 0);
    EyeParasite_Init(&g.eyes[0], 0, 0);
    for (int i = 0; i < 60; ++i) EnemyGroup_Separate(&g, 0.016f);
    EnemyTarget bodies[ENEMY_TYPE_CAPACITY * 3];
    int count = EnemyGroup_Targets(&g, bodies);
    for (int i = 0; i < count; ++i) {
        assert(bodies[i].body.x >= 0 && bodies[i].body.y >= 0);
        for (int j = i + 1; j < count; ++j) {
            float dx = fabsf(bodies[i].body.x - bodies[j].body.x);
            float dy = fabsf(bodies[i].body.y - bodies[j].body.y);
            assert(dx >= ACTOR_SIZE - 0.01f || dy >= ACTOR_SIZE - 0.01f);
        }
    }
    // Nearby walkers spread before touching, but corpses do not obstruct them.
    g.eyeCount = 0;
    Ghoul_Init(&g.ghouls[0], 50, 50);
    FleshSlime_Init(&g.slimes[0], 70, 50);
    EnemyGroup_Separate(&g, 0.05f);
    assert(g.slimes[0].x - g.ghouls[0].x > 20);
    g.slimes[0].hp = 0;
    float previousX = g.ghouls[0].x;
    EnemyGroup_Separate(&g, 0.05f);
    assert(g.ghouls[0].x == previousX);
    // An attack pose is not moved by soft avoidance alone.
    FleshSlime_Init(&g.slimes[0], 70, 50);
    g.ghouls[0].state = GHOUL_ATTACK;
    EnemyGroup_Separate(&g, 0.05f);
    assert(g.ghouls[0].x == previousX);
    puts("Multiple-enemy and spacing tests passed");
    return 0;
}
