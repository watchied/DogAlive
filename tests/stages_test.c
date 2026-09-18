#include <assert.h>
#include <stdio.h>
#include "src/core/stages.h"
int main(void)
{
    Player p;
    Player_Init(&p);
    EnemyGroup enemies;
    Projectile arrows[MAX_PROJECTILES] = {0};
    SlimeShot shots[SLIME_SHOT_CAPACITY] = {0};
    RunningEffect effect = {0};
    StageProgress stage = {0};
    Stage_Load(0, &p, &enemies, arrows, shots, &effect);
    p.x = GAME_WIDTH; p.y = GAME_HEIGHT / 2;
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == 0 && p.x == GAME_WIDTH - STAGE_WALL_SIZE - ACTOR_SIZE);
    p.x = -100; p.y = 1000;
    Stage_ClampPlayer(&p);
    assert(p.x == STAGE_WALL_SIZE && p.y == GAME_HEIGHT - STAGE_WALL_SIZE - ACTOR_SIZE);
    p.hp = 50; p.bowCharges = 1;
    for (int level = 0; level < STAGE_COUNT; ++level) {
        assert(stage.index == level);
        for (int i = 0; i < enemies.ghoulCount; ++i) enemies.ghouls[i].hp = 0;
        for (int i = 0; i < enemies.slimeCount; ++i) enemies.slimes[i].hp = 0;
        for (int i = 0; i < enemies.eyeCount; ++i) enemies.eyes[i].hp = 0;
        if (enemies.king.active) { enemies.king.hp = 0; enemies.king.state = KING_NPC; }
        assert(Stage_EnemiesAlive(&enemies) == 0);
        p.x = 24; p.y = 100;
        Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
        assert(stage.index == level && !stage.completed);
        arrows[0].active = shots[0].active = true;
        SDL_FRect door = Stage_ExitBox(stage.index);
        p.x = door.x; p.y = door.y;
        Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
        assert(p.hp == 50 && p.bowCharges == 1);
        if (level + 1 < STAGE_COUNT) {
            assert(stage.index == level + 1 && !stage.completed);
            assert(!arrows[0].active && !shots[0].active);
        }
    }
    assert(stage.completed && stage.index == STAGE_COUNT - 1);
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == STAGE_COUNT - 1);
    // Backtracking preserves partially damaged enemies and cleared rooms.
    stage = (StageProgress){0};
    Stage_Load(0, &p, &enemies, arrows, shots, &effect);
    for (int i = 0; i < enemies.ghoulCount; ++i) enemies.ghouls[i].hp = 0;
    if (enemies.king.active) { enemies.king.hp = 0; enemies.king.state = KING_NPC; }
    SDL_FRect door = Stage_ExitBox(0);
    p.x = door.x; p.y = door.y;
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == 1);
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == 1); // Arrival must not bounce back automatically.
    enemies.ghouls[0].hp = 17;
    door = Stage_BackBox(1);
    p.x = door.x; p.y = door.y;
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == 0 && Stage_EnemiesAlive(&enemies) == 0);
    door = Stage_ExitBox(0);
    p.x = door.x; p.y = door.y;
    Stage_Update(&stage, &p, &enemies, arrows, shots, &effect);
    assert(stage.index == 1 && enemies.ghouls[0].hp == 17);
    for (int side = STAGE_RIGHT; side <= STAGE_BOTTOM; ++side) {
        door = Stage_DoorBox((StageSide)side, 0.5f);
        Stage_PlaceAtDoor(&p, door, (StageSide)side);
        SDL_FRect body = {p.x, p.y, ACTOR_SIZE, ACTOR_SIZE};
        assert(!SDL_HasRectIntersectionFloat(&door, &body));
        assert(p.x >= STAGE_WALL_SIZE && p.y >= STAGE_WALL_SIZE);
        assert(p.x + ACTOR_SIZE <= GAME_WIDTH - STAGE_WALL_SIZE);
        assert(p.y + ACTOR_SIZE <= GAME_HEIGHT - STAGE_WALL_SIZE);
    }
    puts("Stage directions, backtracking and boundary tests passed");
    return 0;
}
