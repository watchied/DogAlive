#include <assert.h>
#include <stdio.h>
#include "src/core/stages.h"
#include "src/ui/slime_king_draw.h"

static int active_shots(const SlimeKing *k)
{
    int n = 0;
    for (int i = 0; i < KING_SHOT_CAPACITY; ++i) n += k->shots[i].active;
    return n;
}
int main(void)
{
    Player p;
    Player_Init(&p);
    p.x = 24; p.y = 110;
    SlimeKing k;
    // All attack states accept damage, including preparation for burrow/slam.
    for (int state = KING_BUBBLE; state <= KING_BEAM_FIRE; ++state) {
        King_Init(&k, 200, 100);
        k.hp = k.maxHP = 1000;
        King_Enter(&k, (KingState)state);
        assert(King_Targetable(&k));
        King_TakeDamage(&k, 10, true);
        assert(k.hp == 990);
    }
    King_Init(&k, 200, 100);
    assert(!King_Targetable(&k));
    for (int i = 0; i < SLIME_KING_BOSS_OPENING1_COUNT; ++i) {
        k.timer = (i + 0.5f) * KING_INTRO_TIME / SLIME_KING_BOSS_OPENING1_COUNT;
        assert(King_Sprite(&k) == slime_king_boss_opening1[i]);
    }
    k.timer = 0;
    King_Update(&k, &p, KING_INTRO_TIME * 0.75f);
    assert(k.state == KING_INTRO);
    King_Update(&k, &p, KING_INTRO_TIME * 0.26f);
    assert(k.state == KING_IDLE && King_Targetable(&k));
    King_StartSkill(&k, &p, 0);
    King_Update(&k, &p, KING_BUBBLE_WINDUP);
    assert(active_shots(&k) == 1 && k.shots[0].type == KING_SHOT_BUBBLE);
    // Empty arena: expire precisely on the eighth wall contact, including large dt.
    KingShot bubble = {.x = 150, .y = 70, .vx = 200, .radius = 7,
        .lifetime = 100, .active = true, .bounceLimit = 8};
    Player absent = {.hp = 0};
    for (int i = 0; i < 10000 && bubble.active; ++i) King_UpdateShot(&bubble, &absent, 0.01f);
    assert(bubble.bounces == 8 && !bubble.active);
    KingShot laser = {.x = 150, .y = 70, .vx = 200, .radius = 3,
        .lifetime = 100, .active = true, .bounceLimit = 2};
    King_UpdateShot(&laser, &absent, 10);
    assert(laser.bounces == 2 && !laser.active);
    // High-speed beam cannot tunnel through the player.
    p.hp = 100; p.invincibilityTimer = 0; p.x = 100; p.y = 100;
    KingShot beam = {.x = 20, .y = 108, .vx = KING_BEAM_SPEED, .radius = KING_BEAM_RADIUS,
        .damage = KING_BEAM_DAMAGE, .lifetime = 1, .active = true};
    King_UpdateShot(&beam, &p, 0.2f);
    assert(p.hp == 100 - KING_BEAM_DAMAGE && !beam.active);
    King_Init(&k, 200, 100); King_Enter(&k, KING_IDLE);
    King_StartSkill(&k, &p, 1);
    float x = k.x;
    King_Update(&k, &p, KING_DASH_WINDUP);
    assert(k.state == KING_DASH);
    King_Update(&k, &p, 0.05f);
    assert(k.x < x);
    King_Init(&k, 200, 100); King_Enter(&k, KING_IDLE);
    King_StartSkill(&k, &p, 2);
    int expectedSlams = k.slamsLeft, landed = 0;
    assert(expectedSlams >= KING_SLAM_MIN && expectedSlams <= KING_SLAM_MAX);
    p.invincibilityTimer = 100;
    for (int i = 0; i < 10000 && k.state != KING_IDLE; ++i) {
        KingState old = k.state;
        King_Update(&k, &p, 0.01f);
        if (old != KING_SLAM_LAND && k.state == KING_SLAM_LAND) ++landed;
    }
    assert(landed == expectedSlams);
    King_Enter(&k, KING_IDLE);
    King_TakeDamage(&k, KING_MAX_HP / 2, true);
    assert(k.phase == 2 && k.state == KING_PHASE_CHANGE && active_shots(&k) == 0);
    for (int i = 0; i < SLIME_KING_BOSS_OPENING2_COUNT; ++i) {
        k.timer = (i + 0.5f) * KING_PHASE_TRANSITION_TIME / SLIME_KING_BOSS_OPENING2_COUNT;
        assert(King_Sprite(&k) == slime_king_boss_opening2[i]);
    }
    k.timer = 0;
    int phaseHP = k.hp;
    King_TakeDamage(&k, 1000, true);
    assert(k.hp == phaseHP);
    King_Update(&k, &p, KING_PHASE_TRANSITION_TIME * 0.75f);
    assert(k.state == KING_PHASE_CHANGE);
    King_Update(&k, &p, KING_PHASE_TRANSITION_TIME * 0.26f);
    King_StartSkill(&k, &p, 2);
    King_Update(&k, &p, KING_RADIAL_WINDUP);
    assert(active_shots(&k) == 8);
    memset(k.shots, 0, sizeof(k.shots));
    King_StartSkill(&k, &p, 3);
    King_Update(&k, &p, KING_SPIRAL_WINDUP + KING_SPIRAL_COUNT * KING_SPIRAL_INTERVAL);
    assert(active_shots(&k) == 10);
    memset(k.shots, 0, sizeof(k.shots));
    King_StartSkill(&k, &p, 4);
    King_TakeDamage(&k, 1, false);
    assert(k.state == KING_BEAM_CHARGE); // Burn is not an interrupt.
    King_TakeDamage(&k, KING_BEAM_INTERRUPT_DAMAGE, true);
    assert(k.state == KING_STUN);
    King_Update(&k, &p, KING_BEAM_CHARGE_TIME + 1);
    assert(active_shots(&k) == 0);
    King_StartSkill(&k, &p, 4);
    King_Update(&k, &p, KING_BEAM_CHARGE_TIME);
    assert(active_shots(&k) == 1 && k.shots[0].type == KING_SHOT_BEAM);

    // The regular combat path must hit the boss with arrows and burn ticks.
    EnemyGroup group = {0};
    King_Init(&group.king, 150, 100); King_Enter(&group.king, KING_IDLE);
    group.king.hp = group.king.maxHP = 1000; // Isolate damage tests from configurable boss HP.
    Projectile arrows[MAX_PROJECTILES] = {0};
    arrows[0] = (Projectile){.x = 20, .y = 130, .vx = 1000, .lifetime = 1,
        .damage = 20, .active = true, .type = ARROW_FIRE};
    EnemyGroup_Arrows(&group, arrows, 0.2f);
    assert(group.king.hp == 980 && !arrows[0].active);
    EnemyGroup_UpdateArrowEffects(&group, 3.01f);
    assert(group.king.hp == 950);
    // Sword and reflected Slime shots share the boss's direct-hit damage path.
    SDL_FRect bossBody = King_Body(&group.king);
    p.x = bossBody.x - ACTOR_SIZE; p.y = bossBody.y;
    p.direction = PLAYER_RIGHT; p.isAttacking = true;
    p.currentFrame = PLAYER_ATTACK_HIT_FRAME; p.attackHasHit = false;
    int before = group.king.hp;
    EnemyGroup_Melee(&group, &p);
    assert(group.king.hp == before - p.attackDamage);
    assert(EnemyGroup_Sweep(&group, 20, bossBody.y + 8, 1000, 0, 0.2f, 0, 2, 15));
    assert(group.king.hp == before - p.attackDamage - 15);
    // Explosive direct hit receives base damage plus the area damage.
    before = group.king.hp;
    arrows[0] = (Projectile){.x = 20, .y = bossBody.y + 8, .vx = 1000, .lifetime = 1,
        .damage = 20, .active = true, .type = ARROW_EXPLOSIVE};
    EnemyGroup_Arrows(&group, arrows, 0.2f);
    assert(group.king.hp == before - 55);
    // Burn can cross the phase boundary, and the remaining ticks pause rather than disappear.
    group.king.hp = (int)(group.king.maxHP * KING_PHASE_TWO_RATIO) + 1;
    group.burns[3][0] = (EnemyBurn){.damageLeft = 30, .ticksLeft = 6, .timer = 0.5f};
    EnemyGroup_UpdateArrowEffects(&group, 3);
    assert(group.king.phase == 2 && group.king.state == KING_PHASE_CHANGE);
    assert(group.burns[3][0].ticksLeft == 5);
    King_Update(&group.king, &p, KING_PHASE_TRANSITION_TIME);
    group.king.hp = 20;
    group.burns[3][0] = (EnemyBurn){0};
    King_TakeDamage(&group.king, 20, true);
    assert(group.king.state == KING_DEATH && Stage_EnemiesAlive(&group) == 1);
    for (int i = 0; i < SLIME_KING_BOSS_DEATH_COUNT; ++i) {
        group.king.timer = i * KING_DEATH_TIME / SLIME_KING_BOSS_DEATH_COUNT;
        King_Update(&group.king, &p, KING_DEATH_TIME / SLIME_KING_BOSS_DEATH_COUNT * 0.5f);
        assert(group.king.state == KING_DEATH && group.king.frame == i);
        assert(Stage_EnemiesAlive(&group) == 1);
    }
    King_Update(&group.king, &p, KING_DEATH_TIME / SLIME_KING_BOSS_DEATH_COUNT);
    assert(group.king.state == KING_NPC && Stage_EnemiesAlive(&group) == 0);

    // Gate stays locked during the death animation, then unlocks and preserves NPC on return.
    StageProgress stage = {0};
    SlimeShot shots[SLIME_SHOT_CAPACITY] = {0};
    RunningEffect effects = {0};
    Player_Init(&p);
    Stage_Load(0, &p, &group, arrows, shots, &effects);
    SDL_FRect exit = Stage_ExitBox(0);
    p.x = exit.x; p.y = exit.y;
    Stage_Update(&stage, &p, &group, arrows, shots, &effects);
    assert(stage.index == 0);
    King_Enter(&group.king, KING_IDLE);
    King_TakeDamage(&group.king, KING_MAX_HP, true);
    Stage_Update(&stage, &p, &group, arrows, shots, &effects);
    assert(stage.index == 0);
    King_Update(&group.king, &p, KING_DEATH_TIME);
    Stage_Update(&stage, &p, &group, arrows, shots, &effects);
    assert(stage.index == 1);
    exit = Stage_BackBox(1); p.x = exit.x; p.y = exit.y;
    Stage_Update(&stage, &p, &group, arrows, shots, &effects);
    assert(stage.index == 0 && group.king.state == KING_NPC);
    // Every state selects a safe sprite pointer (NPC has a separate smaller sprite).
    for (int state = KING_INTRO; state < KING_NPC; ++state) {
        King_Enter(&k, (KingState)state);
        assert(King_Sprite(&k) != NULL);
    }
    puts("Slime King phases, attacks, bounce limits, combat and exit tests passed");
    return 0;
}
