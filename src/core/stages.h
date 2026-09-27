#ifndef STAGES_H
#define STAGES_H
#include "src/combat/enemy_group.h"
#include "src/effects/running_effect.h"
#include "src/core/floor_tiles.h"
#include "src/core/room_objects.h"

#define STAGE_COUNT 11
#define STAGE_ENTRANCE_ROOM 10
#define STAGE_COFFIN_ROOM 9
#define STAGE_FINAL_ROOM STAGE_COFFIN_ROOM
#define STAGE_START_ROOM 0
#define STAGE_BOSS_ROOM 3
#define STAGE_REWARD_ROOM 4
#define STAGE_WALL_SIZE 4.0f
typedef enum { STAGE_RIGHT, STAGE_LEFT, STAGE_TOP, STAGE_BOTTOM } StageSide;
typedef struct { StageSide nextSide; } StageDefinition;
// Exit directions only. Enemy types/counts/positions are in enemy_layouts.h.
static const StageDefinition stageDefinitions[STAGE_COUNT] = {
    {STAGE_RIGHT},{STAGE_RIGHT},{STAGE_RIGHT},{STAGE_RIGHT},{STAGE_RIGHT},
    {STAGE_TOP},{STAGE_RIGHT},{STAGE_BOTTOM},{STAGE_RIGHT},{STAGE_RIGHT},{STAGE_TOP}
};
#include "src/core/enemy_layouts.h"
static inline bool Stage_IsDirt(int index) { return index==1 || index==2 || index==STAGE_BOSS_ROOM || index==STAGE_REWARD_ROOM; }

#include "src/core/room_layouts.h"

typedef struct {
    int index;
    bool completed;
    bool returningHome;
    bool hasCheckpoint;
    int checkpointStage;
    float respawnX, respawnY;
    RoomObjects rooms[STAGE_COUNT];
    bool visited[STAGE_COUNT];
    EnemyGroup saved[STAGE_COUNT]; // Inactive rooms are frozen until revisited.
    uint8_t floors[STAGE_COUNT][FLOOR_ROWS][FLOOR_COLUMNS];
} StageProgress;

static inline int Stage_EnemiesAlive(const EnemyGroup *g)
{
    int count = 0;
    for (int i = 0; i < g->ghoulCount; ++i) count += g->ghouls[i].hp > 0;
    for (int i = 0; i < g->slimeCount; ++i) count += g->slimes[i].hp > 0;
    for (int i = 0; i < g->eyeCount; ++i) count += g->eyes[i].hp > 0;
    if (g->king.active && g->king.state != KING_NPC) ++count;
    if (g->coffin.active && g->coffin.state != FC_DEAD) ++count;
    return count;
}
static inline StageSide Stage_Opposite(StageSide side)
{
    return side == STAGE_RIGHT ? STAGE_LEFT : side == STAGE_LEFT ? STAGE_RIGHT :
        side == STAGE_TOP ? STAGE_BOTTOM : STAGE_TOP;
}
static inline StageSide Stage_BackSide(int index)
{
    if(index==STAGE_ENTRANCE_ROOM) return STAGE_LEFT;
    if(index==5) return STAGE_BOTTOM;
    if (index == STAGE_REWARD_ROOM) return STAGE_BOTTOM;
    if (index == STAGE_REWARD_ROOM + 1) return Stage_Opposite(stageDefinitions[STAGE_BOSS_ROOM].nextSide);
    return index > 0 ? Stage_Opposite(stageDefinitions[index - 1].nextSide) : STAGE_LEFT;
}
static inline int Stage_Next(int index)
{
    if(index==STAGE_BOSS_ROOM) return STAGE_ENTRANCE_ROOM;
    if(index==STAGE_ENTRANCE_ROOM) return 5;
    if(index==STAGE_FINAL_ROOM) return STAGE_COUNT;
    return index == STAGE_BOSS_ROOM ? STAGE_REWARD_ROOM + 1 : index + 1;
}
static inline int Stage_Previous(int index)
{
    if(index==STAGE_ENTRANCE_ROOM) return STAGE_BOSS_ROOM;
    if(index==5) return STAGE_ENTRANCE_ROOM;
    return index == STAGE_REWARD_ROOM ? STAGE_BOSS_ROOM : index - 1;
}
static inline SDL_FRect Stage_DoorBox(StageSide side, float position)
{
    if (side == STAGE_TOP || side == STAGE_BOTTOM)
        return (SDL_FRect){GAME_WIDTH * position - 20,
            side == STAGE_TOP ? STAGE_WALL_SIZE : GAME_HEIGHT - STAGE_WALL_SIZE - 8, 40, 8};
    return (SDL_FRect){side == STAGE_LEFT ? STAGE_WALL_SIZE : GAME_WIDTH - STAGE_WALL_SIZE - 8,
        GAME_HEIGHT * position - 20, 8, 40};
}
static inline SDL_FRect Stage_ExitBox(int index)
{
    if(index==STAGE_ENTRANCE_ROOM) return (SDL_FRect){136,164,36,20};
    bool sameSide = index > 0 && Stage_BackSide(index) == stageDefinitions[index].nextSide;
    return Stage_DoorBox(stageDefinitions[index].nextSide, sameSide ? 0.7f : 0.5f);
}
static inline SDL_FRect Stage_BackBox(int index)
{
    if(index==STAGE_ENTRANCE_ROOM) return (SDL_FRect){4,196,8,36};
    bool sameSide = Stage_BackSide(index) == stageDefinitions[index].nextSide;
    return Stage_DoorBox(Stage_BackSide(index), sameSide ? 0.3f : 0.5f);
}
static inline void Stage_PlaceAtDoor(Player *p, SDL_FRect door, StageSide side)
{
    p->x = door.x + door.w / 2 - ACTOR_HALF_SIZE;
    p->y = door.y + door.h / 2 - ACTOR_HALF_SIZE;
    if (side == STAGE_LEFT) { p->x = door.x + door.w + 8; p->direction = PLAYER_RIGHT; }
    if (side == STAGE_RIGHT) { p->x = door.x - ACTOR_SIZE - 8; p->direction = PLAYER_LEFT; }
    if (side == STAGE_TOP) { p->y = door.y + door.h + 8; p->direction = PLAYER_DOWN; }
    if (side == STAGE_BOTTOM) { p->y = door.y - ACTOR_SIZE - 8; p->direction = PLAYER_UP; }
    p->aimX = (p->direction == PLAYER_RIGHT) - (p->direction == PLAYER_LEFT);
    p->aimY = (p->direction == PLAYER_DOWN) - (p->direction == PLAYER_UP);
    p->facingLeft = p->direction == PLAYER_LEFT;
}
static inline void Stage_ClampPlayer(Player *p)
{
    p->x = fmaxf(STAGE_WALL_SIZE, fminf(GAME_WIDTH - STAGE_WALL_SIZE - ACTOR_SIZE, p->x));
    p->y = fmaxf(STAGE_WALL_SIZE, fminf(GAME_HEIGHT - STAGE_WALL_SIZE - ACTOR_SIZE, p->y));
}
static inline void Stage_Load(int index, Player *p, EnemyGroup *enemies,
                               Projectile *arrows, SlimeShot *shots, RunningEffect *effect)
{
    EnemyLayout_Load(enemyLayouts[index],enemies);
    Projectiles_Reset(arrows);
    memset(shots, 0, sizeof(SlimeShot) * SLIME_SHOT_CAPACITY);
    *effect = (RunningEffect){0};
    p->x = 24; p->y = GAME_HEIGHT / 2.0f - ACTOR_HALF_SIZE;
    p->direction = PLAYER_RIGHT;
    p->aimX = 1; p->aimY = 0;
    p->isMoving = p->isSprinting = p->isAttacking = p->isShooting = false;
    p->isCharging = p->chargedAttack = false; p->chargeTimer = 0;
    p->parryEffectTimer=0;
    p->isMovingUp = p->isMovingDown = p->isMovingLeft = p->isMovingRight = false;
    p->currentFrame = 0; p->animTimer = 0;
    p->moveWasDown = p->sprintButton = 0;
    memset(p->tapRemaining, 0, sizeof(p->tapRemaining));
}
static inline void Stage_BlockEnemyBody(const RoomDefinition *room,float *x,float *y,SDL_FRect body) {
    for(int i=0;i<room->objectCount;++i) {
        SDL_FRect b=Room_MapObjectBody(&room->objects[i]);
        if(b.w<=0 || b.h<=0) continue;
        float dx=body.x+body.w/2-b.x-b.w/2,dy=body.y+body.h/2-b.y-b.h/2;
        float ox=(body.w+b.w)/2-fabsf(dx),oy=(body.h+b.h)/2-fabsf(dy);
        if(ox<=0 || oy<=0) continue;
        if(ox<oy) { float push=dx<0?-ox:ox;*x+=push;body.x+=push; }
        else { float push=dy<0?-oy:oy;*y+=push;body.y+=push; }
    }
}
static inline void Stage_BlockEnemies(int index,EnemyGroup *g) {
    const RoomDefinition *room=&roomDefinitions[index];
    for(int i=0;i<g->ghoulCount;++i) if(g->ghouls[i].hp>0)
        Stage_BlockEnemyBody(room,&g->ghouls[i].x,&g->ghouls[i].y,(SDL_FRect){g->ghouls[i].x,g->ghouls[i].y,16,16});
    for(int i=0;i<g->slimeCount;++i) if(g->slimes[i].hp>0)
        Stage_BlockEnemyBody(room,&g->slimes[i].x,&g->slimes[i].y,(SDL_FRect){g->slimes[i].x,g->slimes[i].y,16,16});
    if(g->coffin.active && g->coffin.state!=FC_FALL && g->coffin.state!=FC_PORTAL && g->coffin.state!=FC_TELEPORT)
        Stage_BlockEnemyBody(room,&g->coffin.x,&g->coffin.y,Coffin_Body(&g->coffin));
}
// Walkable foreground plus the narrow mouth of the cave.
static inline void Entrance_Clamp(Player *p) {
    Stage_ClampPlayer(p);
    SDL_FRect b=Player_Body(p);
    float top=(b.x>=132 && b.x+b.w<=176)?164:192;
    if(b.y<top) p->y=top-PLAYER_HITBOX_OFFSET;
}
static inline void Stage_Update(StageProgress *stage, Player *p, EnemyGroup *enemies,
                                 Projectile *arrows, SlimeShot *shots, RunningEffect *effect)
{
    Stage_BlockEnemies(stage->index,enemies);
    Stage_ClampPlayer(p); // Also contains damage knockback and body pushes.
    Room_Collide(&roomDefinitions[stage->index], p);
    if(stage->index==STAGE_ENTRANCE_ROOM) Entrance_Clamp(p);
    if (p->hp <= 0 || stage->completed) return;
    SDL_FRect exit = Stage_ExitBox(stage->index);
    SDL_FRect back = Stage_BackBox(stage->index);
    SDL_FRect body = Player_Body(p);
    bool backwards = stage->index > 0 && SDL_HasRectIntersectionFloat(&back, &body);
    if(stage->returningHome && !backwards) return;
    SDL_FRect rewardDoor = Stage_DoorBox(STAGE_TOP, 0.5f);
    bool reward = stage->index == STAGE_BOSS_ROOM && SDL_HasRectIntersectionFloat(&rewardDoor, &body);
    if (!backwards && (Stage_EnemiesAlive(enemies) != 0 ||
        (!reward && (stage->index == STAGE_REWARD_ROOM || !SDL_HasRectIntersectionFloat(&exit, &body))))) return;
    if (!backwards && Stage_Next(stage->index) == STAGE_COUNT) { if(!stage->returningHome) stage->completed = true; return; }
    bool fromReward = stage->index == STAGE_REWARD_ROOM;
    stage->saved[stage->index] = *enemies;
    stage->visited[stage->index] = true;
    stage->index = backwards ? Stage_Previous(stage->index) : reward ? STAGE_REWARD_ROOM : Stage_Next(stage->index);
    Stage_Load(stage->index, p, enemies, arrows, shots, effect);
    if (stage->visited[stage->index]) *enemies = stage->saved[stage->index];
    if (fromReward) Stage_PlaceAtDoor(p, rewardDoor, STAGE_TOP);
    else Stage_PlaceAtDoor(p, backwards ? Stage_ExitBox(stage->index) : Stage_BackBox(stage->index),
        backwards ? stageDefinitions[stage->index].nextSide : Stage_BackSide(stage->index));
    if(stage->index==STAGE_ENTRANCE_ROOM) Entrance_Clamp(p);
}
static inline void Stage_Draw(SDL_Renderer *renderer, SDL_Texture *floorAtlas, const StageProgress *stage, const EnemyGroup *enemies)
{
    SDL_SetRenderDrawColor(renderer, 16 + stage->index * 3, 18, 24, 255);
    SDL_FRect floor = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &floor);
    bool night=FLOOR_NIGHT_ENABLED && Stage_IsDirt(stage->index);
    Uint8 oldR=255,oldG=255,oldB=255;
    SDL_GetTextureColorMod(floorAtlas,&oldR,&oldG,&oldB);
    if(night) SDL_SetTextureColorMod(floorAtlas,FLOOR_NIGHT_RED,FLOOR_NIGHT_GREEN,FLOOR_NIGHT_BLUE);
    Floor_Draw(renderer, floorAtlas, stage->floors[stage->index]);
    SDL_SetTextureColorMod(floorAtlas,oldR,oldG,oldB);
    SDL_SetRenderDrawColor(renderer, 75, 75, 85, 255);
    SDL_FRect walls[] = {{0,0,GAME_WIDTH,STAGE_WALL_SIZE}, {0,GAME_HEIGHT-STAGE_WALL_SIZE,GAME_WIDTH,STAGE_WALL_SIZE},
        {0,0,STAGE_WALL_SIZE,GAME_HEIGHT}, {GAME_WIDTH-STAGE_WALL_SIZE,0,STAGE_WALL_SIZE,GAME_HEIGHT}};
    if (!Stage_IsDirt(stage->index))
        for (int i = 0; i < 4; ++i) SDL_RenderFillRect(renderer, &walls[i]);
    int alive = Stage_EnemiesAlive(enemies);
    SDL_SetRenderDrawColor(renderer, alive ? 180 : 50, alive ? 55 : 220, 65, 255);
    SDL_FRect exit = Stage_ExitBox(stage->index);
    if (stage->index != STAGE_REWARD_ROOM) SDL_RenderFillRect(renderer, &exit);
    if (stage->index == STAGE_BOSS_ROOM) {
        SDL_FRect rewardDoor = Stage_DoorBox(STAGE_TOP, 0.5f);
        SDL_RenderFillRect(renderer, &rewardDoor);
    }
    if (stage->index > 0) {
        SDL_SetRenderDrawColor(renderer, 60, 160, 240, 255);
        SDL_FRect back = Stage_BackBox(stage->index);
        SDL_RenderFillRect(renderer, &back);
    }
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    if(stage->index==STAGE_COFFIN_ROOM) SDL_RenderDebugText(renderer,150,8,"FLESH COFFIN");
    else SDL_RenderDebugTextFormat(renderer, 150, 8, "STAGE %d/%d", stage->index + 1, STAGE_COUNT);
    SDL_RenderDebugTextFormat(renderer, 150, 20, "ENEMIES %d", alive);
    //if (!alive) SDL_RenderDebugText(renderer, 184, GAME_HEIGHT / 2.0f - 4, "EXIT -->");
}
#endif
