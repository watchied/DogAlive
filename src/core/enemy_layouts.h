#ifndef ENEMY_LAYOUTS_H
#define ENEMY_LAYOUTS_H
#include "src/combat/enemy_group.h"
// Included by stages.h after STAGE_COUNT is defined.
// Room index = displayed room number minus 1 (room 10 uses keyboard 0).
// Each row is {type, centerX, centerY}. Add/remove rows to change enemy counts.
// Coordinates use the visual center for every type. Map size: 320 x 240.
// Goblin + Ghoul share 10 slots; Slime + Flesh Slime share 10 slots;
// Eye has 10 slots. At most one of each boss per room.
typedef enum {
    SPAWN_NONE, SPAWN_GOBLIN, SPAWN_GHOUL, SPAWN_SLIME,
    SPAWN_FLESH_SLIME, SPAWN_EYE, SPAWN_SLIME_KING, SPAWN_FLESH_COFFIN
} EnemySpawnType;
typedef struct { EnemySpawnType type; float x,y; } EnemySpawn;
#define ROOM_ENEMY_CAPACITY (ENEMY_TYPE_CAPACITY*3+2)
static const EnemySpawn enemyLayouts[STAGE_COUNT][ROOM_ENEMY_CAPACITY] = {
    [0]={{SPAWN_NONE,0,0}}, // Starting checkpoint.
    [1]={ // Room 2
        {SPAWN_GOBLIN,180,44},
        {SPAWN_GOBLIN,180,180},
        {SPAWN_SLIME,236,40},
    },
    [2]={ // Room 3
        {SPAWN_GOBLIN,180,44},
        {SPAWN_GOBLIN,120,44},
        {SPAWN_GOBLIN,120,150},
        {SPAWN_SLIME,264,40},
        {SPAWN_SLIME,262,150},
    },
    [3]={{SPAWN_SLIME_KING,244,124}},
    [4]={{SPAWN_NONE,0,0}}, // Reward room.
    [5]={ // Room 6
        {SPAWN_GHOUL,180,44},
        {SPAWN_GHOUL,208,44},
        {SPAWN_FLESH_SLIME,236,44},
    },
    [6]={ // Room 7
        {SPAWN_GHOUL,180,44},
        {SPAWN_GHOUL,208,44},
        {SPAWN_GHOUL,236,44},
        {SPAWN_FLESH_SLIME,264,44},
        {SPAWN_EYE,292,44},
    },
    [7]={ // Room 8
        {SPAWN_GHOUL,180,44},
        {SPAWN_GHOUL,208,44},
        {SPAWN_GHOUL,236,44},
        {SPAWN_FLESH_SLIME,264,44},
        {SPAWN_FLESH_SLIME,292,44},
        {SPAWN_EYE,180,74},
        {SPAWN_EYE,208,74},
    },
    [8]={ // Room 9
        {SPAWN_GHOUL,180,44},
        {SPAWN_GHOUL,208,44},
        {SPAWN_GHOUL,236,44},
        {SPAWN_GHOUL,264,44},
        {SPAWN_FLESH_SLIME,292,44},
        {SPAWN_FLESH_SLIME,180,74},
        {SPAWN_FLESH_SLIME,208,74},
        {SPAWN_EYE,236,74},
        {SPAWN_EYE,264,74},
    },
    [9]={{SPAWN_FLESH_COFFIN,278,72}},
};
static inline void EnemyLayout_Load(const EnemySpawn *layout,EnemyGroup *g) {
    EnemyGroup_InitCounts(g,0,0,0);
    for(int i=0;i<ROOM_ENEMY_CAPACITY;++i) {
        EnemySpawn s=layout[i];
        float x=fmaxf(4,fminf(GAME_WIDTH-20,s.x-ACTOR_HALF_SIZE));
        float y=fmaxf(4,fminf(GAME_HEIGHT-20,s.y-ACTOR_HALF_SIZE));
        switch(s.type) {
        case SPAWN_GOBLIN: case SPAWN_GHOUL:
            if(g->ghoulCount>=ENEMY_TYPE_CAPACITY) break;
            if(s.type==SPAWN_GOBLIN) Goblin_Init(&g->ghouls[g->ghoulCount++],x,y);
            else Ghoul_Init(&g->ghouls[g->ghoulCount++],x,y);
            break;
        case SPAWN_SLIME: case SPAWN_FLESH_SLIME:
            if(g->slimeCount>=ENEMY_TYPE_CAPACITY) break;
            if(s.type==SPAWN_SLIME) Slime_Init(&g->slimes[g->slimeCount++],x,y);
            else FleshSlime_Init(&g->slimes[g->slimeCount++],x,y);
            break;
        case SPAWN_EYE:
            if(g->eyeCount<ENEMY_TYPE_CAPACITY) EyeParasite_Init(&g->eyes[g->eyeCount++],x,y);
            break;
        case SPAWN_SLIME_KING:
            if(!g->king.active) {
                King_Init(&g->king,s.x-24,s.y-24);
                g->king.rng^=(uint32_t)SDL_GetPerformanceCounter();
            }
            break;
        case SPAWN_FLESH_COFFIN:
            if(!g->coffin.active) {
                Coffin_Init(&g->coffin);g->coffin.x=s.x;g->coffin.y=s.y;Coffin_Clamp(&g->coffin);
                g->coffin.rng^=(uint32_t)SDL_GetPerformanceCounter();
            }
            break;
        default: break;
        }
    }
}
#endif
