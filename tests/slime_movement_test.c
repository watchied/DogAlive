#include <assert.h>
#include <stdio.h>
#include "src/enemies/slime.h"
static SlimeShot shots[SLIME_SHOT_CAPACITY];
int main(void) {
    Player p;Player_Init(&p);p.x=150;p.y=110;
    Slime s;Slime_Init(&s,90,110);
    for(int i=0;i<1000;++i) FleshSlime_Update(&s,&p,shots,0.01f);
    assert(fabsf(hypotf(s.x-p.x,s.y-p.y)-60)<0.01f);
    assert(fabsf(s.y-110)>1);
    Slime_Init(&s,120,110);FleshSlime_Update(&s,&p,shots,0.1f);
    assert(hypotf(s.x-p.x,s.y-p.y)>30);
    Slime_Init(&s,30,110);FleshSlime_Update(&s,&p,shots,0.1f);
    assert(hypotf(s.x-p.x,s.y-p.y)<120);
    FleshSlime_Init(&s,90,110);FleshSlime_Update(&s,&p,shots,0.1f);
    assert(s.x<90 && s.y==110);
    puts("Slime circles without drifting, retreats nearby, approaches at distance; flesh slime unchanged.");
}
