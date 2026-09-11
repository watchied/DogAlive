#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float speed;
    bool facingLeft;
    
    // สำหรับระบบอนิเมชั่น
    int currentFrame;     // เฟรมปัจจุบัน (0, 1, 2, 3)
    float animTimer;      // ตัวนับเวลาสลับเฟรม
    bool isMoving;        // กำลังเคลื่อนที่อยู่หรือไม่
} Player;

void Game_Init(Player *player);
void Game_Update(Player *player, const bool *keyboardState, float deltaTime);

#endif