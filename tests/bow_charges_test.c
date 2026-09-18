#include <assert.h>
#include <stdio.h>
#include "src/combat/enemy_group.h"

int main(void)
{
    Player p;
    Player_Init(&p);
    Projectile shots[MAX_PROJECTILES] = {0};
    assert(p.bowCharges == 3);
    for (int i = 0; i < 3; ++i) {
        assert(Projectiles_Shoot(shots, &p));
        Projectiles_UpdateShooting(shots, &p, 0.3f);
        assert(shots[i].active && shots[i].type == ARROW_NORMAL);
    }
    assert(p.bowCharges == 0 && !Projectiles_Shoot(shots, &p));
    Projectiles_Recharge(&p, p.shootCooldown / 2);
    assert(p.bowCharges == 0);
    Projectiles_Recharge(&p, p.shootCooldown / 2);
    assert(p.bowCharges == 1);
    p.arrowType = ARROW_FIRE;
    assert(!Projectiles_Shoot(shots, &p));
    Projectiles_Recharge(&p, p.shootCooldown * 2);
    assert(p.bowCharges == 3 && p.shootTimer == 0);
    assert(Projectiles_Shoot(shots, &p) && p.bowCharges == 1);
    p.arrowType = ARROW_EXPLOSIVE;
    Projectiles_UpdateShooting(shots, &p, 0.3f);
    assert(shots[3].type == ARROW_FIRE);
    Projectiles_Recharge(&p, p.shootCooldown * 2);
    assert(Projectiles_Shoot(shots, &p) && p.bowCharges == 0);
    Projectiles_UpdateShooting(shots, &p, 0.3f);
    assert(shots[4].type == ARROW_EXPLOSIVE);
    // Full pool must not consume charges.
    Player_Init(&p);
    for (int i = 0; i < MAX_PROJECTILES; ++i) shots[i].active = true;
    assert(!Projectiles_Shoot(shots, &p) && p.bowCharges == 3);

    EnemyGroup g = {0};
    g.ghoulCount = 1;
    Ghoul_Init(&g.ghouls[0], 40, 0);
    Projectiles_Reset(shots);
    shots[0] = (Projectile){.x = 0, .y = 8, .vx = 1000, .lifetime = 1,
        .damage = 20, .active = true, .type = ARROW_FIRE};
    EnemyGroup_Arrows(&g, shots, 0.1f);
    assert(g.ghouls[0].hp == 80);
    EnemyGroup_UpdateArrowEffects(&g, 0.49f);
    assert(g.ghouls[0].hp == 80);
    EnemyGroup_UpdateArrowEffects(&g, 2.52f);
    assert(g.ghouls[0].hp == 50); // 20 impact + 30 burn = 2.5x.
    EnemyGroup_UpdateArrowEffects(&g, 10);
    assert(g.ghouls[0].hp == 50);

    g = (EnemyGroup){.ghoulCount = 1, .slimeCount = 1, .eyeCount = 1};
    Ghoul_Init(&g.ghouls[0], 40, 0);
    FleshSlime_Init(&g.slimes[0], 50, 20);
    EyeParasite_Init(&g.eyes[0], 100, 100);
    shots[0] = (Projectile){.x = 0, .y = 8, .vx = 1000, .lifetime = 1,
        .damage = 20, .active = true, .type = ARROW_EXPLOSIVE};
    EnemyGroup_Arrows(&g, shots, 0.1f);
    assert(g.ghouls[0].hp == 45); // 20 impact + 35 explosion.
    assert(g.slimes[0].hp == 45 && g.eyes[0].hp == 50);
    assert(g.explosions[0].timer > 0);
    EnemyGroup_Init(&g);
    assert(g.explosions[0].timer == 0 && g.burns[0][0].ticksLeft == 0);
    puts("Bow charges and elemental arrow tests passed");
    return 0;
}
