#include <assert.h>
#include <stdio.h>
#include "src/core/game_core.h"
#include "src/core/room_interactions.h"

int main(void)
{
    Player p; Player_Init(&p);
    bool keys[SDL_SCANCODE_COUNT] = {0};
    StageProgress stage = {.index = 3};
    p.x = 262; p.y = 41;
    assert(Room_Interact(&stage, &p));
    Room_Update(&stage, &p, 1);
    assert(Room_Interact(&stage, &p) && p.enchantBlade && !p.unlockedArrows);
    keys[SDL_SCANCODE_SPACE] = true;
    Game_Update(&p, keys, 0.1f);
    assert(p.isCharging && !p.isAttacking && p.stamina == p.maxStamina);
    keys[SDL_SCANCODE_SPACE] = false;
    Game_Update(&p, keys, 0);
    assert(p.isAttacking && !p.chargedAttack);
    assert(p.stamina == p.maxStamina - PLAYER_MELEE_STAMINA_COST);
    Player_Init(&p); p.enchantBlade = true;
    keys[SDL_SCANCODE_SPACE] = true;
    for (int i = 0; i < 260; ++i) Game_Update(&p, keys, 0.01f);
    assert(p.isCharging && !p.isAttacking && p.chargeTimer == PLAYER_CHARGE_TIME);
    keys[SDL_SCANCODE_SPACE] = false;
    Game_Update(&p, keys, 0);
    assert(p.isAttacking && p.chargedAttack && !p.isCharging);
    assert(Player_MeleeDamage(&p) == (int)(p.attackDamage * PLAYER_CHARGE_DAMAGE_MULTIPLIER));
    for (int dir = PLAYER_DOWN; dir <= PLAYER_RIGHT; ++dir) {
        p.direction = (PlayerDirection)dir;
        SDL_FRect b = Player_AttackBox(&p);
        assert(b.w == (dir <= PLAYER_UP ? PLAYER_CHARGE_WIDTH : PLAYER_CHARGE_REACH));
        assert(b.h == (dir <= PLAYER_UP ? PLAYER_CHARGE_REACH : PLAYER_CHARGE_WIDTH));
    }
    EnemyGroup enemies = {.ghoulCount = 1};
    p.direction = PLAYER_RIGHT; p.currentFrame = PLAYER_ATTACK_HIT_FRAME;
    Ghoul_Init(&enemies.ghouls[0], p.x + ACTOR_SIZE + 45, p.y);
    enemies.ghouls[0].hp = 1000;
    EnemyGroup_Melee(&enemies, &p);
    assert(enemies.ghouls[0].hp == 1000 - Player_MeleeDamage(&p));
    EnemyGroup_Melee(&enemies, &p);
    assert(enemies.ghouls[0].hp == 1000 - Player_MeleeDamage(&p));
    // Main-loop order: player melee resolves before enemy update. A parry must
    // cancel both damage paths, including when the hitboxes overlap the bodies.
    for (int charged = 0; charged <= 1; ++charged) {
        Player_Init(&p); p.x = 100; p.y = 100; p.isAttacking = true;
        p.chargedAttack = charged; p.currentFrame = PLAYER_ATTACK_HIT_FRAME;
        EnemyGroup clash = {.ghoulCount = 1};
        Ghoul_Init(&clash.ghouls[0], 120, 100);
        Ghoul *foe = &clash.ghouls[0];
        foe->state = GHOUL_ATTACK; foe->direction = PLAYER_LEFT;
        foe->timer = GHOUL_ATTACK_WINDUP + 0.4f;
        int hp = foe->hp;
        EnemyGroup_Melee(&clash, &p);
        Ghoul_Update(foe, &p, 0.01f);
        assert(foe->hp == hp && p.hp == p.maxHP && foe->hasHit);
        p.attackHasHit = false; // Even a later damage check in the same swing is suppressed.
        EnemyGroup_Melee(&clash, &p);
        assert(foe->hp == hp);
        ++p.meleeSwingId; p.attackHasHit = false; foe->state = GHOUL_REST;
        EnemyGroup_Melee(&clash, &p);
        assert(foe->hp < hp); // A new ordinary hit still deals damage.
    }
    // Parry consumes the enemy attack once, for either kind of slash.
    for (int charged = 0; charged <= 1; ++charged) {
        Player_Init(&p); p.x = 100; p.y = 100;
        p.isAttacking = true; p.chargedAttack = charged;
        p.currentFrame = PLAYER_ATTACK_HIT_FRAME;
        Ghoul g; Ghoul_Init(&g, 120, 100);
        g.direction = PLAYER_LEFT; g.state = GHOUL_ATTACK;
        g.timer = GHOUL_ATTACK_WINDUP + 0.4f;
        Ghoul_Update(&g, &p, 0.01f);
        assert(g.hasHit && p.hp == p.maxHP && p.hitFlashTimer > 0);
        p.isAttacking = false;
        Ghoul_Update(&g, &p, 0.01f);
        assert(p.hp == p.maxHP);
    }
    Player_Init(&p); p.x = 100; p.y = 100; p.isCharging = true;
    Ghoul g; Ghoul_Init(&g, 120, 100); g.direction = PLAYER_LEFT;
    g.state = GHOUL_ATTACK; g.timer = GHOUL_ATTACK_WINDUP + 0.4f;
    Ghoul_Update(&g, &p, 0.01f);
    assert(p.hp < p.maxHP); // Holding alone is not a parry.
    Player_Init(&p); p.enchantBlade = true; p.stamina = 0;
    keys[SDL_SCANCODE_SPACE] = true; Game_Update(&p, keys, PLAYER_CHARGE_TIME);
    keys[SDL_SCANCODE_SPACE] = false; Game_Update(&p, keys, 0);
    assert(!p.isAttacking && p.stamina == 0);
    Player_Init(&p); p.enchantBlade = true;
    keys[SDL_SCANCODE_SPACE] = keys[SDL_SCANCODE_D] = true;
    float startX = p.x, startY = p.y;
    Game_Update(&p, keys, 0.1f);
    assert(p.isCharging && !p.isSprinting);
    assert(fabsf(p.x - startX - p.speed * PLAYER_CHARGE_MOVE_MULTIPLIER * 0.1f) < 0.001f);
    keys[SDL_SCANCODE_W] = true; startX = p.x; startY = p.y;
    Game_Update(&p, keys, 0.1f);
    assert(fabsf(hypotf(p.x - startX, p.y - startY) - p.speed * PLAYER_CHARGE_MOVE_MULTIPLIER * 0.1f) < 0.001f);
    p.chargeTimer = PLAYER_CHARGE_TIME;
    keys[SDL_SCANCODE_SPACE] = false;
    PlayerDirection facing = p.direction;
    startX = p.x;
    Game_Update(&p, keys, 0.1f);
    assert(p.isAttacking && p.chargedAttack && p.x > startX && p.direction == facing);
    assert(p.currentFrame == 1); // Walking must not overwrite the slash animation.
    p.direction = PLAYER_RIGHT;
    SDL_FRect chargedBox = Player_AttackBox(&p);
    assert(chargedBox.x == p.x + ACTOR_HALF_SIZE);
    assert(chargedBox.w == PLAYER_CHARGE_REACH && chargedBox.h == PLAYER_CHARGE_WIDTH);
    puts("Charge movement, geometry, unlock, damage, stamina and parry tests passed");
    return 0;
}
