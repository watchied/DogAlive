#include <assert.h>
#include <stdio.h>
#include "src/core/room_interactions.h"
#include "src/ui/flesh_coffin_draw.h"
#include "src/ui/room_objects_draw.h"
#include "src/effects/parry_effect.h"
static StageProgress stage;
int main(void) {
    Player p;Player_Init(&p);
    EnemyGroup g;Projectile arrows[MAX_PROJECTILES]={0};SlimeShot shots[SLIME_SHOT_CAPACITY]={0};RunningEffect effect={0};
    assert(Stage_Next(0)==1 && Stage_Next(1)==2 && Stage_Next(2)==STAGE_BOSS_ROOM);
    assert(Stage_Next(STAGE_COFFIN_ROOM)==STAGE_COUNT);
    for(int i=0;i<STAGE_COUNT;++i) if(i!=STAGE_REWARD_ROOM && Stage_Next(i)<STAGE_COUNT)
        assert(Stage_Previous(Stage_Next(i))==i);
    Stage_Load(1,&p,&g,arrows,shots,&effect);
    assert(g.ghoulCount==2 && g.slimeCount==1 && g.ghouls[0].goblin && g.slimes[0].ordinarySlime);
    assert(Ghoul_GetSprite(&g.ghouls[0])==goblin_walk[0]);
    assert(FleshSlime_GetSprite(&g.slimes[0])==slime_slime_walk[0]);
    g.ghouls[0].direction=PLAYER_RIGHT;
    assert(Ghoul_AttackBox(&g.ghouls[0]).w==GOBLIN_MELEE_REACH);
    Ghoul_Init(&g.ghouls[0],100,100);g.ghouls[0].direction=PLAYER_RIGHT;
    assert(Ghoul_AttackBox(&g.ghouls[0]).w==MELEE_REACH*1.25f);
    Stage_Load(2,&p,&g,arrows,shots,&effect);assert(g.ghoulCount==3 && g.slimeCount==2);
    uint32_t seed=123;int plain=0,total=0;
    for(int run=0;run<100;++run) {
        Floor_GenerateDirt(stage.floors[1],&seed);
        for(int y=0;y<FLOOR_ROWS;++y) for(int x=0;x<FLOOR_COLUMNS;++x) {
            int t=stage.floors[1][y][x];
            if(!x || !y || x==FLOOR_COLUMNS-1 || y==FLOOR_ROWS-1)
                assert(t>=MAP_OBJECT_FLOOR_COUNT+MAP_OBJECT_DIRT_COUNT && t<FLOOR_ATLAS_COUNT);
            else { assert(t>=MAP_OBJECT_FLOOR_COUNT && t<MAP_OBJECT_FLOOR_COUNT+MAP_OBJECT_DIRT_COUNT);++total;plain+=t==MAP_OBJECT_FLOOR_COUNT; }
        }
    }
    assert(plain>total*0.58f && plain<total*0.62f);
    uint8_t previous[FLOOR_ROWS][FLOOR_COLUMNS];
    memcpy(previous,stage.floors[1],sizeof(previous));
    Floor_GenerateDirt(stage.floors[1],&seed);
    for(int y=0;y<FLOOR_ROWS;++y) for(int x=0;x<FLOOR_COLUMNS;++x)
        if(!x || !y || x==FLOOR_COLUMNS-1 || y==FLOOR_ROWS-1)
            assert(previous[y][x]==stage.floors[1][y][x]);
    Slime normal;Slime_Init(&normal,200,100);Player_Init(&p);p.x=20;p.y=100;
    memset(shots,0,sizeof(shots));
    for(int i=0;i<400 && !shots[0].active;++i) FleshSlime_Update(&normal,&p,shots,0.01f);
    assert(shots[0].active && shots[0].ordinarySlime);
    memset(shots,0,sizeof(shots));
    EnemyGroup_InitCounts(&g,0,1,0);p.x=20;p.y=100;
    for(int i=0;i<100;++i) EnemyGroup_Update(&g,&p,shots,0.01f);
    int dust=0;for(int i=0;i<COFFIN_DUST_CAPACITY;++i)dust+=g.slimeDust.dust[i].life>0;
    assert(dust>0);
    EnemyGroup_InitCounts(&g,0,1,0);g.slimes[0].ordinarySlime=true;
    for(int i=0;i<100;++i) EnemyGroup_Update(&g,&p,shots,0.01f);
    for(int i=0;i<COFFIN_DUST_CAPACITY;++i)assert(g.slimeDust.dust[i].life==0);
    Stage_Load(STAGE_COFFIN_ROOM,&p,&g,arrows,shots,&effect);
    assert(g.coffin.active && roomDefinitions[STAGE_COFFIN_ROOM].objectCount==10);
    SDL_FRect block=Room_MapObjectBody(&roomDefinitions[STAGE_COFFIN_ROOM].objects[0]);
    p.x=block.x+4;p.y=block.y+block.h+10;float oldX=p.x,oldY=p.y;p.y=block.y-20;
    Room_BlockMovement(&roomDefinitions[STAGE_COFFIN_ROOM],&p,oldX,oldY);
    assert(Player_Body(&p).y>=block.y+block.h);
    p.chargedAttack=true;Player_ShowParry(&p,100,100);p.chargedAttack=false;assert(p.chargedParryEffect);
    assert(SDL_Init(0));
    SDL_Surface *surface=SDL_CreateSurface(GAME_WIDTH*2,GAME_HEIGHT,SDL_PIXELFORMAT_RGBA32);
    SDL_Renderer *r=SDL_CreateSoftwareRenderer(surface);assert(r);
    SDL_Texture *atlas=Floor_CreateAtlas(r);assert(atlas);
    for(int view=0;view<2;++view) {
        SDL_Rect viewport={view*GAME_WIDTH,0,GAME_WIDTH,GAME_HEIGHT};SDL_SetRenderViewport(r,&viewport);
        stage.index=view?STAGE_COFFIN_ROOM:1;
        if(view) Floor_Generate(stage.floors[stage.index],&seed);
        Stage_Load(stage.index,&p,&g,arrows,shots,&effect);
        Stage_Draw(r,atlas,&stage,&g);Room_Draw(r,&stage,&p);
        if(view) Coffin_Draw(r,&g.coffin);
        else {
            for(int i=0;i<g.ghoulCount;++i) King_DrawPixels(r,g.ghouls[i].x+8,g.ghouls[i].y+8,Ghoul_GetSprite(&g.ghouls[i]),16,16,0,false);
            for(int i=0;i<g.slimeCount;++i) King_DrawPixels(r,g.slimes[i].x+8,g.slimes[i].y+8,FleshSlime_GetSprite(&g.slimes[i]),16,16,0,false);
        }
    }
    SDL_RenderPresent(r);assert(SDL_SaveBMP(surface,"bin/dungeon_preview.bmp"));
    SDL_DestroyTexture(atlas);SDL_DestroyRenderer(r);SDL_DestroySurface(surface);SDL_Quit();
    puts("New enemies, room route, dirt distribution, trails, coffin blocks and charged spark passed");
}
