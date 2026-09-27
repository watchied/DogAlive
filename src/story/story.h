#ifndef STORY_H
#define STORY_H
#include "src/story/story_config.h"
#include "src/core/stages.h"
#include "src/ui/flesh_coffin_draw.h"
#include "assets/sprites/npc/dog.h"
#include "assets/sprites/player/knigh(daughter).h"
#include "assets/sprites/player/player_sprites.h"
#include "assets/sprites/cutcene/dungeon entrance.h"
typedef enum {
    STORY_NORMAL,STORY_RESCUE,STORY_RETURN,STORY_EXIT_WALK,STORY_DEATH,STORY_DOG_APPROACH,
    STORY_FADE,STORY_SLIDES,STORY_EPILOGUE,STORY_SKY,STORY_WHITE
} StoryMode;
typedef struct {
    StoryMode mode;
    float timer,clock,dogX,dogY,dogAttackTimer,dogSwoop,idleTime,interactTimer;
    float knightX,knightY,knightAnim;
    int rescueStep,interaction,lastRoom;
    bool helper,dogMoving,dogHit,spaceDown,knightMoving,endingRestart;
    bool dogFacingLeft,knightFacingLeft;
} Story;
typedef struct { SDL_Texture *frames[DUNGEON_ENTRANCE_FRAMES_COUNT]; } StoryArt;
static inline int Story_Frame(float t,const uint32_t *times,int count) {
    for(int i=0;i<count;++i) { t-=times[i]/1000.0f;if(t<0) return i; }
    return count-1;
}
static inline float Story_Duration(const uint32_t *times,int count) {
    float t=0;for(int i=0;i<count;++i)t+=times[i]/1000.0f;return t;
}
static inline void Story_Enter(Story *s,StoryMode mode) { s->mode=mode;s->timer=0; }
static inline bool Story_Locked(const Story *s) { return s->mode>=STORY_EXIT_WALK || s->mode==STORY_RESCUE; }
static inline float Story_TimeScale(const Story *s) {
    return s->mode==STORY_RESCUE && s->rescueStep>=2?RESCUE_PARRY_TIME_SCALE:1.0f;
}
static inline bool Story_MoveDog(Story *s,float x,float y,float speed,float dt) {
    float dx=x-s->dogX,dy=y-s->dogY,d=hypotf(dx,dy);
    float step=fminf(d,speed*dt);s->dogMoving=step>0.01f;
    if(s->dogMoving && fabsf(dx)>0.01f) s->dogFacingLeft=dx<0;
    if(d>0) { s->dogX+=dx/d*step;s->dogY+=dy/d*step; }
    return d<=step+1;
}
static inline SDL_FRect Story_DogBody(const Story *s) {
    return (SDL_FRect){s->dogX-DOG_BODY_SIZE/2,s->dogY-DOG_BODY_SIZE/2,DOG_BODY_SIZE,DOG_BODY_SIZE};
}
static inline void Story_DogCollision(Story *s,StageProgress *stage,Player *p,EnemyGroup *g) {
    if(!s->helper || s->mode!=STORY_NORMAL || p->hp<=0) return;
    Stage_BlockEnemyBody(&roomDefinitions[stage->index],&s->dogX,&s->dogY,Story_DogBody(s));
    FleshCoffin *c=&g->coffin;
    if(Coffin_Targetable(c) && c->state!=FC_FALL && c->state!=FC_DASH) {
        SDL_FRect b=Coffin_Body(c);
        float dx=s->dogX-b.x-b.w/2,dy=s->dogY-b.y-b.h/2;
        float ox=(DOG_BODY_SIZE+b.w)/2-fabsf(dx),oy=(DOG_BODY_SIZE+b.h)/2-fabsf(dy);
        if(ox>0 && oy>0) { if(ox<oy)s->dogX+=dx<0?-ox:ox;else s->dogY+=dy<0?-oy:oy; }
    }
    s->dogX=fmaxf(10,fminf(GAME_WIDTH-10,s->dogX));
    s->dogY=fmaxf(10,fminf(GAME_HEIGHT-10,s->dogY));
    SDL_FRect body=Player_Body(p),dog=Story_DogBody(s);
    float dx=body.x+body.w/2-s->dogX,dy=body.y+body.h/2-s->dogY;
    float ox=(body.w+dog.w)/2-fabsf(dx),oy=(body.h+dog.h)/2-fabsf(dy);
    if(ox>0 && oy>0) { if(ox<oy)p->x+=dx<0?-ox:ox;else p->y+=dy<0?-oy:oy; }
}
static inline void Story_BeginRescue(Story *s,Player *p,FleshCoffin *c) {
    Story_Enter(s,STORY_RESCUE);s->rescueStep=0;
    s->dogX=-16;s->dogY=p->y+8;
    p->isAttacking=p->isCharging=p->isShooting=p->isSprinting=p->isMoving=false;
    p->chargeTimer=0;
    Coffin_ClearAttacks(c);c->moving=false;c->flash=0;Coffin_Enter(c,FC_IDLE);
}
static inline void Story_Update(Story *s,StageProgress *stage,Player *p,EnemyGroup *g,const bool *keys,float dt) {
    dt*=Story_TimeScale(s);
    s->clock+=dt;s->timer+=dt;s->dogMoving=false;
    bool press=keys[SDL_SCANCODE_SPACE] && !s->spaceDown;s->spaceDown=keys[SDL_SCANCODE_SPACE];
    FleshCoffin *c=&g->coffin;
    if(s->mode==STORY_NORMAL && c->active && c->rescueRequested) Story_BeginRescue(s,p,c);
    if(s->mode==STORY_RESCUE) {
        p->isMoving=p->isSprinting=false;
        c->flash=0;
        if(s->rescueStep==0 && s->timer>=RESCUE_RESTRAIN_TIME) {
            s->rescueStep=1;s->timer=0;
            c->x=p->x+48;c->y=p->y+8;Coffin_Clamp(c);Coffin_Aim(c,p);Coffin_Enter(c,FC_CHARGE);
        } else if(s->rescueStep==1) {
            c->timer=s->timer;
            if(s->timer>=RESCUE_CHARGE_TIME) { s->rescueStep=2;s->timer=0;Coffin_Enter(c,FC_CHARGED_SLASH); }
        } else if(s->rescueStep==2) {
            c->timer=s->timer;
            Story_MoveDog(s,p->x+20,p->y+8,600,dt);
            if(s->timer>=RESCUE_PARRY_TIME) {
                c->hit=true;c->parried=true;Coffin_Enter(c,FC_STUN);c->hit=true;
                Player_ShowParry(p,s->dogX,s->dogY);
                p->parryEffectX=s->dogX;p->parryEffectY=s->dogY;p->chargedParryEffect=true;
                s->rescueStep=3;s->timer=0;
            }
        } else if(s->rescueStep==3 && s->timer>=RESCUE_FINISH_TIME) {
            c->rescueRequested=false;c->rescueDone=true;c->parried=false;Coffin_Enter(c,FC_STUN);
            s->helper=true;s->dogAttackTimer=0;s->dogSwoop=0;
            p->invincibilityTimer=PLAYER_INVINCIBILITY_TIME;
            p->attackWasDown=keys[SDL_SCANCODE_SPACE];Story_Enter(s,STORY_NORMAL);
        }
        return;
    }
    if(s->mode==STORY_NORMAL && s->helper && p->hp>0 && c->active) {
        s->dogAttackTimer+=dt;
        if(s->dogSwoop>0) {
            s->dogSwoop=fmaxf(0,s->dogSwoop-dt);
            Story_MoveDog(s,c->x,c->y,300,dt);
            if(!s->dogHit && Coffin_Overlap(Story_DogBody(s),Coffin_Body(c)) && Coffin_Targetable(c)) {
                Coffin_Damage(c,DOG_HELP_DAMAGE);s->dogHit=true;
            }
        } else {
            // Wander around the arena independently of the player.
            float angle=floorf(s->clock/2.5f)*1.7f;
            Story_MoveDog(s,GAME_WIDTH/2+cosf(angle)*DOG_ROAM_RADIUS,
                GAME_HEIGHT/2+sinf(angle)*DOG_ROAM_RADIUS,DOG_ROAM_SPEED,dt);
            if(s->dogAttackTimer>=DOG_HELP_INTERVAL && Coffin_Targetable(c)) {
                s->dogAttackTimer=0;s->dogSwoop=DOG_SWOOP_TIME;s->dogHit=false;
            }
        }
    }
    if(s->mode==STORY_NORMAL && c->active && c->state==FC_DEAD) {
        s->helper=true;stage->returningHome=true;stage->completed=false;p->walkOnly=true;
        Story_Enter(s,STORY_RETURN);
    }
    if(s->mode==STORY_RETURN) {
        p->walkOnly=true;p->isSprinting=false;
        if(s->lastRoom!=stage->index) { s->dogX=p->x-10;s->dogY=p->y+8; }
        float distance=hypotf(p->x+8-s->dogX,p->y+8-s->dogY);
        if(distance>20) Story_MoveDog(s,p->x+8,p->y+8,p->speed,dt);
        if(stage->index==STAGE_ENTRANCE_ROOM) {
            p->isAttacking=p->isCharging=p->isShooting=false;
            Story_Enter(s,STORY_EXIT_WALK);
        }
    } else if(s->mode==STORY_EXIT_WALK) {
        float dx=ENDING_WALK_TARGET_X-(p->x+8),dy=ENDING_WALK_TARGET_Y-(p->y+8);
        float distance=hypotf(dx,dy),step=fminf(distance,p->speed*dt);
        p->isMoving=distance>0.01f;
        if(distance>0) {
            p->x+=dx/distance*step;p->y+=dy/distance*step;
            p->direction=Enemy_Face(dx,dy);p->facingLeft=dx<0;
        }
        p->currentFrame=(int)(s->timer/0.12f)%4;
        if(hypotf(p->x+8-s->dogX,p->y+8-s->dogY)>24)
            Story_MoveDog(s,p->x+8,p->y+8,p->speed,dt);
        if(distance<=step+0.01f) {
            p->x=ENDING_WALK_TARGET_X-8;p->y=ENDING_WALK_TARGET_Y-8;p->isMoving=false;
            Story_Enter(s,STORY_DEATH);
        }
    } else if(s->mode==STORY_DEATH) {
        if(s->timer>=ENDING_DEATH_TIME)
            Story_Enter(s,STORY_DOG_APPROACH);
    } else if(s->mode==STORY_DOG_APPROACH) {
        if(Story_MoveDog(s,p->x+18,p->y+12,p->speed,dt)) Story_Enter(s,STORY_FADE);
    } else if(s->mode==STORY_FADE) {
        if(s->timer>=ENDING_FADE_TIME) { Story_Enter(s,STORY_SLIDES);s->endingRestart=true; }
    } else if(s->mode==STORY_SLIDES) {
        if(s->timer>=ENDING_SLIDES_TIME) {
            Story_Enter(s,STORY_EPILOGUE);s->knightX=140;s->knightY=206;s->interaction=0;s->idleTime=0;
            s->spaceDown=keys[SDL_SCANCODE_SPACE];
        }
    } else if(s->mode==STORY_EPILOGUE) {
        s->interactTimer=fmaxf(0,s->interactTimer-dt);
        int dx=(keys[SDL_SCANCODE_D]||keys[SDL_SCANCODE_RIGHT])-(keys[SDL_SCANCODE_A]||keys[SDL_SCANCODE_LEFT]);
        int dy=(keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_DOWN])-(keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_UP]);
        s->knightMoving=(dx||dy) && s->interactTimer<=0;
        if(s->knightMoving) {
            if(dx) s->knightFacingLeft=dx<0;
            float scale=dx&&dy?0.70710678f:1;
            s->knightX=fmaxf(8,fminf(GAME_WIDTH-24,s->knightX+dx*p->speed*scale*dt));
            s->knightY=fmaxf(184,fminf(GAME_HEIGHT-20,s->knightY+dy*p->speed*scale*dt));
            s->knightAnim+=dt;s->idleTime=0;
        } else s->idleTime+=dt;
        float tx=s->interaction==0?EPILOGUE_DOG_X:EPILOGUE_GRAVE_X;
        float ty=s->interaction==0?EPILOGUE_DOG_Y:EPILOGUE_GRAVE_Y;
        if(press && s->interaction<2 && s->interactTimer<=0 &&
            hypotf(s->knightX+8-tx,s->knightY+8-ty)<28) {
            ++s->interaction;s->interactTimer=0.8f;s->idleTime=0;
        }
        if(s->interaction==2 && s->idleTime>EPILOGUE_IDLE_TIME && s->interactTimer<=0) Story_Enter(s,STORY_SKY);
    } else if(s->mode==STORY_SKY && s->timer>=EPILOGUE_PAN_TIME) {
        Story_Enter(s,STORY_WHITE);s->endingRestart=true;
    }
    s->lastRoom=stage->index;
}
static inline bool StoryArt_Init(StoryArt *art,SDL_Renderer *r) {
    for(int i=0;i<DUNGEON_ENTRANCE_FRAMES_COUNT;++i) {
        SDL_Surface *surface=SDL_CreateSurface(320,240,SDL_PIXELFORMAT_RGBA32);
        if(!surface) return false;
        for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
            uint16_t c=dungeon_entrance_frames[i][y*320+x];
            Uint8 *pixel=(Uint8*)surface->pixels+y*surface->pitch+x*4;
            pixel[0]=((c>>11)&31)*255/31;pixel[1]=((c>>5)&63)*255/63;pixel[2]=(c&31)*255/31;pixel[3]=255;
        }
        art->frames[i]=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);
        if(!art->frames[i]) return false;
        SDL_SetTextureScaleMode(art->frames[i],SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(art->frames[i],SDL_BLENDMODE_BLEND);
    }
    return true;
}
static inline void StoryArt_Close(StoryArt *a) { for(int i=0;i<10;++i) SDL_DestroyTexture(a->frames[i]); }
static inline void Story_Background(SDL_Renderer *r,StoryArt *a,int frame,float alpha,float y) {
    SDL_SetTextureAlphaMod(a->frames[frame],(Uint8)(255*fmaxf(0,fminf(1,alpha))));
    SDL_FRect dst={0,y,320,240};SDL_RenderTexture(r,a->frames[frame],NULL,&dst);
}
static inline void Story_OverlayColor(SDL_Renderer *r,Uint8 red,Uint8 green,Uint8 blue,float alpha) {
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r,red,green,blue,(Uint8)(255*fmaxf(0,fminf(1,alpha))));
    SDL_FRect full={0,0,320,240};SDL_RenderFillRect(r,&full);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
}
static inline void Story_DrawCharacter(SDL_Renderer *r,float x,float y,const uint16_t *pixels,bool left) {
    uint16_t mirrored[16*16];
    if(left) {
        for(int py=0;py<16;++py) for(int px=0;px<16;++px) mirrored[py*16+px]=pixels[py*16+15-px];
        pixels=mirrored;
    }
    King_DrawPixels(r,x,y,pixels,16,16,0,false);
}
static inline void Story_Draw(Story *s,StoryArt *art,SDL_Renderer *r,const Player *p) {
    if(s->mode>=STORY_DEATH) {
        if(s->mode==STORY_SLIDES) {
            float blend=fminf(5,s->timer/ENDING_SLIDES_TIME*5);int frame=(int)blend;
            Story_Background(r,art,2+frame,1,0);
            if(frame<5) Story_Background(r,art,3+frame,blend-frame,0);
            return;
        }
        if(s->mode>=STORY_EPILOGUE) {
            float pan=s->mode==STORY_SKY?fminf(1,s->timer/EPILOGUE_PAN_TIME):s->mode==STORY_WHITE?1:0;
            pan=pan*pan*(3-2*pan);
            SDL_SetRenderDrawColor(r,20,47,102,255);SDL_FRect sky={0,0,320,240};SDL_RenderFillRect(r,&sky);
            Story_Background(r,art,7+s->interaction,1,pan*190);
            if(s->mode==STORY_EPILOGUE || s->mode==STORY_SKY) {
                const uint16_t *knight=s->interactTimer>0?knight_interact[0]:s->knightMoving?knight_walk[(int)(s->knightAnim/0.12f)%KNIGHT_WALK_COUNT]:knight_idle[0];
                Story_DrawCharacter(r,s->knightX+8,s->knightY+8+pan*190,knight,s->knightFacingLeft);
                SDL_SetRenderDrawColor(r,255,255,255,255);
                if(s->mode==STORY_EPILOGUE) SDL_RenderDebugText(r,10,224,s->interaction==0?"Space: Talk to Dog":s->interaction==1?"Space: Visit grave":"Rest a moment...");
            }
            if(s->mode==STORY_WHITE) Story_OverlayColor(r,255,255,255,s->timer/EPILOGUE_WHITE_TIME);
            return;
        }
        Story_Background(r,art,0,1,0);
        int frame=s->mode==STORY_DEATH?Story_Frame(s->timer/ENDING_DEATH_TIME*Story_Duration(undying_player_dead_true_end_duration_ms,UNDYING_PLAYER_DEAD_TRUE_END_COUNT),undying_player_dead_true_end_duration_ms,UNDYING_PLAYER_DEAD_TRUE_END_COUNT):UNDYING_PLAYER_DEAD_TRUE_END_COUNT-1;
        King_DrawPixels(r,p->x+8,p->y+8,undying_player_dead_true_end[frame],16,16,0,false);
    }
    if(s->helper || s->mode==STORY_RESCUE || s->mode>=STORY_DEATH) {
        const uint16_t *dog=s->dogSwoop>0 || (s->mode==STORY_RESCUE && s->rescueStep==2)?
            dog_attack[(int)(s->clock/0.1f)%DOG_ATTACK_COUNT]:
            s->dogMoving?dog_walk[(int)(s->clock/0.12f)%DOG_WALK_COUNT]:dog_idle_stand[0];
        if(s->mode!=STORY_RESCUE || s->rescueStep>=2)
            Story_DrawCharacter(r,s->dogX,s->dogY,dog,s->dogFacingLeft);
    }
    if(s->mode==STORY_RESCUE) {
        if(s->rescueStep<3) for(int i=0;i<3;++i)
            King_DrawPixels(r,p->x+8+(i-1)*10,p->y+8,blood_spellblade_frames[0],BLOOD_SPELLBLADE_WIDTH,BLOOD_SPELLBLADE_HEIGHT,(i-1)*0.7f,false);
        SDL_SetRenderDrawColor(r,0,0,0,255);
        SDL_FRect bars[2]={{0,0,320,22},{0,218,320,22}};
        SDL_RenderFillRects(r,bars,2);
    }
    if(s->mode==STORY_RETURN) {
        Story_OverlayColor(r,10,12,30,0.07f+0.035f*sinf(s->clock*2.5f));
        SDL_SetRenderDrawColor(r,255,240,220,255);SDL_RenderDebugText(r,116,32,"Return Home");
    }
    if(s->mode==STORY_FADE) Story_OverlayColor(r,0,0,0,s->timer/ENDING_FADE_TIME);
}
#endif
