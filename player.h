#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#define PLAYER_START_X 100.0f
#define PLAYER_START_Y 100.0f
#define PLAYER_DEFAULT_SPEED 120.0f
#define PLAYER_MAX_HP 100
#define PLAYER_ATTACK_DAMAGE 25
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
    int maxHP;
    int attackDamage;
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
        .attackDamage = PLAYER_ATTACK_DAMAGE,
        .direction = PLAYER_RIGHT};
}

#endif
