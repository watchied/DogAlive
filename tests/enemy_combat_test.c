#include <assert.h>
#include <stdio.h>
#include "src/combat/enemy_combat.h"

static Player player_at(float x, float y)
{
    return (Player){.x = x, .y = y, .hp = 100, .maxHP = 100,
        .attackDamage = 25, .direction = PLAYER_RIGHT};
}

int main(void)
{
    FleshSlime f;
    EyeParasite e;
    Ghoul g;
    SlimeShot shots[SLIME_SHOT_CAPACITY] = {0};
    Projectile arrows[MAX_PROJECTILES] = {0};
    Player p = player_at(0, 0);
    FleshSlime_Init(&f, 250, 0);
    for (int i = 0; i < 40; ++i) FleshSlime_Update(&f, &p, shots, 0.05f);
    assert(!shots[0].active); // The longer preparation must not release an early shot.
    for (int i = 0; i < 20; ++i) FleshSlime_Update(&f, &p, shots, 0.05f);
    assert(f.x < 250 && shots[0].active && shots[0].vx < 0);
    assert(fabsf(sqrtf(shots[0].vx * shots[0].vx + shots[0].vy * shots[0].vy) - 65.0f) < 0.01f);
    for (int i = 0; i < 100; ++i) SlimeShots_Update(shots, &p, 0.05f);
    assert(p.hp == 80 && !shots[0].active);

    p = player_at(20, 0);
    EyeParasite_Init(&e, 0, 0);
    SDL_FRect blast = EyeParasite_BlastBox(&e);
    assert(blast.w == 64 && blast.h == 64 && blast.x == -24 && blast.y == -24);
    Player edge = player_at(39, 0);
    assert(EyeParasite_BlastHitsPlayer(&e, &edge));
    edge = player_at(33, 33);
    assert(!EyeParasite_BlastHitsPlayer(&e, &edge));
    EyeParasite_Update(&e, &p, 0.05f);
    assert(e.state == EYE_PARASITE_WALK && e.proximityTimer > 0);
    for (int i = 0; i < 20; ++i) EyeParasite_Update(&e, &p, 0.05f);
    assert(p.hp == 100 && e.hp > 0 && e.state == EYE_PARASITE_ATTACK);
    for (int i = 0; i < 12; ++i) EyeParasite_Update(&e, &p, 0.05f);
    assert(p.hp == 70 && e.hp == 0);
    for (int i = 0; i < 30; ++i) EyeParasite_Update(&e, &p, 0.05f);
    assert(p.hp == 70 && e.deathFinished);

    // Escaping the marked circle avoids damage; killing an armed eye cancels it.
    p = player_at(20, 0);
    EyeParasite_Init(&e, 0, 0);
    EyeParasite_Update(&e, &p, EYE_PARASITE_DECISION_DELAY * 0.75f);
    assert(e.state == EYE_PARASITE_WALK);
    p.x = 100;
    EyeParasite_Update(&e, &p, 0.05f);
    assert(e.proximityTimer == 0 && e.state == EYE_PARASITE_WALK);
    p.x = e.x + 20;
    EyeParasite_Update(&e, &p, EYE_PARASITE_DECISION_DELAY * 0.75f);
    assert(e.state == EYE_PARASITE_WALK); // Previous visit must not count.
    EyeParasite_Update(&e, &p, EYE_PARASITE_DECISION_DELAY * 0.3f);
    assert(e.state == EYE_PARASITE_ATTACK);
    // Escaping after committing still permits the blast, but avoids its damage.
    p.x = e.x + 41;
    EyeParasite_Update(&e, &p, 2.0f);
    assert(p.hp == 100 && e.hp == 0);
    EyeParasite_Init(&e, 20, 0);
    EyeParasite_Update(&e, &p, 0.81f);
    assert(e.state == EYE_PARASITE_ATTACK);
    EyeParasite_TakeDamage(&e, 100, 0, 0);
    EyeParasite_Update(&e, &p, 2);
    assert(p.hp == 100 && e.deathFinished && e.explosionTimer == 0);

    // The Ghoul consuming the melee hit flag must not hide the other targets.
    p = player_at(0, 0);
    p.isAttacking = true; p.currentFrame = PLAYER_ATTACK_HIT_FRAME;
    Ghoul_Init(&g, 200, 200);
    FleshSlime_Init(&f, 16, 0);
    EyeParasite_Init(&e, 16, 0);
    Enemies_CheckPlayerAttack(&g, &f, &e, &p);
    assert(f.hp == 55 && e.hp == 25 && g.hp == 100 && p.attackHasHit);
    Enemies_CheckPlayerAttack(&g, &f, &e, &p);
    assert(f.hp == 55 && e.hp == 25);

    // Swept arrows hit the nearest living body, including Ghoul, once.
    for (int target = 0; target < 3; ++target) {
        Ghoul_Init(&g, target == 0 ? 30 : 90, 0);
        FleshSlime_Init(&f, target == 1 ? 30 : 110, 0);
        EyeParasite_Init(&e, target == 2 ? 30 : 130, 0);
        Projectiles_Reset(arrows);
        arrows[0] = (Projectile){.x = 0, .y = 8, .vx = 1000,
            .lifetime = 1, .damage = 10, .active = true};
        Enemies_UpdateArrows(arrows, &g, &f, &e, 0.2f);
        assert(!arrows[0].active);
        assert(g.hp == (target == 0 ? 90 : 100));
        assert(f.hp == (target == 1 ? 70 : 80));
        assert(e.hp == (target == 2 ? 40 : 50));
    }
    FleshSlime_TakeDamage(&f, 100, 0, 0);
    FleshSlime_Update(&f, &p, shots, 2);
    assert(f.deathFinished && f.hp == 0);
    FleshSlime_Init(&f, 270, 60);
    EyeParasite_Init(&e, 270, 195);
    assert(f.hp == 80 && !f.deathFinished && e.hp == 50 && !e.deathFinished);
    puts("Enemy combat tests passed");
    // Parry is tied to the sword hit beat and snapshots arrow damage.
    memset(shots, 0, sizeof(shots));
    p = player_at(0, 0);
    p.isAttacking = true;
    p.arrowDamage = 37;
    shots[0] = (SlimeShot){.x = 22, .y = 8, .vx = -65, .lifetime = 1,
        .damage = 20, .active = true};
    SlimeShots_CheckPlayerAttack(shots, &p);
    assert(!shots[0].reflected);
    p.currentFrame = PLAYER_ATTACK_HIT_FRAME;
    SlimeShots_CheckPlayerAttack(shots, &p);
    assert(shots[0].reflected && shots[0].vx == 65 && shots[0].damage == 37);
    assert(shots[0].hitFlashTimer > 0);
    SlimeShots_CheckPlayerAttack(shots, &p);
    assert(shots[0].vx == 65); // Already reflected bullets cannot flip back.
    Ghoul_Init(&g, 200, 0);
    FleshSlime_Init(&f, 50, 0);
    EyeParasite_Init(&e, 100, 0);
    SlimeShots_UpdateCombat(shots, &p, &g, &f, &e, 1);
    assert(f.hp == 43 && g.hp == 100 && e.hp == 50 && p.hp == 100);
    assert(!shots[0].active && f.hitFlashTimer > 0);
    // Reflected shots no longer hurt the player even when crossing their body.
    shots[0] = (SlimeShot){.x = 22, .y = 8, .vx = -65, .lifetime = 1,
        .damage = 37, .active = true, .reflected = true};
    SlimeShots_UpdateCombat(shots, &p, &g, &f, &e, 0.5f);
    assert(p.hp == 100);
    // A spent swing and a projectile outside the sword box cannot parry.
    shots[0].reflected = false;
    shots[0].x = 22;
    p.attackHasHit = true;
    SlimeShots_CheckPlayerAttack(shots, &p);
    assert(!shots[0].reflected);
    p.attackHasHit = false;
    shots[0].x = -20;
    SlimeShots_CheckPlayerAttack(shots, &p);
    assert(!shots[0].reflected);
    puts("Slime projectile parry tests passed");
    return 0;
}
