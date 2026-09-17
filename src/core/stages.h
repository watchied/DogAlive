#ifndef STAGES_H
#define STAGES_H
#include "src/combat/enemy_group.h"
#include "src/effects/running_effect.h"
#include "src/core/floor_tiles.h"

#define STAGE_COUNT 5
#define STAGE_WALL_SIZE 4.0f
typedef enum { STAGE_RIGHT, STAGE_LEFT, STAGE_TOP, STAGE_BOTTOM } StageSide;
typedef struct { int ghouls, slimes, eyes; StageSide nextSide; } StageDefinition;
// {ghouls, slimes, eyes, next exit side}. Choose STAGE_TOP/BOTTOM/LEFT/RIGHT.
// The next room's return door is automatically on the opposite side.
// If both doors share a wall, they are separated at 30% and 70% of that wall.
static const StageDefinition stageDefinitions[STAGE_COUNT] = {
    {2, 0, 0, STAGE_RIGHT},
    {2, 1, 0, STAGE_TOP},
    {3, 1, 1, STAGE_RIGHT},
    {3, 2, 2, STAGE_BOTTOM},
    {4, 3, 2, STAGE_RIGHT}
};
typedef struct {
    int index;
    bool completed;
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
    return count;
}
static inline StageSide Stage_Opposite(StageSide side)
{
    return side == STAGE_RIGHT ? STAGE_LEFT : side == STAGE_LEFT ? STAGE_RIGHT :
        side == STAGE_TOP ? STAGE_BOTTOM : STAGE_TOP;
}
static inline StageSide Stage_BackSide(int index)
{
    return index > 0 ? Stage_Opposite(stageDefinitions[index - 1].nextSide) : STAGE_LEFT;
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
    bool sameSide = index > 0 && Stage_BackSide(index) == stageDefinitions[index].nextSide;
    return Stage_DoorBox(stageDefinitions[index].nextSide, sameSide ? 0.7f : 0.5f);
}
static inline SDL_FRect Stage_BackBox(int index)
{
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
    StageDefinition d = stageDefinitions[index];
    EnemyGroup_InitCounts(enemies, d.ghouls, d.slimes, d.eyes);
    Projectiles_Reset(arrows);
    memset(shots, 0, sizeof(SlimeShot) * SLIME_SHOT_CAPACITY);
    *effect = (RunningEffect){0};
    p->x = 24; p->y = GAME_HEIGHT / 2.0f - ACTOR_HALF_SIZE;
    p->direction = PLAYER_RIGHT;
    p->aimX = 1; p->aimY = 0;
    p->isMoving = p->isSprinting = p->isAttacking = p->isShooting = false;
    p->isMovingUp = p->isMovingDown = p->isMovingLeft = p->isMovingRight = false;
    p->currentFrame = 0; p->animTimer = 0;
    p->moveWasDown = p->sprintButton = 0;
    memset(p->tapRemaining, 0, sizeof(p->tapRemaining));
}
static inline void Stage_Update(StageProgress *stage, Player *p, EnemyGroup *enemies,
                                 Projectile *arrows, SlimeShot *shots, RunningEffect *effect)
{
    Stage_ClampPlayer(p); // Also contains damage knockback and body pushes.
    if (p->hp <= 0 || stage->completed) return;
    SDL_FRect exit = Stage_ExitBox(stage->index);
    SDL_FRect back = Stage_BackBox(stage->index);
    SDL_FRect body = {p->x, p->y, ACTOR_SIZE, ACTOR_SIZE};
    bool backwards = stage->index > 0 && SDL_HasRectIntersectionFloat(&back, &body);
    if (!backwards && (Stage_EnemiesAlive(enemies) != 0 || !SDL_HasRectIntersectionFloat(&exit, &body))) return;
    if (!backwards && stage->index + 1 == STAGE_COUNT) { stage->completed = true; return; }
    stage->saved[stage->index] = *enemies;
    stage->visited[stage->index] = true;
    stage->index += backwards ? -1 : 1;
    Stage_Load(stage->index, p, enemies, arrows, shots, effect);
    if (stage->visited[stage->index]) *enemies = stage->saved[stage->index];
    Stage_PlaceAtDoor(p, backwards ? Stage_ExitBox(stage->index) : Stage_BackBox(stage->index),
        backwards ? stageDefinitions[stage->index].nextSide : Stage_BackSide(stage->index));
}
static inline void Stage_Draw(SDL_Renderer *renderer, SDL_Texture *floorAtlas, const StageProgress *stage, const EnemyGroup *enemies)
{
    SDL_SetRenderDrawColor(renderer, 16 + stage->index * 3, 18, 24, 255);
    SDL_FRect floor = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &floor);
    Floor_Draw(renderer, floorAtlas, stage->floors[stage->index]);
    SDL_SetRenderDrawColor(renderer, 75, 75, 85, 255);
    SDL_FRect walls[] = {{0,0,GAME_WIDTH,STAGE_WALL_SIZE}, {0,GAME_HEIGHT-STAGE_WALL_SIZE,GAME_WIDTH,STAGE_WALL_SIZE},
        {0,0,STAGE_WALL_SIZE,GAME_HEIGHT}, {GAME_WIDTH-STAGE_WALL_SIZE,0,STAGE_WALL_SIZE,GAME_HEIGHT}};
    for (int i = 0; i < 4; ++i) SDL_RenderFillRect(renderer, &walls[i]);
    int alive = Stage_EnemiesAlive(enemies);
    SDL_SetRenderDrawColor(renderer, alive ? 180 : 50, alive ? 55 : 220, 65, 255);
    SDL_FRect exit = Stage_ExitBox(stage->index);
    SDL_RenderFillRect(renderer, &exit);
    if (stage->index > 0) {
        SDL_SetRenderDrawColor(renderer, 60, 160, 240, 255);
        SDL_FRect back = Stage_BackBox(stage->index);
        SDL_RenderFillRect(renderer, &back);
    }
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderDebugTextFormat(renderer, 150, 8, "STAGE %d/%d", stage->index + 1, STAGE_COUNT);
    SDL_RenderDebugTextFormat(renderer, 150, 20, "ENEMIES %d", alive);
    //if (!alive) SDL_RenderDebugText(renderer, 184, GAME_HEIGHT / 2.0f - 4, "EXIT -->");
}
#endif
