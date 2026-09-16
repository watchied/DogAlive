#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "src/core/game_config.h"

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
#define PLAYER_STAMINA_REGEN 20.0f
#define PLAYER_SPRINT_MIN_STAMINA 20.0f
#define PLAYER_DOUBLE_TAP_TIME 0.25f
#define PLAYER_ATTACK_DAMAGE 25
#define PLAYER_ARROW_DAMAGE 15
#define PLAYER_ARROW_SPEED 200.0f
#define PLAYER_SHOOT_COOLDOWN 2.0f
#define PLAYER_ARROW_LIFETIME 1.5f
#define PLAYER_SHOOT_FRAME_COUNT 5
#define PLAYER_SHOOT_RELEASE_FRAME (PLAYER_SHOOT_FRAME_COUNT / 2)
#define PLAYER_SHOOT_FRAME_TIME 0.1f
#define PLAYER_ATTACK_HIT_FRAME 4

typedef enum
{
    PLAYER_DOWN,
    PLAYER_UP,
    PLAYER_LEFT,
    PLAYER_RIGHT
} PlayerDirection;

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
        .arrowLifetime = PLAYER_ARROW_LIFETIME,
        .aimX = 1.0f,
        .aimY = 0.0f,
        .direction = PLAYER_RIGHT};
}

#endif
