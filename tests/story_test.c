#include <assert.h>
#include <stdio.h>
#include "src/story/story.h"
#include "src/core/game_core.h"
static StageProgress stage;
static EnemyGroup g;
static Story s;
static Player p;
static bool keys[SDL_SCANCODE_COUNT];
static void advance(float seconds) {
    for(float t=0;t<seconds;t+=0.01f) Story_Update(&s,&stage,&p,&g,keys,0.01f);
}
int main(void) {
    Player_Init(&p);stage.index=STAGE_COFFIN_ROOM;p.x=120;p.y=120;
    Coffin_Init(&g.coffin);g.coffin.phase=2;Coffin_Enter(&g.coffin,FC_IDLE);
    Coffin_Damage(&g.coffin,COFFIN_HP*2);
    assert(g.coffin.hp==COFFIN_HP/2 && g.coffin.rescueRequested);
    Coffin_Damage(&g.coffin,100);assert(g.coffin.hp==COFFIN_HP/2);
    advance(0.01f);assert(s.mode==STORY_RESCUE && Story_Locked(&s));
    int hp=p.hp;
    advance(RESCUE_RESTRAIN_TIME+RESCUE_CHARGE_TIME+(RESCUE_PARRY_TIME+RESCUE_FINISH_TIME)/RESCUE_PARRY_TIME_SCALE+0.2f);
    assert(s.mode==STORY_NORMAL && s.helper && g.coffin.rescueDone && !g.coffin.rescueRequested && p.hp==hp);
    g.coffin.x=180;g.coffin.y=120;Coffin_Enter(&g.coffin,FC_IDLE);
    s.dogX=150;s.dogY=120;s.dogAttackTimer=0;
    int bossHP=g.coffin.hp;
    float restX=s.dogX,restY=s.dogY;
    advance(DOG_HELP_INTERVAL-0.2f);assert(g.coffin.hp==bossHP && (s.dogX!=restX || s.dogY!=restY));
    advance(1.0f);assert(g.coffin.hp==bossHP-DOG_HELP_DAMAGE);
    Coffin_Damage(&g.coffin,COFFIN_HP);assert(g.coffin.state==FC_DEATH);
    g.coffin.state=FC_DEAD;advance(0.01f);
    assert(s.mode==STORY_RETURN && p.walkOnly && stage.returningHome);
    p.isSprinting=true;p.sprintButton=8;keys[SDL_SCANCODE_D]=true;
    float x=p.x;Game_Update(&p,keys,0.1f);
    assert(!p.isSprinting && fabsf(p.x-x-p.speed*0.1f)<0.01f);
    keys[SDL_SCANCODE_D]=false;
    assert(Stage_Next(STAGE_BOSS_ROOM)==STAGE_ENTRANCE_ROOM);
    assert(Stage_Next(STAGE_ENTRANCE_ROOM)==5 && Stage_Previous(5)==STAGE_ENTRANCE_ROOM);
    for(int i=0;i<320;i+=8) {
        p.x=(float)i;p.y=0;Entrance_Clamp(&p);
        SDL_FRect body=Player_Body(&p);
        assert(body.y>=((body.x>=132 && body.x+body.w<=176)?164:192));
    }
    stage.index=STAGE_ENTRANCE_ROOM;p.x=148;p.y=190;
    advance(0.01f);assert(s.mode==STORY_EXIT_WALK && Story_Locked(&s));
    advance(2);assert(s.mode==STORY_DEATH);
    assert(p.x+8==ENDING_WALK_TARGET_X && p.y+8==ENDING_WALK_TARGET_Y);
    s.timer=0;advance(ENDING_DEATH_TIME-0.1f);assert(s.mode==STORY_DEATH);
    advance(0.2f);
    advance(4);assert(s.mode==STORY_SLIDES);
    s.timer=ENDING_SLIDES_TIME-0.1f;advance(0.12f);assert(s.mode==STORY_EPILOGUE);
    s.knightX=EPILOGUE_DOG_X-8;s.knightY=EPILOGUE_DOG_Y-8;
    keys[SDL_SCANCODE_SPACE]=true;advance(0.01f);assert(s.interaction==1 && s.interactTimer>0);
    advance(1);assert(s.interaction==1); // Holding interact never consumes both steps.
    keys[SDL_SCANCODE_SPACE]=false;advance(0.01f);
    s.knightX=EPILOGUE_GRAVE_X-8;s.knightY=EPILOGUE_GRAVE_Y-8;
    keys[SDL_SCANCODE_SPACE]=true;advance(0.01f);assert(s.interaction==2);
    keys[SDL_SCANCODE_SPACE]=false;advance(EPILOGUE_IDLE_TIME+0.1f);assert(s.mode==STORY_SKY);
    advance(EPILOGUE_PAN_TIME+0.1f);assert(s.mode==STORY_WHITE && s.endingRestart);
    s.dogX=100;s.dogY=100;Story_MoveDog(&s,50,100,40,0.1f);assert(s.dogFacingLeft);
    Story_MoveDog(&s,150,100,40,0.1f);assert(!s.dogFacingLeft);
    s.mode=STORY_EPILOGUE;s.interaction=0;s.knightX=150;s.knightY=200;
    keys[SDL_SCANCODE_A]=true;advance(0.1f);assert(s.knightFacingLeft);
    keys[SDL_SCANCODE_A]=false;keys[SDL_SCANCODE_D]=true;advance(0.1f);assert(!s.knightFacingLeft);
    keys[SDL_SCANCODE_D]=false;
    s.mode=STORY_RESCUE;s.rescueStep=2;s.timer=0;g.coffin.flash=1;
    Story_Update(&s,&stage,&p,&g,keys,0.1f);
    assert(fabsf(s.timer-0.1f*RESCUE_PARRY_TIME_SCALE)<0.001f && g.coffin.flash==0);
    s.mode=STORY_NORMAL;s.helper=true;s.dogX=100;s.dogY=100;
    stage.index=STAGE_COFFIN_ROOM;g.coffin.active=false;p.hp=p.maxHP;p.x=92;p.y=92;
    Story_DogCollision(&s,&stage,&p,&g);
    assert(!Coffin_Overlap(Story_DogBody(&s),Player_Body(&p)));
    assert(SDL_Init(0));
    SDL_Surface *surface=SDL_CreateSurface(960,240,SDL_PIXELFORMAT_RGBA32);
    SDL_Renderer *r=SDL_CreateSoftwareRenderer(surface);StoryArt art={0};assert(StoryArt_Init(&art,r));
    for(int i=0;i<3;++i) {
        SDL_Rect vp={320*i,0,320,240};SDL_SetRenderViewport(r,&vp);
        if(i==0) Story_Background(r,&art,1,1,0);
        else { s.mode=i==1?STORY_SLIDES:STORY_EPILOGUE;s.timer=12;s.interaction=2;Story_Draw(&s,&art,r,&p); }
    }
    SDL_RenderPresent(r);assert(SDL_SaveBMP(surface,"bin/story_preview.bmp"));
    StoryArt_Close(&art);SDL_DestroyRenderer(r);SDL_DestroySurface(surface);SDL_Quit();
    puts("Rescue threshold, lock, parry, dog damage, return, boundaries, ED and epilogue passed");
}
