#include <assert.h>
#include <stdio.h>
#include "src/core/game_core.h"
#include "src/combat/enemy_group.h"
#include "src/effects/melee_slash.h"
int main(void)
{
    Player p;
    Player_Init(&p); p.enchantBlade = false; // Test immediate normal slash independently of starting gear.
    bool keys[SDL_SCANCODE_COUNT] = {0};
    // Spend once per accepted swing, reject unaffordable/busy requests.
    p.stamina = PLAYER_MELEE_STAMINA_COST;
    keys[SDL_SCANCODE_SPACE] = true;
    Game_Update(&p, keys, 0.01f);
    assert(p.isAttacking && p.stamina == 0);
    Game_Update(&p, keys, 0.01f);
    assert(p.stamina == 0);
    Player_Init(&p); p.enchantBlade = false; // Test immediate normal slash independently of starting gear.
    p.stamina = PLAYER_MELEE_STAMINA_COST - 1;
    Game_Update(&p, keys, 0.01f);
    assert(!p.isAttacking && p.stamina == PLAYER_MELEE_STAMINA_COST - 1);
    Player_Init(&p); p.enchantBlade = false; // Test immediate normal slash independently of starting gear.
    p.isShooting = true;
    float stamina = p.stamina;
    Game_Update(&p, keys, 0.01f);
    assert(!p.isAttacking && p.stamina == stamina);
    Player_Init(&p); p.enchantBlade = false; // Test immediate normal slash independently of starting gear.
    keys[SDL_SCANCODE_SPACE] = true;
    EnemyGroup enemies = {.ghoulCount = 1};
    Ghoul_Init(&enemies.ghouls[0], p.x + ACTOR_SIZE, p.y);
    bool hit = false;
    for (int i = 0; i < 40; ++i) {
        Game_Update(&p, keys, 0.016f);
        int before = enemies.ghouls[0].hp;
        EnemyGroup_Melee(&enemies, &p);
        if (enemies.ghouls[0].hp != before) {
            assert(!hit);
            hit = true;
            assert(MeleeSlash_Frame(p.currentFrame * MELEE_ACTOR_FRAME_TIME + p.animTimer,
                sword_slash_frames_duration_ms, SWORD_SLASH_FRAMES_COUNT) == 2);
        }
    }
    assert(hit);
    Player_Init(&p); p.enchantBlade = false; // Test immediate normal slash independently of starting gear.
    Ghoul g;
    Ghoul_Init(&g, p.x, p.y);
    g.state = GHOUL_ATTACK;
    hit = false;
    for (int i = 0; i < 70; ++i) {
        int hp = p.hp;
        Ghoul_Update(&g, &p, 0.016f);
        if (p.hp != hp) {
            assert(!hit);
            hit = true;
            assert(MeleeSlash_Frame(g.timer - GHOUL_ATTACK_WINDUP,
                monster_slash_frames_duration_ms, MONSTER_SLASH_FRAMES_COUNT) == 2);
        }
    }
    assert(hit);
    assert(MeleeSlash_Frame(0, sword_slash_frames_duration_ms, 6) == -1);
    assert(MeleeSlash_Frame(0.29f, sword_slash_frames_duration_ms, 6) == -1);
    assert(MeleeSlash_Frame(0.30f, sword_slash_frames_duration_ms, 6) == 0);
    assert(MeleeSlash_Frame(0.40f, sword_slash_frames_duration_ms, 6) == 2);
    assert(MeleeSlash_Frame(0.51f, sword_slash_frames_duration_ms, 6) == -1);
    puts("Player and Ghoul damage matches peak slash frame");
    return 0;
}
