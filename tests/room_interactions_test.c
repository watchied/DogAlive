#include <assert.h>
#include <stdio.h>
#include "src/core/room_interactions.h"
#include "src/player/bow_input.h"
#include "src/core/game_core.h"

int main(void)
{
    StageProgress s = {0};
    Player p;
    Player_Init(&p);
    EnemyGroup enemies;
    Projectile arrows[MAX_PROJECTILES] = {0};
    SlimeShot shots[SLIME_SHOT_CAPACITY] = {0};
    RunningEffect effect = {0};
    Stage_Load(0, &p, &enemies, arrows, shots, &effect);
    assert(!Room_Interact(&s, &p));
    assert(!p.unlockedArrows && !Projectiles_Shoot(arrows, &p));
    Projectiles_Recharge(&p, 10);
    assert(p.bowCharges == 0);
    p.x = 92; p.y = 112;
    float stamina = p.stamina;
    bool keys[SDL_SCANCODE_COUNT] = {0}; keys[SDL_SCANCODE_SPACE] = true;
    if (Room_Interact(&s, &p)) p.attackWasDown = true;
    Game_Update(&p, keys, 0);
    assert(!p.isAttacking && p.stamina == stamina);
    assert(s.hasCheckpoint && s.checkpointStage == 0 && !Player_HasArrow(&p, ARROW_NORMAL));
    p.x = 192; p.y = 96;
    assert(Room_Interact(&s, &p));
    Room_Update(&s, &p, 1);
    assert(!Player_HasArrow(&p, ARROW_NORMAL));
    assert(Room_Interact(&s, &p) && Player_HasArrow(&p, ARROW_NORMAL));
    p.x = 92; p.y = 112;
    assert(!Player_HasArrow(&p, ARROW_FIRE));
    BowInput input = {0};
    BowInput_Press(&input, 0); BowInput_Update(&input, &p, 500);
    assert(p.arrowType == ARROW_NORMAL);
    Room_Update(&s, &p, 1);
    p.x += 4;
    assert(Room_Interact(&s, &p) && s.respawnX == p.x && s.rooms[0].checkpointTimer > 0);
    float spawnX = p.x;
    s.index = STAGE_BOSS_ROOM;
    Stage_Load(s.index, &p, &enemies, arrows, shots, &effect);
    enemies.king.hp = 1;
    p.hp = 0;
    Stage_Respawn(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == 0 && p.x == spawnX && p.hp == p.maxHP);
    assert(!s.visited[STAGE_BOSS_ROOM]); // Unfinished boss fight resets.
    s.index = STAGE_BOSS_ROOM;
    Stage_Load(s.index, &p, &enemies, arrows, shots, &effect);
    SDL_FRect door = Stage_DoorBox(STAGE_TOP, 0.5f);
    p.x = door.x; p.y = door.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_BOSS_ROOM);
    SDL_FRect sideDoor = Stage_ExitBox(s.index);
    p.x = sideDoor.x; p.y = sideDoor.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_BOSS_ROOM);
    p.x = door.x; p.y = door.y;
    enemies.king.hp = 0; enemies.king.state = KING_NPC;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_REWARD_ROOM);
    door = Stage_BackBox(s.index); p.x = door.x; p.y = door.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_BOSS_ROOM && p.y < 40);
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_BOSS_ROOM);
    sideDoor = Stage_ExitBox(s.index); p.x = sideDoor.x; p.y = sideDoor.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_REWARD_ROOM + 1);
    door = Stage_BackBox(s.index); p.x = door.x; p.y = door.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_BOSS_ROOM && p.x > GAME_WIDTH - 50);
    door = Stage_DoorBox(STAGE_TOP, 0.5f); p.x = door.x; p.y = door.y;
    Stage_Update(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_REWARD_ROOM);
    p.x = 152; p.y = 167;
    assert(Room_Interact(&s, &p) && s.checkpointStage == STAGE_REWARD_ROOM);
    p.x = 92; p.y = 87;
    assert(Room_Interact(&s, &p));
    assert(!Player_HasArrow(&p, ARROW_FIRE));
    Room_Update(&s, &p, 0.1f);
    assert(s.rooms[s.index].chests[0].state == CHEST_OPENING);
    Room_Update(&s, &p, 1);
    assert(s.rooms[s.index].chests[0].state == CHEST_OPEN);
    assert(!Player_HasArrow(&p, ARROW_FIRE));
    assert(Room_Interact(&s, &p) && Player_HasArrow(&p, ARROW_FIRE));
    assert(!Player_HasArrow(&p, ARROW_EXPLOSIVE));
    input = (BowInput){0}; BowInput_Press(&input, 0); BowInput_Update(&input, &p, 500);
    assert(p.arrowType == ARROW_NORMAL); // Skip locked explosive arrows.
    p.x = 212; p.y = 87; p.invincibilityTimer = 100;
    assert(Room_Interact(&s, &p));
    Room_Update(&s, &p, 1);
    assert(p.hp == 0 && s.rooms[s.index].chests[1].blastTimer > 0);
    Stage_Respawn(&s, &p, &enemies, arrows, shots, &effect);
    assert(s.index == STAGE_REWARD_ROOM && p.x == 152 && p.y == 167);
    assert(Player_HasArrow(&p, ARROW_FIRE));
    assert(s.rooms[s.index].chests[0].state == CHEST_COLLECTED);
    assert(s.rooms[s.index].chests[1].state == CHEST_COLLECTED);
    assert(s.saved[STAGE_BOSS_ROOM].king.state == KING_NPC);
    const ChestDefinition *trap = &roomDefinitions[STAGE_REWARD_ROOM].chests[1];
    p.x = trap->x + CHEST_TRAP_RADIUS - PLAYER_HITBOX_OFFSET; p.y = trap->y - ACTOR_HALF_SIZE;
    assert(Room_TrapHits(&p, trap));
    p.x += 0.1f; assert(!Room_TrapHits(&p, trap));
    // Escape during the opening: explosion still plays but does not kill outside its circle.
    s.rooms[s.index].chests[1] = (RoomChest){.state = CHEST_OPENING, .timer = 0.1f};
    Room_Update(&s, &p, 0.2f); assert(p.hp == p.maxHP);
    // All chest states block movement from every side, even at very large steps.
    const RoomDefinition *room = &roomDefinitions[STAGE_START_ROOM];
    SDL_FRect box = Room_ChestBody(&room->chests[0]);
    for (int state = CHEST_CLOSED; state <= CHEST_COLLECTED; ++state) {
        s.rooms[0].chests[0].state = (ChestState)state;
        p.x = box.x + 100; p.y = box.y;
        Room_BlockMovement(room, &p, box.x - 100, box.y);
        assert(p.x == box.x - PLAYER_HITBOX_SIZE - PLAYER_HITBOX_OFFSET);
        p.x = box.x - 100;
        Room_BlockMovement(room, &p, box.x + 100, box.y);
        assert(p.x == box.x + box.w - PLAYER_HITBOX_OFFSET);
        p.x = box.x; p.y = box.y + 100;
        Room_BlockMovement(room, &p, box.x, box.y - 100);
        assert(p.y == box.y - PLAYER_HITBOX_SIZE - PLAYER_HITBOX_OFFSET);
        p.y = box.y - 100;
        Room_BlockMovement(room, &p, box.x, box.y + 100);
        assert(p.y == box.y + box.h - PLAYER_HITBOX_OFFSET);
        p.x = box.x; p.y = box.y;
        Room_Collide(room, &p);
        SDL_FRect body = Player_Body(&p);
        assert(!Ghoul_Overlaps(body, box)); // Touching edges is allowed; penetration is not.
    }
    puts("Checkpoint, unlock, chest, trap, respawn and boss gate tests passed");
    return 0;
}
