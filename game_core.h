#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PLAYER_DOWN,
    PLAYER_UP,
    PLAYER_LEFT,
    PLAYER_RIGHT
} PlayerDirection;

typedef struct {
    float x;
    float y;
    float speed;
    bool facingLeft;
    PlayerDirection direction;
    
    // สำหรับระบบอนิเมชั่น
    int currentFrame;     // เฟรมปัจจุบัน (0, 1, 2, 3)
    float animTimer;      // ตัวนับเวลาสลับเฟรม
    bool isMoving;        // กำลังเคลื่อนที่อยู่หรือไม่
    bool isMovingRight;   // กำลังเคลื่อนที่ไปทางขวาหรือไม่
    bool isMovingLeft;    // กำลังเคลื่อนที่ไปทางซ้ายหรือไม่
    bool isMovingUp;      // กำลังเคลื่อนที่ขึ้นหรือไม่
    bool isMovingDown;    // กำลังเคลื่อนที่ลงหรือไม่
    bool isAttacking;
} Player;

void Game_Init(Player *player);
void Game_Update(Player *player, const bool *keyboardState, float deltaTime);

#endif
