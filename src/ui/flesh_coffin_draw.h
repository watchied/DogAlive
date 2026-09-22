#ifndef FLESH_COFFIN_DRAW_H
#define FLESH_COFFIN_DRAW_H
#include "src/enemies/flesh_coffin.h"
#include "src/ui/slime_king_draw.h"
#include "assets/sprites/enemies/flesh coffin(boss).h"
#include "assets/sprites/projectiles/blood_spellblade.h"
#include "assets/sprites/projectiles/mini_blood_spellblade.h"
#include "assets/sprites/projectiles/blood_dust.h"
#include "src/effects/melee_slash.h"
#include "assets/sprites/effects/charge-slash.h"

static inline const uint16_t *Coffin_Sprite(const FleshCoffin *c) {
    bool front=c->direction==PLAYER_UP || c->direction==PLAYER_RIGHT;
    switch(c->state) {
    case FC_DORMANT: return flesh_coffin_opening1[0];
    case FC_INTRO: return flesh_coffin_opening1[King_VisualFrame(c->timer,COFFIN_INTRO_TIME,FLESH_COFFIN_OPENING1_COUNT)];
    case FC_TRANSITION: return flesh_coffin_opening2[King_VisualFrame(c->timer,COFFIN_TRANSITION_TIME,FLESH_COFFIN_OPENING2_COUNT)];
    case FC_DEATH: case FC_DEAD: return flesh_coffin_death[King_VisualFrame(c->timer,COFFIN_DEATH_TIME,FLESH_COFFIN_DEATH_COUNT)];
    case FC_EYE:
        // The exported eye-light animation contains the phase-one body.
        if(c->phase==2) return flesh_coffin_idle_phase2[0];
        return flesh_coffin_eye_lightup_before_attack[King_VisualFrame(c->timer,COFFIN_EYE_TIME,FLESH_COFFIN_EYE_LIGHTUP_BEFORE_ATTACK_COUNT)];
    case FC_RELEASE: return flesh_coffin_sword_up_before_attack1_2_3[King_VisualFrame(c->timer,COFFIN_RELEASE_TIME,FLESH_COFFIN_SWORD_UP_BEFORE_ATTACK1_2_3_COUNT)];
    case FC_RETURN: return flesh_coffin_sword_appear[King_VisualFrame(c->timer,COFFIN_RETURN_TIME,FLESH_COFFIN_SWORD_APPEAR_COUNT)];
    case FC_PORTAL:
        return c->swordHP>0?flesh_coffin_attack4_1_sword[King_VisualFrame(c->timer,COFFIN_PORTAL_TIME,FLESH_COFFIN_ATTACK4_1_SWORD_COUNT)]:
            flesh_coffin_attack4_1_nosword[King_VisualFrame(c->timer,COFFIN_PORTAL_TIME,FLESH_COFFIN_ATTACK4_1_NOSWORD_COUNT)];
    case FC_FALL: case FC_RECOVER:
        return c->phase==2?flesh_coffin_idle_phase2[0]:c->swordHP>0?flesh_coffin_idle_sword[0]:flesh_coffin_idle_nosword[0];
    case FC_TELEPORT:
        if (c->skill == 2) {
            float half = COFFIN_TELEPORT_TIME * 0.5f;
            if (c->timer < half) {
                int frame = (int)(c->timer / half * FLESH_COFFIN_PHASE2_TELEPORT_COUNT);
                if (frame >= FLESH_COFFIN_PHASE2_TELEPORT_COUNT) frame = FLESH_COFFIN_PHASE2_TELEPORT_COUNT - 1;
                return flesh_coffin_phase2_teleport[frame];
            } else {
                float p = fminf(1.0f, (c->timer - half) / half);
                int frame = (int)((1.0f - p) * (FLESH_COFFIN_PHASE2_TELEPORT_COUNT - 1));
                if (frame < 0) frame = 0;
                if (frame >= FLESH_COFFIN_PHASE2_TELEPORT_COUNT) frame = FLESH_COFFIN_PHASE2_TELEPORT_COUNT - 1;
                return flesh_coffin_phase2_teleport[frame];
            }
        }
        return flesh_coffin_phase2_teleport[King_VisualFrame(c->timer,COFFIN_TELEPORT_TIME,FLESH_COFFIN_PHASE2_TELEPORT_COUNT)];
    case FC_SLASH: {
        float hitTime=Coffin_SlashHitTime(c);
        int count=front?FLESH_COFFIN_PHASE2ATTACK_SWORDFRONT_COUNT:FLESH_COFFIN_PHASE2ATTACK_SWORDBACK_COUNT;
        int hitFrame=COFFIN_SLASH_HIT_FRAME<count?COFFIN_SLASH_HIT_FRAME:count-1;
        int frame;
        if(c->timer<hitTime) {
            frame=(int)(c->timer/hitTime*hitFrame);
        } else {
            float recovery=fmaxf(0.001f,Coffin_SlashDuration(c)-hitTime);
            frame=hitFrame+(int)((c->timer-hitTime)/recovery*(count-hitFrame));
        }
        if(frame>=count) frame=count-1;
        return front ? flesh_coffin_phase2Attack_swordFront[frame] :
            flesh_coffin_phase2Attack_swordBack[frame];
    }
    case FC_CHARGE: return flesh_coffin_phase_attack_front_charge[King_VisualFrame(c->timer,COFFIN_CHARGE_TIME,FLESH_COFFIN_PHASE_ATTACK_FRONT_CHARGE_COUNT)];
    case FC_CHARGED_SLASH: return front?flesh_coffin_phase2_aftercharge_front[King_VisualFrame(c->timer,COFFIN_CHARGED_SLASH_TIME,FLESH_COFFIN_PHASE2_AFTERCHARGE_FRONT_COUNT)]:
        flesh_coffin_phase2_aftercharge_back[King_VisualFrame(c->timer,COFFIN_CHARGED_SLASH_TIME,FLESH_COFFIN_PHASE2_AFTERCHARGE_BACK_COUNT)];
    case FC_STUN: return flesh_coffin_stun[1];
    case FC_SLASH_WAIT: return flesh_coffin_idle_phase2[0];
    case FC_DASH: return flesh_coffin_walk_phase2[(int)(c->timer/0.06f)%FLESH_COFFIN_WALK_PHASE2_COUNT];
    default:
        if(c->phase==2) return c->moving?flesh_coffin_walk_phase2[(int)(c->walkTimer/0.1f)%FLESH_COFFIN_WALK_PHASE2_COUNT]:flesh_coffin_idle_phase2[0];
        if(c->moving) {
            return c->swordHP>0 && !c->swords[0].active?
                flesh_coffin_wak_sword[(int)(c->walkTimer/0.1f)%FLESH_COFFIN_WAK_SWORD_COUNT]:
                flesh_coffin_wak_nosword[(int)(c->walkTimer/0.1f)%FLESH_COFFIN_WAK_NOSWORD_COUNT];
        }
        return c->swordHP>0 && !c->swords[0].active?flesh_coffin_idle_sword[0]:flesh_coffin_idle_nosword[0];
    }
}
static inline void Coffin_DrawGround(SDL_Renderer *r,const FleshCoffin *c) {
    if(!c->active) return;
    if(c->phase==2) {
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,235,30,45,85);
        if((c->state==FC_SLASH || c->state==FC_CHARGED_SLASH || c->state==FC_SLASH_WAIT) && !c->hit) {
            SDL_FRect box=Coffin_SlashBox(c);SDL_RenderFillRect(r,&box);
            SDL_SetRenderDrawColor(r,255,65,65,220);SDL_RenderRect(r,&box);
        }
        if(c->state==FC_FALL || c->state==FC_RECOVER) {
            for(int y=-(int)COFFIN_SLAM_RADIUS;y<(int)COFFIN_SLAM_RADIUS;++y) {
                float dy=y+0.5f,half=sqrtf(COFFIN_SLAM_RADIUS*COFFIN_SLAM_RADIUS-dy*dy);
                SDL_FRect row={Coffin_SlamX(c)-half,Coffin_SlamY(c)+y,half*2,1};SDL_RenderFillRect(r,&row);
            }
        }
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
    }
    for(int i=0;i<COFFIN_DUST_CAPACITY;++i) if(c->dust[i].life>0)
        King_DrawPixels(r,c->dust[i].x,c->dust[i].y,blood_dust_frames[0],BLOOD_DUST_WIDTH,BLOOD_DUST_HEIGHT,0,false);
}
static inline void Coffin_Draw(SDL_Renderer *r,const FleshCoffin *c) {
    if(!c->active || c->state==FC_DEAD) return;
    if(c->trailActive) {
        if(c->trailTimer<COFFIN_TRAIL_DELAY) {
            SDL_SetRenderDrawColor(r,160,30,60,255); SDL_RenderLine(r,c->dashX,c->dashY,c->trailX,c->trailY);
        } else {
            SDL_SetRenderDrawColor(r,255,100,110,255);
            SDL_RenderLine(r,c->dashX,c->dashY,c->trailX,c->trailY);
        }
    }
    if(c->state==FC_FALL || c->state==FC_RECOVER) {
        SDL_SetRenderDrawColor(r,255,65,80,255);King_DrawCircle(r,Coffin_SlamX(c),Coffin_SlamY(c),COFFIN_SLAM_RADIUS);
    }
    if(c->state==FC_STABS && c->timer<COFFIN_STAB_WARNING) {
        const CoffinSword *s=&c->swords[c->count%3];
        SDL_SetRenderDrawColor(r,240,90,90,255);SDL_RenderLine(r,s->fromX,s->fromY,s->toX,s->toY);
    }
    if(c->slamEffectTimer>0) {
        int frame=King_VisualFrame(COFFIN_SLAM_EFFECT_TIME-c->slamEffectTimer,
            COFFIN_SLAM_EFFECT_TIME,SLIME_KING_SLAM_FRAMES_COUNT);
        King_DrawPixels(r,c->slamEffectX,c->slamEffectY,slime_king_slam_frames[frame],
            SLIME_KING_SLAM_WIDTH,SLIME_KING_SLAM_HEIGHT,0,false);
    }
    float lift=c->state==FC_FALL?80*(1-fminf(1,c->timer/COFFIN_SLAM_FALL_TIME)):0;
    King_DrawPixels(r,c->x,c->y-lift,Coffin_Sprite(c),FLESH_COFFIN_WIDTH,FLESH_COFFIN_HEIGHT,0,c->flash>0);
    if(c->state==FC_SLASH || c->state==FC_CHARGED_SLASH) {
        float hitTime=Coffin_SlashHitTime(c);
        bool charged=c->state==FC_CHARGED_SLASH;
        int frame=MeleeSlash_Frame(c->timer-hitTime+MELEE_ACTOR_FRAME_TIME*MELEE_HIT_FRAME,
            charged?fire_slash_frames_duration_ms:monster_slash_frames_duration_ms,
            charged?FIRE_SLASH_FRAMES_COUNT:MONSTER_SLASH_FRAMES_COUNT);
        if(frame>=0) MeleeSlash_Draw(r,c->x-ACTOR_HALF_SIZE,c->y-ACTOR_HALF_SIZE,c->direction,
            charged?fire_slash_frames[frame]:monster_slash_frames[frame],
            charged?FIRE_SLASH_WIDTH:MONSTER_SLASH_WIDTH,charged?FIRE_SLASH_HEIGHT:MONSTER_SLASH_HEIGHT,
            (charged?COFFIN_CHARGED_REACH*CHARGE_SLASH_VISUAL_SCALE:COFFIN_SLASH_REACH)/(charged?FIRE_SLASH_WIDTH:MONSTER_SLASH_WIDTH),
            (charged?COFFIN_CHARGED_WIDTH*CHARGE_SLASH_VISUAL_SCALE:COFFIN_SLASH_WIDTH)/(charged?FIRE_SLASH_HEIGHT:MONSTER_SLASH_HEIGHT),
            (charged?COFFIN_CHARGED_REACH:COFFIN_SLASH_REACH)/2);
    }
    if(c->phase==2 && c->state==FC_EYE) {
        SDL_SetRenderDrawColor(r,255,110+(int)(100*fabsf(sinf(c->timer*12))),100,255);
        SDL_FRect eye={c->x-1,c->y-6,2,4};
        SDL_RenderFillRect(r,&eye);
    }
    for(int i=0;i<3;++i) if(c->swords[i].active) {
        const CoffinSword *s=&c->swords[i];
        float angle=c->state==FC_STABS?atan2f(s->toY-s->fromY,s->toX-s->fromX)+1.57079632679f:0;
        King_DrawPixels(r,s->x,s->y,blood_spellblade_frames[0],BLOOD_SPELLBLADE_WIDTH,BLOOD_SPELLBLADE_HEIGHT,angle,false);
    }
    for(int i=0;i<COFFIN_SHOT_CAPACITY;++i) if(c->shots[i].active) {
        const CoffinShot *s=&c->shots[i];
        King_DrawPixels(r,s->x,s->y,mini_blood_spellblade_frames[0],MINI_BLOOD_SPELLBLADE_WIDTH,MINI_BLOOD_SPELLBLADE_HEIGHT,
            atan2f(s->vy,s->vx)+1.57079632679f,s->reflected);
        if(!s->reflectable) { SDL_SetRenderDrawColor(r,255,40,40,255); SDL_FRect box={s->x-5,s->y-5,10,10};SDL_RenderRect(r,&box); }
    }
    if(c->state==FC_DORMANT || c->state==FC_INTRO) return;
    SDL_SetRenderDrawColor(r,245,200,200,255);SDL_RenderDebugTextFormat(r,48,GAME_HEIGHT-36,"FLESH COFFIN - PHASE %d",c->phase);
    SDL_FRect bar={40,GAME_HEIGHT-22,240,6};SDL_SetRenderDrawColor(r,50,15,25,255);SDL_RenderFillRect(r,&bar);
    bar.w*=c->hp/(float)COFFIN_HP;SDL_SetRenderDrawColor(r,200,35,65,255);SDL_RenderFillRect(r,&bar);
    if(c->phase==1) {
        SDL_SetRenderDrawColor(r,225,210,240,255);
        SDL_RenderDebugTextFormat(r,40,GAME_HEIGHT-49,"SWORDS %d / %d",c->swordHP,COFFIN_SWORDS_HP);
        bar=(SDL_FRect){40,GAME_HEIGHT-12,240,4};
        SDL_SetRenderDrawColor(r,45,35,55,255);SDL_RenderFillRect(r,&bar);
        bar.w*=c->swordHP/(float)COFFIN_SWORDS_HP;
        SDL_SetRenderDrawColor(r,195,170,205,255);SDL_RenderFillRect(r,&bar);
    }
}
#endif
