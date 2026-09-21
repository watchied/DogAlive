#include <assert.h>
#include "src/ui/flesh_coffin_draw.h"
int main(void) {
    assert(SDL_Init(0));
    SDL_Surface *surface=SDL_CreateSurface(GAME_WIDTH*4,GAME_HEIGHT*3,SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    SDL_Renderer *renderer=SDL_CreateSoftwareRenderer(surface);assert(renderer);
    const CoffinState states[]={FC_DORMANT,FC_INTRO,FC_GROUND,FC_WALL,FC_FALL,FC_TRANSITION,FC_SLASH,FC_TELEPORT,FC_CHARGE,FC_CHARGED_SLASH,FC_STUN,FC_DEATH};
    const char *names[]={"Dormant","Opening","Ground swords","Wall split","Slam","Phase transition","Front slash","Teleport","Charge","Charged slash","Stunned","Death"};
    const float times[]={0,0.6f,0.3f,0.4f,0.2f,6.0f,0.5f,0.35f,2.0f,0.8f,0.3f,0.8f};
    for(int i=0;i<12;++i) {
        SDL_Rect viewport={(i%4)*GAME_WIDTH,(i/4)*GAME_HEIGHT,GAME_WIDTH,GAME_HEIGHT};
        assert(SDL_SetRenderViewport(renderer,&viewport));
        SDL_SetRenderDrawColor(renderer,24,25,30,255);
        SDL_FRect bg={0,0,GAME_WIDTH,GAME_HEIGHT};SDL_RenderFillRect(renderer,&bg);
        FleshCoffin c;Coffin_Init(&c);c.x=160;c.y=125;c.phase=i>=5?2:1;c.state=states[i];c.timer=times[i];c.direction=PLAYER_RIGHT;
        if(i==2 || i==3) for(int j=0;j<3;++j)c.swords[j]=(CoffinSword){.x=65+j*90,.y=65,.active=true};
        Coffin_Draw(renderer,&c);
        SDL_SetRenderDrawColor(renderer,255,255,255,255);SDL_RenderDebugText(renderer,12,12,names[i]);
    }
    SDL_RenderPresent(renderer);
    assert(SDL_SaveBMP(surface,"bin/flesh_coffin_preview.bmp"));
    SDL_DestroyRenderer(renderer);SDL_DestroySurface(surface);SDL_Quit();return 0;
}
