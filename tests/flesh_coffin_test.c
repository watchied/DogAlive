#include <assert.h>
#include <stdio.h>
#include "src/core/room_interactions.h"
#include "src/ui/flesh_coffin_draw.h"
static int shots(const FleshCoffin *c) { int n=0;for(int i=0;i<COFFIN_SHOT_CAPACITY;++i)n+=c->shots[i].active;return n; }
static void tick(FleshCoffin *c,Player *p,float time) { for(float t=0;t<time;t+=0.01f) { p->invincibilityTimer=100;Coffin_Update(c,p,0.01f); } }
static void combat(FleshCoffin *c,int phase) { Coffin_Init(c);c->phase=phase;Coffin_Enter(c,FC_IDLE); }
int main(void) {
    Player p;Player_Init(&p);p.x=24;p.y=180;
    FleshCoffin c;Coffin_Init(&c);
    c.phase=2;c.state=FC_IDLE;c.moving=false;
    assert(Coffin_Sprite(&c)==flesh_coffin_idle_phase2[0]);
    c.state=FC_EYE;
    assert(Coffin_Sprite(&c)==flesh_coffin_idle_phase2[0]);
    Coffin_Init(&c);
    assert(c.hp==1000 && c.swordHP==400 && !Coffin_Targetable(&c));
    assert(Coffin_Sprite(&c)==flesh_coffin_opening1[0]);
    tick(&c,&p,1);assert(c.state==FC_DORMANT);
    p.x=c.x-35;p.y=c.y;Coffin_Update(&c,&p,0.01f);assert(c.state==FC_INTRO);
    Coffin_Damage(&c,100);assert(c.hp==COFFIN_HP);
    tick(&c,&p,COFFIN_INTRO_TIME);assert(c.state==FC_IDLE);
    p.x=24;p.y=180;
    Coffin_Start(&c,&p,0);assert(c.state==FC_EYE);
    tick(&c,&p,COFFIN_EYE_TIME+COFFIN_RELEASE_TIME+0.02f);
    assert(c.state==FC_GROUND);
    for(int i=0;i<3;++i) assert(c.swords[i].active && c.swords[i].x>=COFFIN_GROUND_MARGIN && c.swords[i].y>=COFFIN_GROUND_MARGIN);
    tick(&c,&p,COFFIN_GROUND_INTERVAL);assert(c.count==1 && shots(&c)==3*COFFIN_GROUND_RAYS);
    tick(&c,&p,COFFIN_GROUND_INTERVAL*2);assert(c.count==3);
    tick(&c,&p,COFFIN_GROUND_INTERVAL+0.05f);assert(c.state==FC_RETURN);
    tick(&c,&p,COFFIN_RETURN_TIME);assert(c.state==FC_IDLE && !c.swords[0].active);
    combat(&c,1);Coffin_Start(&c,&p,1);
    tick(&c,&p,COFFIN_EYE_TIME+COFFIN_RELEASE_TIME+0.02f);
    assert(c.state==FC_STABS);
    tick(&c,&p,(COFFIN_STAB_WARNING+COFFIN_STAB_TRAVEL)*COFFIN_STAB_COUNT+0.1f);
    assert(c.count==9 && c.state==FC_RETURN);
    combat(&c,1);Coffin_Start(&c,&p,2);
    tick(&c,&p,COFFIN_EYE_TIME+COFFIN_RELEASE_TIME+COFFIN_WALL_WARNING+0.05f);
    assert(shots(&c)==3*COFFIN_WALL_RAYS);
    assert(c.wallEdges[0]!=c.wallEdges[1] && c.wallEdges[0]!=c.wallEdges[2] && c.wallEdges[1]!=c.wallEdges[2]);
    int perEdge[4]={0};
    for(int i=0;i<COFFIN_SHOT_CAPACITY;++i) if(c.shots[i].active) {
        CoffinShot *shot=&c.shots[i];
        int edge=shot->vx>0?0:shot->vx<0?1:shot->vy>0?2:3;
        ++perEdge[edge];
    }
    for(int i=0;i<3;++i) assert(perEdge[c.wallEdges[i]]==COFFIN_WALL_RAYS);
    for(int i=0;i<COFFIN_SHOT_CAPACITY;++i) if(c.shots[i].active) assert(!c.shots[i].reflectable);
    // Reflected mini blades hit the sword shield before boss HP.
    combat(&c,1);c.x=180;c.y=100;p.x=100;p.y=100;p.direction=PLAYER_RIGHT;p.isAttacking=true;p.currentFrame=PLAYER_ATTACK_HIT_FRAME;
    Coffin_Spawn(&c,120,108,-1,0,true);
    Coffin_UpdateShots(&c,&p,0.01f);assert(c.shots[0].reflected);
    Coffin_UpdateShots(&c,&p,1);assert(c.hp==COFFIN_HP && c.swordHP==COFFIN_SWORDS_HP-p.arrowDamage);
    Coffin_Spawn(&c,120,108,-1,0,false);Coffin_UpdateShots(&c,&p,0.01f);assert(!c.shots[0].reflected);
    p.isAttacking=false;
    combat(&c,1);Coffin_DamageSwords(&c,COFFIN_SWORDS_HP);
    assert(c.hp==COFFIN_HP && c.swordHP==0);
    for(int skill=0;skill<5;++skill) { Coffin_Start(&c,&p,skill);assert(c.skill==3 && c.state==FC_PORTAL); }
    tick(&c,&p,COFFIN_PORTAL_TIME+COFFIN_SLAM_FALL_TIME+0.05f);
    assert(c.state==FC_RECOVER && shots(&c)==0);
    combat(&c,1);Coffin_Start(&c,&p,3);
    tick(&c,&p,COFFIN_EYE_TIME+COFFIN_PORTAL_TIME+0.02f);
    p.x=GAME_WIDTH-25;p.y=GAME_HEIGHT-25;
    tick(&c,&p,COFFIN_SLAM_FALL_TIME+0.02f);
    assert(c.state==FC_RECOVER && shots(&c)==COFFIN_SLAM_RAYS);
    combat(&c,1);
    Coffin_Damage(&c,10);assert(c.hp==COFFIN_HP && c.swordHP==COFFIN_SWORDS_HP-10);
    Coffin_Damage(&c,COFFIN_SWORDS_HP+100);
    assert(c.hp==COFFIN_HP && c.swordHP==0 && c.phase==1);
    Coffin_Damage(&c,COFFIN_HP/4);
    assert(c.phase==2 && c.state==FC_TRANSITION && !Coffin_Targetable(&c));
    for(int i=0;i<FLESH_COFFIN_OPENING2_COUNT;++i) { c.timer=(i+0.5f)*COFFIN_TRANSITION_TIME/FLESH_COFFIN_OPENING2_COUNT;assert(Coffin_Sprite(&c)==flesh_coffin_opening2[i]); }
    c.timer=0;tick(&c,&p,COFFIN_TRANSITION_TIME);assert(c.state==FC_IDLE);
    // Delayed trail damage: no hit before two seconds, then one swept path hit.
    combat(&c,2);c.x=70;c.y=100;p.x=145;p.y=92;Coffin_Start(&c,&p,0);
    assert(c.state==FC_DASH);tick(&c,&p,COFFIN_DASH_TIME+0.01f);assert(c.trailActive);
    p.invincibilityTimer=0;int hp=p.hp;
    c.trailTimer=COFFIN_TRAIL_DELAY-0.02f;Coffin_Update(&c,&p,0.01f);assert(p.hp==hp);
    Coffin_Update(&c,&p,0.02f);assert(p.hp==hp-COFFIN_TRAIL_DAMAGE);
    // Parry is cancellation only, including main-loop player damage order.
    EnemyGroup g={0};combat(&g.coffin,2);g.coffin.x=125;g.coffin.y=108;g.coffin.direction=PLAYER_LEFT;
    Coffin_Enter(&g.coffin,FC_SLASH);g.coffin.timer=0.4f;
    Player_Init(&p);p.x=100;p.y=100;p.direction=PLAYER_RIGHT;p.isAttacking=true;p.currentFrame=PLAYER_ATTACK_HIT_FRAME;
    EnemyGroup_Melee(&g,&p);Coffin_Update(&g.coffin,&p,0.15f);
    assert(g.coffin.hp==COFFIN_HP && p.hp==p.maxHP && g.coffin.hit);
    p.isAttacking=false;
    combat(&c,2);Coffin_Start(&c,&p,2);tick(&c,&p,COFFIN_EYE_TIME+0.02f);
    int repeats=c.repeats;assert(repeats>=1 && repeats<=3);
    for(int i=0;i<repeats;++i) {
        tick(&c,&p,COFFIN_TELEPORT_TIME+0.02f);assert(c.state==FC_SLASH_WAIT);
        float standX=c.x,standY=c.y;
        tick(&c,&p,COFFIN_TELEPORT_SLASH_PAUSE*0.5f);
        assert(c.state==FC_SLASH_WAIT && c.x==standX && c.y==standY);
        tick(&c,&p,COFFIN_TELEPORT_SLASH_PAUSE*0.5f+0.02f);assert(c.state==FC_SLASH);
        tick(&c,&p,COFFIN_SLASH_TIME+0.02f);
    }
    assert(c.state==FC_IDLE);
    combat(&c,2);Coffin_Start(&c,&p,3);tick(&c,&p,COFFIN_EYE_TIME+COFFIN_TELEPORT_TIME+COFFIN_SLAM_FALL_TIME+0.05f);assert(c.state==FC_RECOVER);
    combat(&c,2);Coffin_Start(&c,&p,4);tick(&c,&p,COFFIN_EYE_TIME+0.01f);assert(c.state==FC_CHARGE);
    tick(&c,&p,COFFIN_CHARGE_TIME+0.01f);assert(c.state==FC_CHARGED_SLASH);
    tick(&c,&p,COFFIN_CHARGED_SLASH_TIME+0.01f);assert(c.state==FC_STUN);
    assert(Coffin_Sprite(&c)==flesh_coffin_stun[1]);
    float x=c.x,y=c.y;tick(&c,&p,COFFIN_STUN_TIME-0.1f);assert(c.state==FC_STUN && c.x==x && c.y==y);
    assert(Coffin_Sprite(&c)==flesh_coffin_stun[1]);
    Coffin_Damage(&c,COFFIN_HP);assert(c.state==FC_DEATH);tick(&c,&p,COFFIN_DEATH_TIME+0.01f);assert(c.state==FC_DEAD);
    // Arrows and shared sword HP route through the ordinary combat target list.
    memset(&g,0,sizeof(g));combat(&g.coffin,1);g.coffin.x=200;g.coffin.y=100;
    g.coffin.swords[0]=(CoffinSword){.x=100,.y=100,.active=true};
    assert(EnemyGroup_Sweep(&g,50,100,100,0,0.6f,0,0,20));
    assert(g.coffin.swordHP==COFFIN_SWORDS_HP-20 && g.coffin.hp==COFFIN_HP);
    g.coffin.swords[0].active=false;
    Projectile arrows[MAX_PROJECTILES]={0};arrows[0]=(Projectile){.x=100,.y=100,.vx=200,.damage=20,.lifetime=1,.active=true,.type=ARROW_FIRE};
    EnemyGroup_Arrows(&g,arrows,0.6f);assert(g.coffin.hp==COFFIN_HP && g.coffin.swordHP==COFFIN_SWORDS_HP-40);
    EnemyGroup_UpdateArrowEffects(&g,3.01f);assert(g.coffin.hp==COFFIN_HP && g.coffin.swordHP==COFFIN_SWORDS_HP-70);
    // Full test-room route, death gate and persistent defeated state.
    StageProgress stage={0};SlimeShot slime[SLIME_SHOT_CAPACITY]={0};RunningEffect effect={0};Player_Init(&p);
    Stage_Load(0,&p,&g,arrows,slime,&effect);
    SDL_FRect door=Stage_ExitBox(0);p.x=door.x;p.y=door.y;Stage_Update(&stage,&p,&g,arrows,slime,&effect);
    assert(stage.index==STAGE_COFFIN_ROOM && g.coffin.state==FC_DORMANT);
    door=Stage_ExitBox(stage.index);p.x=door.x;p.y=door.y;Stage_Update(&stage,&p,&g,arrows,slime,&effect);assert(stage.index==STAGE_COFFIN_ROOM);
    Coffin_Enter(&g.coffin,FC_IDLE);Coffin_DamageSwords(&g.coffin,COFFIN_SWORDS_HP);Coffin_Damage(&g.coffin,COFFIN_HP);assert(Stage_EnemiesAlive(&g)==1);
    tick(&g.coffin,&p,COFFIN_DEATH_TIME+0.01f);Stage_Update(&stage,&p,&g,arrows,slime,&effect);assert(stage.index==STAGE_BOSS_ROOM);
    door=Stage_BackBox(stage.index);p.x=door.x;p.y=door.y;Stage_Update(&stage,&p,&g,arrows,slime,&effect);
    assert(stage.index==STAGE_COFFIN_ROOM && g.coffin.state==FC_DEAD);
    combat(&c,1);c.state=FC_FALL;
    assert(Coffin_Sprite(&c)==flesh_coffin_idle_sword[0]);
    c.swordHP=0;assert(Coffin_Sprite(&c)==flesh_coffin_idle_nosword[0]);
    c.phase=2;assert(Coffin_Sprite(&c)==flesh_coffin_idle_phase2[0]);
    Player_Init(&p);p.x=100;p.y=100;
    Coffin_Slam(&c,&p);
    int patches=0;
    for(int i=0;i<COFFIN_DUST_CAPACITY;++i) if(c.dust[i].life>0) {
        ++patches;assert(c.dust[i].life==COFFIN_DUST_LIFETIME);
        assert(c.dust[i].x>=COFFIN_DUST_MARGIN && c.dust[i].x<=GAME_WIDTH-COFFIN_DUST_MARGIN);
    }
    assert(patches==COFFIN_DUST_COUNT);
    memset(c.dust,0,sizeof(c.dust));c.dustContactTimer=0;p.invincibilityTimer=0;
    c.dust[0]=c.dust[1]=(CoffinDust){108,108,COFFIN_DUST_LIFETIME};
    hp=p.hp;Coffin_UpdateDust(&c,&p,COFFIN_DUST_TICK_TIME/2);assert(p.hp==hp);
    Coffin_UpdateDust(&c,&p,COFFIN_DUST_TICK_TIME/2);assert(p.hp==hp-COFFIN_DUST_DAMAGE);
    p.invincibilityTimer=0;Coffin_UpdateDust(&c,&p,COFFIN_DUST_TICK_TIME);
    assert(p.hp==hp-2*COFFIN_DUST_DAMAGE);
    p.x=20;hp=p.hp;Coffin_UpdateDust(&c,&p,COFFIN_DUST_LIFETIME);
    assert(p.hp==hp && c.dust[0].life==0 && c.dustContactTimer==0);
    Coffin_SpawnDust(&c);Coffin_Enter(&c,FC_IDLE);Coffin_Damage(&c,COFFIN_HP);
    for(int i=0;i<COFFIN_DUST_CAPACITY;++i) assert(c.dust[i].life==0);
    // Phase 1 walking tests
    combat(&c, 1);
    c.x = 200; c.y = 100;
    p.x = 100; p.y = 100;
    Coffin_Update(&c, &p, 0.1f);
    assert(c.moving && c.x < 200);
    assert(Coffin_Sprite(&c) == flesh_coffin_wak_sword[(int)(c.walkTimer/0.1f)%FLESH_COFFIN_WAK_SWORD_COUNT]);
    c.state = FC_GROUND;
    c.swords[0].active = true;
    Coffin_Update(&c, &p, 0.1f);
    assert(c.moving);
    assert(Coffin_Sprite(&c) == flesh_coffin_wak_nosword[(int)(c.walkTimer/0.1f)%FLESH_COFFIN_WAK_NOSWORD_COUNT]);
    c.state = FC_IDLE;
    c.swords[0].active = false;
    c.swordHP = 0;
    c.x = 220;
    Coffin_Update(&c, &p, 0.1f);
    assert(c.moving);
    assert(Coffin_Sprite(&c) == flesh_coffin_wak_nosword[(int)(c.walkTimer/0.1f)%FLESH_COFFIN_WAK_NOSWORD_COUNT]);
    p.x = c.x - 20; p.y = c.y - 8;
    Coffin_Update(&c, &p, 0.1f);
    assert(!c.moving);
    assert(Coffin_Sprite(&c) == flesh_coffin_idle_nosword[0]);
    // Orbit swords stay evenly spaced, deal contact damage and return.
    combat(&c,1);c.x=160;c.y=120;Player_Init(&p);p.x=20;p.y=20;
    Coffin_Start(&c,&p,4);tick(&c,&p,COFFIN_EYE_TIME+COFFIN_RELEASE_TIME+0.02f);
    assert(c.state==FC_ORBIT);
    for(int i=0;i<3;++i) assert(fabsf(hypotf(c.swords[i].x-c.x,c.swords[i].y-c.y)-COFFIN_ORBIT_RADIUS)<0.01f);
    p.x=c.swords[0].x-8;p.y=c.swords[0].y-8;p.invincibilityTimer=0;
    hp=p.hp;Coffin_Update(&c,&p,0.01f);assert(p.hp==hp-COFFIN_ORBIT_DAMAGE);
    tick(&c,&p,COFFIN_ORBIT_TIME);assert(c.state==FC_RETURN);
    tick(&c,&p,COFFIN_RETURN_TIME);assert(c.state==FC_IDLE && !c.swords[0].active);
    combat(&c,1);Coffin_Enter(&c,FC_ORBIT);c.swords[0].active=true;
    Coffin_Damage(&c,COFFIN_SWORDS_HP);assert(c.state==FC_IDLE && !c.swords[0].active);
    // Nearby phase-two attacks use the longer windup and stop at melee distance.
    combat(&c,2);c.x=150;c.y=120;p.x=180;p.y=112;p.isAttacking=false;
    Coffin_Start(&c,&p,1);
    assert(c.skill==1 && c.state==FC_EYE);
    tick(&c,&p,COFFIN_EYE_TIME+0.01f);assert(c.state==FC_SLASH);
    x=c.x;y=c.y;hp=p.hp;p.invincibilityTimer=0;
    c.timer=COFFIN_CLOSE_SLASH_HIT_TIME-0.02f;
    Coffin_Update(&c,&p,0.01f);assert(p.hp==hp && hypotf(c.x-x,c.y-y)<=COFFIN_WALK_SPEED*COFFIN_SLASH_MOVE_MULTIPLIER*0.01f+0.001f);
    Coffin_Update(&c,&p,0.02f);assert(p.hp==hp-COFFIN_SLASH_DAMAGE);
    c.direction=PLAYER_RIGHT;c.timer=COFFIN_CLOSE_SLASH_HIT_TIME;
    assert(Coffin_Sprite(&c)==flesh_coffin_phase2Attack_swordFront[COFFIN_SLASH_HIT_FRAME]);
    c.timer=COFFIN_CLOSE_SLASH_TIME-0.001f;
    assert(Coffin_Sprite(&c)==flesh_coffin_phase2Attack_swordFront[FLESH_COFFIN_PHASE2ATTACK_SWORDFRONT_COUNT-1]);
    combat(&c,2);Player_Init(&p);
    int closeSlashes=0;
    for(int n=0;n<10000;++n) {
        c.x=150;c.y=120;p.x=180;p.y=112;
        c.trailActive=false;Coffin_Enter(&c,FC_IDLE);c.timer=COFFIN_REST_TIME;
        Coffin_Update(&c,&p,0.001f);
        closeSlashes+=c.skill==1;
    }
    assert(closeSlashes>2700 && closeSlashes<3300);
    combat(&c,1);p.x=152;p.y=112;
    unsigned directions=0;
    for(int n=0;n<200;++n) {
        Coffin_SetStab(&c,&p);
        CoffinSword *sword=&c.swords[0];
        float dx=sword->fromX-160,dy=sword->fromY-120;
        int direction=(int)lroundf(atan2f(dy,dx)*8/6.28318530718f);
        directions|=1u<<((direction+8)%8);
        assert(Coffin_Sweep(Player_Body(&p),sword->fromX,sword->fromY,sword->toX,sword->toY,0));
    }
    assert(directions==255);
    // Slam visuals and damage share the point between the legs.
    combat(&c,1);c.x=160;c.y=100;Player_Init(&p);
    p.x=152;p.y=Coffin_SlamY(&c)+COFFIN_SLAM_RADIUS-4;
    hp=p.hp;Coffin_Slam(&c,&p);
    assert(p.hp==hp-COFFIN_SLAM_DAMAGE);
    assert(c.slamEffectY==Coffin_SlamY(&c) && c.slamEffectTimer==COFFIN_SLAM_EFFECT_TIME);
    p.invincibilityTimer=0;p.y=c.y-COFFIN_SLAM_RADIUS-8;
    hp=p.hp;Coffin_Slam(&c,&p);assert(p.hp==hp);
    combat(&c,2);Player_Init(&p);p.x=220;p.y=112;
    c.x=100;c.y=120;c.direction=PLAYER_LEFT;c.skill=1;Coffin_Enter(&c,FC_SLASH);
    Coffin_Update(&c,&p,0.1f);
    assert(fabsf(c.x-100-COFFIN_WALK_SPEED*COFFIN_SLASH_MOVE_MULTIPLIER*0.1f)<0.001f);
    assert(c.direction==PLAYER_LEFT && c.moving);
    x=c.x;Coffin_Enter(&c,FC_CHARGED_SLASH);Coffin_Update(&c,&p,0.1f);
    assert(c.x==x && !c.moving);
    combat(&c,2);c.x=150;c.y=100;
    SDL_FRect body=Coffin_Body(&c);
    assert(body.x+body.w/2==c.x+COFFIN_BODY_OFFSET_X);
    assert(body.y+body.h/2==c.y+COFFIN_BODY_OFFSET_Y);
    Player_Init(&p);p.collisionGraceTimer=0;
    p.x=body.x+body.w/2-ACTOR_HALF_SIZE;
    p.y=body.y+body.h-PLAYER_HITBOX_OFFSET-2;
    Coffin_ResolvePlayerCollision(&c,&p);
    assert(!Coffin_Overlap(body,Player_Body(&p)));
    puts("Flesh Coffin dust DoT, fall sprites, skills, phases, parry, phase 1 walking and route passed");
    return 0;
}
