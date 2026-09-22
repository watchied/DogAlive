#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "src/core/game_core.h"
#include "src/player/bow_input.h"
#include "src/core/room_interactions.h"
int main(void) {
    Player p;Player_Init(&p);p.maxHP=p.hp=100;
    assert(p.healingPotions==2);
    assert(!Player_UsePotion(&p));
    p.hp=20;assert(Player_UsePotion(&p) && p.hp==20 && p.healingPotions==1);
    assert(!Player_UsePotion(&p) && p.healingPotions==1);
    bool keys[SDL_SCANCODE_COUNT]={0};keys[SDL_SCANCODE_D]=true;
    float oldX=p.x;p.isSprinting=true;p.sprintButton=8;
    Game_Update(&p,keys,0.5f);
    assert(!p.isSprinting);
    assert(fabsf(p.x-oldX-p.speed*PLAYER_POTION_MOVE_MULTIPLIER*0.5f)<0.001f);
    Player_UpdatePotion(&p,1.0f);assert(p.hp==20);
    Player_UpdatePotion(&p,0.0f);assert(p.hp==20 && p.potionUseTimer==1.0f);
    Player_UpdatePotion(&p,1.0f);assert(p.hp==70 && p.potionUseTimer==0);
    Player_UpdatePotion(&p,5.0f);assert(p.hp==70);
    oldX=p.x;Game_Update(&p,keys,0.5f);
    assert(fabsf(p.x-oldX-p.speed*0.5f)<0.001f);
    assert(Player_UsePotion(&p) && p.hp==70 && p.healingPotions==0); Player_UpdatePotion(&p,2.1f); assert(p.hp==p.maxHP);
    p.hp=20;assert(!Player_UsePotion(&p));
    p.healingPotions=1;p.hp=0;assert(!Player_UsePotion(&p));
    p.hp=20;p.isCharging=true;assert(!Player_UsePotion(&p));p.isCharging=false;
    Player_UnlockArrow(&p,ARROW_NORMAL);p.unlockedArrows=1;p.arrowType=ARROW_NORMAL;p.potionSelected=false;
    BowInput input={0};BowInput_Press(&input,0);BowInput_Update(&input,&p,500);
    assert(p.potionSelected && !BowInput_Release(&input,&p,501));
    BowInput_Press(&input,600);assert(BowInput_Release(&input,&p,650));
    assert(Player_UsePotion(&p));
    BowInput_Press(&input,1000);BowInput_Update(&input,&p,1500);assert(!p.potionSelected);
    p.hp=0;Player_UpdatePotion(&p,3.0f);assert(p.hp==0 && p.potionUseTimer==0);
    FleshCoffin c;Coffin_Init(&c);c.phase=2;c.x=100;c.y=100;c.direction=PLAYER_RIGHT;
    Coffin_Enter(&c,FC_CHARGED_SLASH);c.timer=COFFIN_SLASH_PARRY_START+0.01f;
    p.hp=100;p.x=100+COFFIN_CHARGED_REACH-8;p.y=92;p.direction=PLAYER_LEFT;
    p.isAttacking=true;p.currentFrame=PLAYER_ATTACK_HIT_FRAME;
    p.chargedAttack=false;assert(!Coffin_Parry(&c,&p) && !c.hit && !c.parried);
    p.chargedAttack=true;assert(Coffin_Parry(&c,&p));
    c.parried=false;Coffin_Enter(&c,FC_SLASH);c.timer=COFFIN_SLASH_PARRY_START+0.01f;
    p.chargedAttack=false;assert(!Coffin_Parry(&c,&p));
    p.x=120;assert(Coffin_Parry(&c,&p));
    puts("Potion use, item cycling and extended charged parry passed");
}
