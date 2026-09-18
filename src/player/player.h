#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "src/core/game_config.h"
#include "src/effects/melee_timing.h"

#define PLAYER_START_X 100.0f
#define PLAYER_START_Y 100.0f
#define PLAYER_DEFAULT_SPEED 40.0f
#define PLAYER_MAX_HP 100
#define PLAYER_COLLISION_GRACE_TIME 0.5f
#define PLAYER_HIT_FLASH_TIME 0.16f
#define PLAYER_INVINCIBILITY_TIME 0.3f
#define PLAYER_HIT_KNOCKBACK 2.0f
#define PLAYER_MAX_STAMINA 100
#define PLAYER_SPRINT_MULTIPLIER 1.7f
#define PLAYER_STAMINA_DRAIN 25.0f
#define PLAYER_MELEE_REACH 12.0f // Melee reach in world pixels; affects attacks and slime parries.
#define PLAYER_STAMINA_REGEN 20.0f
#define PLAYER_SPRINT_MIN_STAMINA 20.0f
#define PLAYER_DOUBLE_TAP_TIME 0.25f
#define PLAYER_ATTACK_DAMAGE 25
#define PLAYER_ARROW_DAMAGE 15
#define PLAYER_ARROW_SPEED 200.0f
#define PLAYER_SHOOT_COOLDOWN 2.5f
#define PLAYER_BOW_MAX_CHARGES 3
#define FIRE_ARROW_BURN_MULTIPLIER 1.5f
#define FIRE_ARROW_TICKS 6
#define FIRE_ARROW_TICK_TIME 0.5f
#define EXPLOSIVE_ARROW_MULTIPLIER 1.75f
#define EXPLOSIVE_ARROW_RADIUS 24.0f
#define PLAYER_ARROW_LIFETIME 1.5f
#define PLAYER_SHOOT_FRAME_COUNT 5
#define PLAYER_SHOOT_RELEASE_FRAME (PLAYER_SHOOT_FRAME_COUNT / 2)
#define PLAYER_SHOOT_FRAME_TIME 0.05f
#define PLAYER_ATTACK_HIT_FRAME MELEE_HIT_FRAME

typedef enum
{
    PLAYER_DOWN,
    PLAYER_UP,
    PLAYER_LEFT,
    PLAYER_RIGHT
} PlayerDirection;

typedef enum { ARROW_NORMAL, ARROW_FIRE, ARROW_EXPLOSIVE } ArrowType;
static inline int Arrow_ChargeCost(ArrowType type) { return (int)type + 1; }

typedef struct
{
    float x;
    float y;
    float speed;
    int hp;
    bool deathStarted;
    bool deathFinished;
    int deathFrame;
    float deathTimer;
    float hitFlashTimer;
    float collisionGraceTimer;
    float invincibilityTimer;
    int maxHP;
    int attackDamage;
    int arrowDamage;
    float stamina;
    float maxStamina;
    bool isSprinting;
    unsigned int moveWasDown;
    unsigned int sprintButton;
    float tapRemaining[4];
    float arrowSpeed;
    float shootCooldown;
    float arrowLifetime;
    float shootTimer;
    int bowCharges;
    ArrowType arrowType;
    ArrowType shootingArrowType; // Snapshot so switching cannot change an already paid shot.
    bool isShooting;
    bool arrowReleased;
    float aimX, aimY;
    bool facingLeft;
    PlayerDirection direction;
    

    // สำหรับระบบอนิเมชั่น
    int currentFrame;   // เฟรมปัจจุบัน (0, 1, 2, 3)
    float animTimer;    // ตัวนับเวลาสลับเฟรม
    bool isMoving;      // กำลังเคลื่อนที่อยู่หรือไม่
    bool isMovingRight; // กำลังเคลื่อนที่ไปทางขวาหรือไม่
    bool isMovingLeft;  // กำลังเคลื่อนที่ไปทางซ้ายหรือไม่
    bool isMovingUp;    // กำลังเคลื่อนที่ขึ้นหรือไม่
    bool isMovingDown;  // กำลังเคลื่อนที่ลงหรือไม่
    bool isAttacking;
    bool attackWasDown;
    bool attackHasHit;
} Player;

// Player melee hitbox extends from the front edge of the body.
static inline SDL_FRect Player_AttackBox(const Player *p)
{
    switch (p->direction)
    {
    case PLAYER_UP:
        return (SDL_FRect){p->x, p->y - PLAYER_MELEE_REACH, ACTOR_SIZE, PLAYER_MELEE_REACH};
    case PLAYER_DOWN:
        return (SDL_FRect){p->x, p->y + ACTOR_SIZE, ACTOR_SIZE, PLAYER_MELEE_REACH};
    case PLAYER_LEFT:
        return (SDL_FRect){p->x - PLAYER_MELEE_REACH, p->y, PLAYER_MELEE_REACH, ACTOR_SIZE};
    default:
        return (SDL_FRect){p->x + ACTOR_SIZE, p->y, PLAYER_MELEE_REACH, ACTOR_SIZE};
    }
}

static inline void Player_Init(Player *player)
{
    *player = (Player){
        .x = PLAYER_START_X,
        .y = PLAYER_START_Y,
        .speed = PLAYER_DEFAULT_SPEED,
        .hp = PLAYER_MAX_HP,
        .maxHP = PLAYER_MAX_HP,
        .stamina = PLAYER_MAX_STAMINA,
        .maxStamina = PLAYER_MAX_STAMINA,
        .attackDamage = PLAYER_ATTACK_DAMAGE,
        .arrowDamage = PLAYER_ARROW_DAMAGE,
        .arrowSpeed = PLAYER_ARROW_SPEED,
        .shootCooldown = PLAYER_SHOOT_COOLDOWN,
        .bowCharges = PLAYER_BOW_MAX_CHARGES,
        .arrowLifetime = PLAYER_ARROW_LIFETIME,
        .aimX = 1.0f,
        .aimY = 0.0f,
        .direction = PLAYER_RIGHT};
}

#endif
