#include <assert.h>
#include <stdio.h>
#include "src/core/stages.h"
int main(void) {
    EnemyGroup g;
    EnemySpawn custom[ROOM_ENEMY_CAPACITY]={
        {SPAWN_GOBLIN,80,60},{SPAWN_GHOUL,120,90},
        {SPAWN_SLIME,160,110},{SPAWN_FLESH_SLIME,220,150},{SPAWN_EYE,260,180}
    };
    EnemyLayout_Load(custom,&g);
    assert(g.ghoulCount==2 && g.slimeCount==2 && g.eyeCount==1);
    assert(g.ghouls[0].goblin && !g.ghouls[1].goblin);
    assert(g.ghouls[0].x==72 && g.ghouls[0].y==52);
    assert(g.slimes[0].ordinarySlime && !g.slimes[1].ordinarySlime);
    assert(g.slimes[1].x==212 && g.slimes[1].y==142);
    assert(g.eyes[0].x==252 && g.eyes[0].y==172);
    for(int i=0;i<ROOM_ENEMY_CAPACITY;++i) custom[i]=(EnemySpawn){SPAWN_GOBLIN,80,80};
    EnemyLayout_Load(custom,&g);assert(g.ghoulCount==ENEMY_TYPE_CAPACITY);
    EnemyLayout_Load(enemyLayouts[STAGE_BOSS_ROOM],&g);assert(g.king.active && g.king.x==220 && g.king.y==100);
    EnemyLayout_Load(enemyLayouts[STAGE_COFFIN_ROOM],&g);assert(g.coffin.active && g.coffin.x==278 && g.coffin.y==72);
    puts("Editable spawns: mixed variants, coordinates, capacity and bosses passed");
}
