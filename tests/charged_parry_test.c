#include <assert.h>
#include "src/enemies/flesh_coffin.h"
int main(void) {
    FleshCoffin c; Coffin_Init(&c); c.phase=2;c.x=100;c.y=100;c.direction=PLAYER_RIGHT;
    Player p;Player_Init(&p);p.x=120;p.y=92;p.direction=PLAYER_LEFT;
    p.isAttacking=true;p.currentFrame=PLAYER_ATTACK_HIT_FRAME;
    Coffin_Enter(&c,FC_CHARGED_SLASH);c.timer=COFFIN_SLASH_PARRY_START+0.01f;
    assert(!Coffin_Parry(&c,&p) && !c.hit);
    p.chargedAttack=true;assert(Coffin_Parry(&c,&p));
    c.parried=false;p.chargedAttack=false;Coffin_Enter(&c,FC_SLASH);c.timer=COFFIN_SLASH_PARRY_START+0.01f;
    assert(Coffin_Parry(&c,&p));
}
