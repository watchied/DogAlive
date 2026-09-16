#include <assert.h>
#include <stdio.h>
#include "src/combat/enemy_group.h"

static void ghoul_hit(Player *p)
{
    Ghoul g;
    Ghoul_Init(&g, p->x, p->y);
    g.state = GHOUL_ATTACK;
    g.timer = GHOUL_ATTACK_WINDUP + 0.41f;
    Ghoul_Update(&g, p, 0);
    assert(g.hasHit);
}
int main(void)
{
    Player p;
    Player_Init(&p);
    ghoul_hit(&p);
    assert(p.hp == 80 && p.invincibilityTimer == PLAYER_INVINCIBILITY_TIME);
    float x = p.x, y = p.y;
    ghoul_hit(&p);
    Enemy_HurtPlayer(&p, 30, 0, 0);
    assert(p.hp == 80 && p.x == x && p.y == y);
    p.invincibilityTimer -= 0.29f;
    Enemy_HurtPlayer(&p, 30, 0, 0);
    assert(p.hp == 80);
    p.invincibilityTimer = fmaxf(0, p.invincibilityTimer - 0.02f);
    Enemy_HurtPlayer(&p, 30, 0, 0);
    assert(p.hp == 50 && p.invincibilityTimer == PLAYER_INVINCIBILITY_TIME);
    ghoul_hit(&p);
    assert(p.hp == 50);
    Player_Init(&p);
    assert(p.invincibilityTimer == 0);
    puts("Player invincibility tests passed");
    return 0;
}
