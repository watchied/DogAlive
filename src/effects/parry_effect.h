#ifndef PARRY_EFFECT_H
#define PARRY_EFFECT_H
#include "src/player/player.h"
#include <math.h>

static inline void ParryEffect_Draw(SDL_Renderer *r,const Player *p)
{
    if(p->parryEffectTimer<=0) return;
    float progress=1-p->parryEffectTimer/PLAYER_PARRY_EFFECT_TIME;
    float cx=p->parryEffectX,cy=p->parryEffectY;
    // Gold burst is distinct from the white damage flash. Fixed impact position.
    SDL_SetRenderDrawColor(r,255,220,80,255);
    if(progress<0.65f) for(int i=0;i<8;++i) {
        float a=i*0.78539816339f,near=3+progress*12,far=10+progress*22;
        SDL_RenderLine(r,cx+cosf(a)*near,cy+sinf(a)*near,cx+cosf(a)*far,cy+sinf(a)*far);
        SDL_FRect spark={cx+cosf(a)*far-1,cy+sinf(a)*far-1,2,2};SDL_RenderFillRect(r,&spark);
    }
    if(progress<0.2f) {
        SDL_SetRenderDrawColor(r,255,255,255,255);
        SDL_FRect flash={cx-3,cy-3,6,6};SDL_RenderFillRect(r,&flash);
    }
    float x=fmaxf(2,fminf(GAME_WIDTH-44,cx-20));
    float y=fmaxf(2,fminf(GAME_HEIGHT-12,cy-23-progress*10));
    SDL_SetRenderDrawColor(r,20,15,5,255);SDL_RenderDebugText(r,x+1,y+1,"PARRY");
    SDL_SetRenderDrawColor(r,255,235,110,255);SDL_RenderDebugText(r,x,y,"PARRY");
}
#endif
