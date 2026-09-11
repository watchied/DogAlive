#include "game_core.h"
#include <SDL3/SDL.h>

void Game_Init(Player *player) {
    player->x = 100.0f;
    player->y = 100.0f;
    player->speed = 120.0f;
    player->facingLeft = false;
    
    player->currentFrame = 0;
    player->animTimer = 0.0f;
    player->isMoving = false;
}

void Game_Update(Player *player, const bool *keyboardState, float deltaTime) {
    if (keyboardState == NULL) return;

    float moveAmount = player->speed * deltaTime;
    player->isMoving = false;

    // ตรวจสอบการกดปุ่ม
    if (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP]) {
        player->y -= moveAmount;
        player->isMoving = true;
    }
    if (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN]) {
        player->y += moveAmount;
        player->isMoving = true;
    }
    if (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT]) {
        player->x -= moveAmount;
        player->facingLeft = true;
        player->isMoving = true;
    }
    if (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT]) {
        player->x += moveAmount;
        player->facingLeft = false;
        player->isMoving = true;
    }

    // คำนวณอนิเมชั่นเมื่อมีการเคลื่อนที่
    // คำนวณอนิเมชั่นเมื่อมีการเคลื่อนที่
if (player->isMoving) {
    player->animTimer += deltaTime;
    
    // ปรับเปลี่ยนความเร็วอนิเมชั่นตรงนี้:ฅ
    // 0.1f  = 10 FPS (เร็ว)
    // 0.2f  = 5  FPS (กำลังดี/ช้าลงเท่าตัว)
    // 0.25f = 4  FPS (ช้าละเมียด)
    if (player->animTimer >= 0.25f) { 
        player->animTimer = 0.0f;
        player->currentFrame = (player->currentFrame + 1) % 4; // วนลูป 0 -> 1 -> 2 -> 3
    }
} else {
    // เมื่อหยุดเดิน ให้กลับไปยืนท่าปกติ (เฟรม 0)
    player->currentFrame = 0;
    player->animTimer = 0.0f;
}
}