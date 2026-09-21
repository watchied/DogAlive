#ifndef FLESH_COFFIN_H
#define FLESH_COFFIN_H
#include "src/enemies/flesh_coffin_config.h"
#include "src/enemies/enemy_common.h"
#include <string.h>

typedef enum {
    FC_DORMANT, FC_INTRO, FC_IDLE, FC_EYE, FC_RELEASE, FC_GROUND, FC_STABS,
    FC_WALL, FC_RETURN, FC_PORTAL, FC_FALL, FC_RECOVER, FC_TRANSITION,
    FC_DASH, FC_SLASH, FC_TELEPORT, FC_CHARGE, FC_CHARGED_SLASH, FC_STUN, FC_DEATH, FC_DEAD
} CoffinState;
typedef struct { float x, y, vx, vy, life; int damage; bool active, reflectable, reflected; } CoffinShot;
typedef struct { float x, y, fromX, fromY, toX, toY; bool active; } CoffinSword;
typedef struct { float x, y, life; } CoffinDust;
typedef struct {
    bool active, hit, parried, moving;
    int hp, swordHP, phase, skill, lastSkill, count, repeats;
    int wallEdges[3];
    unsigned int parriedSwing;
    uint32_t rng;
    float x, y, timer, flash, aimX, aimY, targetX, targetY;
    float dashX, dashY, trailX, trailY, trailTimer;
    float dashDustDistance;
    bool trailActive, trailHit;
    PlayerDirection direction;
    CoffinState state;
    CoffinSword swords[3];
    CoffinShot shots[COFFIN_SHOT_CAPACITY];
    CoffinDust dust[COFFIN_DUST_CAPACITY];
    float dustContactTimer;
    float walkTimer;
} FleshCoffin;
static inline uint32_t Coffin_Random(FleshCoffin *c) { c->rng = c->rng * 1664525u + 1013904223u; return c->rng; }
static inline float Coffin_Range(FleshCoffin *c, float lo, float hi) { return lo + (Coffin_Random(c) % 10001) / 10000.0f * (hi - lo); }
static inline void Coffin_Enter(FleshCoffin *c, CoffinState state) { c->state = state; c->timer = 0; c->hit = false; }
static inline void Coffin_Clamp(FleshCoffin *c) {
    c->x = fmaxf(20, fminf(GAME_WIDTH - 20, c->x));
    c->y = fmaxf(26, fminf(GAME_HEIGHT - 26, c->y));
}
static inline SDL_FRect Coffin_Body(const FleshCoffin *c) { return (SDL_FRect){c->x - COFFIN_BODY_WIDTH / 2, c->y - COFFIN_BODY_HEIGHT / 2, COFFIN_BODY_WIDTH, COFFIN_BODY_HEIGHT}; }
static inline SDL_FRect Coffin_SwordBody(const CoffinSword *s) { return (SDL_FRect){s->x - 8, s->y - 16, 16, 32}; }
static inline bool Coffin_Targetable(const FleshCoffin *c) {
    return c->active && c->hp > 0 && c->state != FC_DORMANT && c->state != FC_INTRO && c->state != FC_TRANSITION && c->state != FC_TELEPORT && c->state != FC_PORTAL;
}
static inline void Coffin_ClearAttacks(FleshCoffin *c) {
    memset(c->shots, 0, sizeof(c->shots)); memset(c->swords, 0, sizeof(c->swords)); c->trailActive = false;
    memset(c->dust,0,sizeof(c->dust));c->dustContactTimer=0;
}
static inline void Coffin_Init(FleshCoffin *c) {
    *c = (FleshCoffin){.active = true, .hp = COFFIN_HP, .swordHP = COFFIN_SWORDS_HP,
        .phase = 1, .x = GAME_WIDTH - 42, .y = 48, .state = FC_DORMANT, .lastSkill = -1, .rng = 0xC0FF1u};
}
static inline void Coffin_Damage(FleshCoffin *c, int damage) {
    if (!Coffin_Targetable(c) || damage <= 0) return;
    c->hp = c->hp > damage ? c->hp - damage : 0; c->flash = 0.12f;
    if (!c->hp) { Coffin_ClearAttacks(c); Coffin_Enter(c, FC_DEATH); }
    else if (c->phase == 1 && c->hp <= COFFIN_HP * COFFIN_PHASE_RATIO) {
        c->phase = 2; Coffin_ClearAttacks(c); Coffin_Enter(c, FC_TRANSITION);
    }
}
static inline void Coffin_DamageSwords(FleshCoffin *c, int damage) {
    if (!c->active || c->phase != 1 || c->swordHP <= 0 || damage <= 0) return;
    c->swordHP = c->swordHP > damage ? c->swordHP - damage : 0;
    if (!c->swordHP) {
        memset(c->swords, 0, sizeof(c->swords));
        if (c->state == FC_RELEASE || c->state == FC_GROUND || c->state == FC_STABS || c->state == FC_WALL || c->state == FC_RETURN)
            Coffin_Enter(c, FC_IDLE);
    }
}
static inline void Coffin_Aim(FleshCoffin *c, const Player *p) {
    float dx = p->x + ACTOR_HALF_SIZE - c->x, dy = p->y + ACTOR_HALF_SIZE - c->y;
    float len = hypotf(dx, dy); c->aimX = len > 0 ? dx / len : 1; c->aimY = len > 0 ? dy / len : 0;
    c->direction = Enemy_Face(dx, dy);
}
static inline bool Coffin_Overlap(SDL_FRect a, SDL_FRect b) { return a.x < b.x+b.w && a.x+a.w > b.x && a.y < b.y+b.h && a.y+a.h > b.y; }
static inline SDL_FRect Coffin_SlashBox(const FleshCoffin *c) {
    float r = COFFIN_SLASH_REACH, w = COFFIN_SLASH_WIDTH;
    switch (c->direction) {
    case PLAYER_UP: return (SDL_FRect){c->x-w/2,c->y-r,w,r};
    case PLAYER_DOWN: return (SDL_FRect){c->x-w/2,c->y,w,r};
    case PLAYER_LEFT: return (SDL_FRect){c->x-r,c->y-w/2,r,w};
    default: return (SDL_FRect){c->x,c->y-w/2,r,w};
    }
}
static inline bool Coffin_Parry(FleshCoffin *c, Player *p) {
    if (c->parried && p->isAttacking && c->parriedSwing == p->meleeSwingId) return true;
    if (!Coffin_Targetable(c) || (c->state != FC_SLASH && c->state != FC_CHARGED_SLASH) || c->hit ||
        c->timer < COFFIN_SLASH_PARRY_START || !Player_ParryActive(p) ||
        !Coffin_Overlap(Player_AttackBox(p), Coffin_SlashBox(c))) return false;
    c->hit = true; c->parried = true; c->parriedSwing = p->meleeSwingId;
    c->flash = p->hitFlashTimer = PLAYER_PARRY_FLASH_TIME;
    Player_ShowParry(p,c->x,c->y);
    return true;
}
static inline void Coffin_Spawn(FleshCoffin *c, float x, float y, float ux, float uy, bool reflectable) {
    for (int i = 0; i < COFFIN_SHOT_CAPACITY; ++i) if (!c->shots[i].active) {
        c->shots[i] = (CoffinShot){.x=x,.y=y,.vx=ux*COFFIN_SHOT_SPEED,.vy=uy*COFFIN_SHOT_SPEED,
            .life=COFFIN_SHOT_LIFE,.damage=COFFIN_SHOT_DAMAGE,.active=true,.reflectable=reflectable}; return;
    }
}
static inline void Coffin_Radial(FleshCoffin *c, float x, float y, int count) {
    for (int i=0;i<count;++i) { float a = i*6.28318530718f/count; Coffin_Spawn(c,x,y,cosf(a),sinf(a),true); }
}
static inline bool Coffin_Sweep(SDL_FRect box, float x, float y, float nx, float ny, float radius) {
    box.x -= radius; box.y -= radius; box.w += radius*2; box.h += radius*2;
    return SDL_GetRectAndLineIntersectionFloat(&box,&x,&y,&nx,&ny);
}
static inline void Coffin_UpdateShots(FleshCoffin *c, Player *p, float dt) {
    for (int i=0;i<COFFIN_SHOT_CAPACITY;++i) {
        CoffinShot *s=&c->shots[i]; if (!s->active) continue;
        float step=fminf(dt,s->life), nx=s->x+s->vx*step, ny=s->y+s->vy*step;
        if (!s->reflected && s->reflectable && Player_ParryActive(p) && Coffin_Sweep(Player_AttackBox(p),s->x,s->y,nx,ny,4)) {
            float dx=c->x-s->x,dy=c->y-s->y,len=hypotf(dx,dy);
            s->vx=(len>0?dx/len:1)*COFFIN_SHOT_SPEED; s->vy=(len>0?dy/len:0)*COFFIN_SHOT_SPEED;
            s->reflected=true; s->damage=p->arrowDamage; s->life=COFFIN_SHOT_LIFE;
            continue;
        }
        if (s->reflected) {
            if (Coffin_Targetable(c) && Coffin_Sweep(Coffin_Body(c),s->x,s->y,nx,ny,4)) { s->active=false; Coffin_Damage(c,s->damage); }
        } else if (p->hp>0 && Coffin_Sweep(Player_Body(p),s->x,s->y,nx,ny,4)) {
            Enemy_HurtPlayer(p,s->damage,s->x,s->y); s->active=false;
        }
        s->x=nx; s->y=ny; s->life-=dt;
        if (s->life<=0 || nx<0 || nx>GAME_WIDTH || ny<0 || ny>GAME_HEIGHT) s->active=false;
    }
}
static inline void Coffin_AddDust(FleshCoffin *c,float x,float y) {
        int slot=0;
        for(int i=0;i<COFFIN_DUST_CAPACITY;++i) {
            if(c->dust[i].life<=0) { slot=i;break; }
            if(c->dust[i].life<c->dust[slot].life) slot=i;
        }
        c->dust[slot]=(CoffinDust){x,y,COFFIN_DUST_LIFETIME};
}
static inline void Coffin_SpawnDust(FleshCoffin *c) {
    for(int n=0;n<COFFIN_DUST_COUNT;++n)
        Coffin_AddDust(c,Coffin_Range(c,COFFIN_DUST_MARGIN,GAME_WIDTH-COFFIN_DUST_MARGIN),
            Coffin_Range(c,COFFIN_DUST_MARGIN,GAME_HEIGHT-COFFIN_DUST_MARGIN));
}
static inline void Coffin_UpdateDust(FleshCoffin *c,Player *p,float dt) {
    SDL_FRect body=Player_Body(p);
    float exposure=0,sourceX=0,sourceY=0;
    for(int i=0;i<COFFIN_DUST_CAPACITY;++i) {
        CoffinDust *d=&c->dust[i];if(d->life<=0) continue;
        float activeTime=fminf(dt,d->life);
        float dx=d->x-fmaxf(body.x,fminf(d->x,body.x+body.w));
        float dy=d->y-fmaxf(body.y,fminf(d->y,body.y+body.h));
        if(dx*dx+dy*dy<=COFFIN_DUST_RADIUS*COFFIN_DUST_RADIUS && activeTime>exposure) {
            exposure=activeTime;sourceX=d->x;sourceY=d->y;
        }
        d->life=fmaxf(0,d->life-dt);
    }
    if(exposure<=0 || p->hp<=0) { c->dustContactTimer=0;return; }
    c->dustContactTimer+=exposure;
    if(c->dustContactTimer>=COFFIN_DUST_TICK_TIME) {
        c->dustContactTimer=fmodf(c->dustContactTimer,COFFIN_DUST_TICK_TIME);
        // Overlapping patches share one tick; normal player invincibility applies.
        Enemy_HurtPlayer(p,COFFIN_DUST_DAMAGE,sourceX,sourceY);
    }
}
static inline void Coffin_Slam(FleshCoffin *c, Player *p) {
    float dx=c->x-fmaxf(p->x + PLAYER_HITBOX_OFFSET, fminf(c->x, p->x + PLAYER_HITBOX_OFFSET + PLAYER_HITBOX_SIZE));
    float dy=c->y-fmaxf(p->y + PLAYER_HITBOX_OFFSET, fminf(c->y, p->y + PLAYER_HITBOX_OFFSET + PLAYER_HITBOX_SIZE));
    if (dx*dx+dy*dy<=COFFIN_SLAM_RADIUS*COFFIN_SLAM_RADIUS) Enemy_HurtPlayer(p,COFFIN_SLAM_DAMAGE,c->x,c->y);
    if (c->phase==1 && c->swordHP>0) Coffin_Radial(c,c->x,c->y,COFFIN_SLAM_RAYS);
    if (c->phase==2) Coffin_SpawnDust(c);
}
static inline void Coffin_PlaceNear(FleshCoffin *c,const Player *p, bool front) {
    int side=front?p->direction:(int)(Coffin_Random(c)%4);
    c->x=p->x+ACTOR_HALF_SIZE+((side==PLAYER_RIGHT)-(side==PLAYER_LEFT))*COFFIN_TELEPORT_DISTANCE;
    c->y=p->y+ACTOR_HALF_SIZE+((side==PLAYER_DOWN)-(side==PLAYER_UP))*COFFIN_TELEPORT_DISTANCE;
    Coffin_Clamp(c); Coffin_Aim(c,p);
}
static inline void Coffin_WalkTowardsPlayer(FleshCoffin *c, const Player *p, float dt) {
    if (hypotf(p->x + 8 - c->x, p->y + 8 - c->y) > 36) {
        c->moving = true;
        Coffin_Aim(c, p);
        c->x += c->aimX * COFFIN_WALK_SPEED * dt;
        c->y += c->aimY * COFFIN_WALK_SPEED * dt;
        Coffin_Clamp(c);
    }
}
static inline void Coffin_Start(FleshCoffin *c, const Player *p,int skill) {
    c->skill= c->phase==1 && c->swordHP<=0 ? 3 : skill; c->lastSkill=c->skill; c->count=0;
    Coffin_Aim(c,p);
    if (c->phase==2 && c->skill==0) {
        c->dashX=c->x; c->dashY=c->y;
        c->targetX=fmaxf(20,fminf(GAME_WIDTH-20,c->x+c->aimX*COFFIN_DASH_SPEED*COFFIN_DASH_TIME));
        c->targetY=fmaxf(26,fminf(GAME_HEIGHT-26,c->y+c->aimY*COFFIN_DASH_SPEED*COFFIN_DASH_TIME));
        c->dashDustDistance=0;
        Coffin_Enter(c,FC_DASH);
    } else Coffin_Enter(c,FC_EYE);
}
static inline void Coffin_SetStab(FleshCoffin *c,const Player *p) {
    int side=(int)(Coffin_Random(c)%4), i=c->count%3;
    CoffinSword *s=&c->swords[i];
    float ux=(side==PLAYER_RIGHT)-(side==PLAYER_LEFT),uy=(side==PLAYER_DOWN)-(side==PLAYER_UP);
    s->fromX=fmaxf(20,fminf(GAME_WIDTH-20,p->x+8+ux*80));
    s->fromY=fmaxf(20,fminf(GAME_HEIGHT-20,p->y+8+uy*80));
    s->toX=fmaxf(12,fminf(GAME_WIDTH-12,p->x+8-ux*30));
    s->toY=fmaxf(12,fminf(GAME_HEIGHT-12,p->y+8-uy*30));
    s->x=s->fromX;s->y=s->fromY;s->active=true;c->hit=false;
}
static inline void Coffin_Update(FleshCoffin *c,Player *p,float dt) {
    if (!c->active || c->state==FC_DEAD) return;
    c->moving=false;
    c->flash=fmaxf(0,c->flash-dt);
    if (c->state==FC_DEATH) { c->timer+=dt; if(c->timer>=COFFIN_DEATH_TIME) Coffin_Enter(c,FC_DEAD); return; }
    if (p->hp<=0) return;
    Coffin_UpdateDust(c,p,dt);
    if(p->hp<=0) return;
    Coffin_UpdateShots(c,p,dt);
    if (c->state==FC_DEATH) return;
    if (c->trailActive) {
        c->trailTimer+=dt;
        if (!c->trailHit && c->trailTimer>=COFFIN_TRAIL_DELAY) {
            c->trailHit=true;
            if (Coffin_Sweep(Player_Body(p),c->dashX,c->dashY,c->trailX,c->trailY,COFFIN_TRAIL_RADIUS))
                Enemy_HurtPlayer(p,COFFIN_TRAIL_DAMAGE,c->dashX,c->dashY);
        }
        if(c->trailTimer>=COFFIN_TRAIL_DELAY+COFFIN_TRAIL_EFFECT_TIME) c->trailActive=false;
    }
    c->timer+=dt;
    switch(c->state) {
    case FC_DORMANT:
        if(hypotf(p->x+8-c->x,p->y+8-c->y)<=COFFIN_TRIGGER_RADIUS) Coffin_Enter(c,FC_INTRO);
        break;
    case FC_INTRO: if(c->timer>=COFFIN_INTRO_TIME) Coffin_Enter(c,FC_IDLE); break;
    case FC_TRANSITION: if(c->timer>=COFFIN_TRANSITION_TIME) Coffin_Enter(c,FC_IDLE); break;
    case FC_IDLE:
        if(!c->trailActive) Coffin_WalkTowardsPlayer(c,p,dt);
        if(c->timer>=COFFIN_REST_TIME && !c->trailActive) {
            int count=c->phase==1?4:5, skill=Coffin_Random(c)%count;
            if(skill==c->lastSkill) skill=(skill+1)%count;
            Coffin_Start(c,p,skill);
        } break;
    case FC_EYE:
        if(c->timer<COFFIN_EYE_TIME) break;
        if(c->phase==1) Coffin_Enter(c,c->skill==3?FC_PORTAL:FC_RELEASE);
        else if(c->skill==1) { Coffin_Aim(c,p); Coffin_Enter(c,FC_SLASH); }
        else if(c->skill==4) Coffin_Enter(c,FC_CHARGE);
        else { c->repeats=COFFIN_STRIKES_MIN+Coffin_Random(c)%(COFFIN_STRIKES_MAX-COFFIN_STRIKES_MIN+1); Coffin_Enter(c,FC_TELEPORT); }
        break;
    case FC_RELEASE:
        if(c->timer<COFFIN_RELEASE_TIME) break;
        for(int i=0;i<3;++i) c->swords[i]=(CoffinSword){.active=true,
            .x=Coffin_Range(c,COFFIN_GROUND_MARGIN,GAME_WIDTH-COFFIN_GROUND_MARGIN),
            .y=Coffin_Range(c,COFFIN_GROUND_MARGIN,GAME_HEIGHT-COFFIN_GROUND_MARGIN)};
        if(c->skill==0) Coffin_Enter(c,FC_GROUND);
        else if(c->skill==1) { Coffin_Enter(c,FC_STABS); Coffin_SetStab(c,p); }
        else {
            int edges[4]={0,1,2,3};
            for(int i=0;i<3;++i) {
                int pick=i+Coffin_Random(c)%(4-i), edge=edges[pick];
                edges[pick]=edges[i];edges[i]=edge;c->wallEdges[i]=edge;
                c->swords[i].x=edge<2?(edge==0?14:GAME_WIDTH-14):Coffin_Range(c,COFFIN_WALL_MARGIN,GAME_WIDTH-COFFIN_WALL_MARGIN);
                c->swords[i].y=edge<2?Coffin_Range(c,COFFIN_WALL_MARGIN,GAME_HEIGHT-COFFIN_WALL_MARGIN):(edge==2?14:GAME_HEIGHT-14);
            }
            Coffin_Enter(c,FC_WALL);
        } break;
    case FC_GROUND:
        Coffin_WalkTowardsPlayer(c,p,dt);
        if(c->timer>=(c->count+1)*COFFIN_GROUND_INTERVAL && c->count<COFFIN_GROUND_WAVES) {
            for(int i=0;i<3;++i) Coffin_Radial(c,c->swords[i].x,c->swords[i].y,COFFIN_GROUND_RAYS);
            ++c->count;
        }
        if(c->timer>=(COFFIN_GROUND_WAVES+1)*COFFIN_GROUND_INTERVAL) Coffin_Enter(c,FC_RETURN);
        break;
    case FC_STABS: {
        Coffin_WalkTowardsPlayer(c,p,dt);
        CoffinSword *s=&c->swords[c->count%3];
        float t=fmaxf(0,fminf(1,(c->timer-COFFIN_STAB_WARNING)/COFFIN_STAB_TRAVEL));
        float nx=s->fromX+(s->toX-s->fromX)*t,ny=s->fromY+(s->toY-s->fromY)*t;
        if(t>0 && !c->hit && Coffin_Sweep(Player_Body(p),s->x,s->y,nx,ny,7)) {
            Enemy_HurtPlayer(p,COFFIN_STAB_DAMAGE,s->x,s->y);c->hit=true;
        }
        s->x=nx;s->y=ny;
        if(c->timer>=COFFIN_STAB_WARNING+COFFIN_STAB_TRAVEL) {
            if(++c->count>=COFFIN_STAB_COUNT) Coffin_Enter(c,FC_RETURN);
            else { c->timer=0; Coffin_SetStab(c,p); }
        } break;
    }
    case FC_WALL:
        Coffin_WalkTowardsPlayer(c,p,dt);
        if(c->timer>=COFFIN_WALL_WARNING && !c->hit) {
            c->hit=true;
            for(int sword=0;sword<3;++sword) {
                int edge=c->wallEdges[sword];
                for(int i=0;i<COFFIN_WALL_RAYS;++i) {
                    // Randomize within separate bands to keep all three blades distinct.
                    float length=(edge<2?GAME_HEIGHT:GAME_WIDTH)-2*COFFIN_WALL_MARGIN;
                    float along=COFFIN_WALL_MARGIN+length*(i+Coffin_Range(c,0.1f,0.9f))/COFFIN_WALL_RAYS;
                    Coffin_Spawn(c,edge<2?(edge==0?10:GAME_WIDTH-10):along,
                        edge<2?along:(edge==2?10:GAME_HEIGHT-10),
                        edge==0?1:edge==1?-1:0,edge==2?1:edge==3?-1:0,false);
                }
            }
        }
        if(c->timer>=COFFIN_WALL_WARNING+0.5f) Coffin_Enter(c,FC_RETURN);
        break;
    case FC_RETURN:
        for(int i=0;i<3;++i) { float t=fminf(1,dt/fmaxf(0.001f,COFFIN_RETURN_TIME-c->timer+dt));
            c->swords[i].x+=(c->x-c->swords[i].x)*t;c->swords[i].y+=(c->y-c->swords[i].y)*t; }
        if(c->timer>=COFFIN_RETURN_TIME) { memset(c->swords,0,sizeof(c->swords));Coffin_Enter(c,FC_IDLE); } break;
    case FC_PORTAL: case FC_TELEPORT:
        if(c->state==FC_TELEPORT && c->skill==2) {
            float half=COFFIN_TELEPORT_TIME*0.5f;
            if(c->timer>=half) {
                if(!c->hit) {
                    c->hit=true;
                    Coffin_PlaceNear(c,p,false);
                } else {
                    Coffin_Aim(c,p);
                }
            }
            if(c->timer>=COFFIN_TELEPORT_TIME) {
                Coffin_Aim(c,p);
                Coffin_Enter(c,FC_SLASH);
            }
        } else if(c->timer>=(c->state==FC_PORTAL?COFFIN_PORTAL_TIME:COFFIN_TELEPORT_TIME)) {
            if(c->phase==1 || c->skill==3) {
                c->x=p->x+8;c->y=p->y+8;Coffin_Clamp(c);c->targetX=c->x;c->targetY=c->y;Coffin_Enter(c,FC_FALL);
            } else { Coffin_PlaceNear(c,p,false);Coffin_Enter(c,FC_SLASH); }
        } break;
    case FC_FALL:
        if(c->timer>=COFFIN_SLAM_FALL_TIME) { Coffin_Slam(c,p);Coffin_Enter(c,FC_RECOVER); } break;
    case FC_RECOVER: if(c->timer>=COFFIN_SLAM_RECOVERY) Coffin_Enter(c,FC_IDLE); break;
    case FC_DASH: {
        float progress=fminf(1,c->timer/COFFIN_DASH_TIME);
        float dx=c->targetX-c->dashX,dy=c->targetY-c->dashY,length=hypotf(dx,dy);
        c->x=c->dashX+dx*progress;c->y=c->dashY+dy*progress;
        while(length>0 && c->dashDustDistance<=length*progress) {
            float along=c->dashDustDistance/length;
            Coffin_AddDust(c,c->dashX+dx*along,c->dashY+dy*along);
            c->dashDustDistance+=COFFIN_DASH_DUST_SPACING;
        }
        if(c->timer>=COFFIN_DASH_TIME) {
            c->trailX=c->x;c->trailY=c->y;c->trailTimer=0;c->trailHit=false;c->trailActive=true;Coffin_Enter(c,FC_IDLE);
        } break;
    }
    case FC_CHARGE:
        if(c->timer>=COFFIN_CHARGE_TIME) { Coffin_PlaceNear(c,p,true);Coffin_Enter(c,FC_CHARGED_SLASH); } break;
    case FC_SLASH: case FC_CHARGED_SLASH: {
        bool charged=c->state==FC_CHARGED_SLASH;
        Coffin_Parry(c,p);
        if(!c->hit && c->timer>=(charged?COFFIN_CHARGED_HIT_TIME:COFFIN_SLASH_HIT_TIME)) {
            c->hit=true;
            if(Coffin_Overlap(Coffin_SlashBox(c),Player_Body(p)))
                Enemy_HurtPlayer(p,charged?COFFIN_CHARGED_DAMAGE:COFFIN_SLASH_DAMAGE,c->x,c->y);
        }
        if(c->timer>=(charged?COFFIN_CHARGED_SLASH_TIME:COFFIN_SLASH_TIME)) {
            if(charged) Coffin_Enter(c,FC_STUN);
            else if(c->skill==2 && --c->repeats>0) Coffin_Enter(c,FC_TELEPORT);
            else Coffin_Enter(c,FC_IDLE);
        } break;
    }
    case FC_STUN: if(c->timer>=COFFIN_STUN_TIME) Coffin_Enter(c,FC_IDLE); break;
    default: break;
    }
    if(c->moving) c->walkTimer+=dt;
}

static inline void Coffin_ResolvePlayerCollision(const FleshCoffin *c,Player *p) {
    if(!Coffin_Targetable(c) || p->hp<=0 || p->collisionGraceTimer>0 || c->state==FC_FALL || c->state==FC_DASH) return;
    SDL_FRect b=Coffin_Body(c);
    float dx=p->x+ACTOR_HALF_SIZE-c->x,dy=p->y+ACTOR_HALF_SIZE-c->y;
    float ox=(PLAYER_HITBOX_SIZE+b.w)/2-fabsf(dx),oy=(PLAYER_HITBOX_SIZE+b.h)/2-fabsf(dy);
    if(ox<=0 || oy<=0) return;
    if(ox<oy) p->x+=dx<0?-ox:ox;else p->y+=dy<0?-oy:oy;
}
#endif
