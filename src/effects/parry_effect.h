#ifndef PARRY_EFFECT_H
#define PARRY_EFFECT_H
#include "src/player/player.h"
#include <math.h>
#include "assets/sprites/effects/hit-spark.h"

static inline void ParryEffect_Draw(SDL_Renderer *r,const Player *p)
{
    if(p->parryEffectTimer<=0) return;
    float progress=1-p->parryEffectTimer/PLAYER_PARRY_EFFECT_TIME;
    float cx=p->parryEffectX,cy=p->parryEffectY;
    // Play once at the fixed midpoint recorded when the parry succeeds.
    float elapsed=PLAYER_PARRY_EFFECT_TIME-p->parryEffectTimer;
    int frame=0;
    while(frame<HIT_SPARK_FRAMES_COUNT) {
        float duration=hit_spark_frames_duration_ms[frame]/1000.0f;
        if(elapsed<duration) break;
        elapsed-=duration;
        ++frame;
    }
    if(frame<HIT_SPARK_FRAMES_COUNT) {
        float scale=p->chargedParryEffect?1.5f:1.0f;
        const uint16_t *pixels=hit_spark_frames[frame];
        for(int py=0;py<HIT_SPARK_HEIGHT;++py)
            for(int px=0;px<HIT_SPARK_WIDTH;++px) {
                uint16_t color=pixels[py*HIT_SPARK_WIDTH+px];
                if(color==0x07E0) continue;
                SDL_SetRenderDrawColor(r,((color>>11)&31)*255/31,
                    ((color>>5)&63)*255/63,(color&31)*255/31,255);
                SDL_FRect pixel={cx+(px-HIT_SPARK_WIDTH/2.0f)*scale,
                    cy+(py-HIT_SPARK_HEIGHT/2.0f)*scale,scale,scale};
                SDL_RenderFillRect(r,&pixel);
            }
    }
    float x=fmaxf(2,fminf(GAME_WIDTH-44,cx-20));
    float y=fmaxf(2,fminf(GAME_HEIGHT-12,cy-23-progress*10));
    SDL_SetRenderDrawColor(r,20,15,5,255);SDL_RenderDebugText(r,x+1,y+1,"PARRY");
    SDL_SetRenderDrawColor(r,255,235,110,255);SDL_RenderDebugText(r,x,y,"PARRY");
}
#endif
