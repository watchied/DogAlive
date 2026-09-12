#include "game_core.h"
#include <SDL3/SDL.h>

void Game_Init(Player *player)
{
    Player_Init(player);
}

void Game_Update(Player *player, const bool *keyboardState, float deltaTime)
{
    if (keyboardState == NULL)
        return;

    // ตรวจการกด Space ครั้งใหม่
    bool attackDown = keyboardState[SDL_SCANCODE_SPACE];
    bool attackPressed = attackDown && !player->attackWasDown;
    player->attackWasDown = attackDown;

    if (attackPressed && !player->isAttacking)
    {
        player->isAttacking = true;
        player->attackHasHit = false; // รีเซ็ตทุกครั้งที่เริ่มตี
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }

    if (player->isAttacking)
    {
        player->isMoving = false;
        player->isMovingUp = false;
        player->isMovingDown = false;
        player->isMovingLeft = false;
        player->isMovingRight = false;

        player->animTimer += deltaTime;

        // ท่าตีมี 5 เฟรม ใช้เวลาเฟรมละ 0.1 วินาที
        while (player->animTimer >= 0.1f)
        {
            player->animTimer -= 0.1f;
            player->currentFrame++;

            if (player->currentFrame >= 5)
            {
                player->isAttacking = false;
                player->currentFrame = 0;
                player->animTimer = 0.0f;
                break;
            }
        }

        return;
    }

    int dx =
        (keyboardState[SDL_SCANCODE_D] ||
         keyboardState[SDL_SCANCODE_RIGHT]) -
        (keyboardState[SDL_SCANCODE_A] ||
         keyboardState[SDL_SCANCODE_LEFT]);

    int dy =
        (keyboardState[SDL_SCANCODE_S] ||
         keyboardState[SDL_SCANCODE_DOWN]) -
        (keyboardState[SDL_SCANCODE_W] ||
         keyboardState[SDL_SCANCODE_UP]);

    PlayerDirection previousDirection = player->direction;

    player->isMovingRight = dx > 0;
    player->isMovingLeft = dx < 0;
    player->isMovingUp = dy < 0;
    player->isMovingDown = dy > 0;
    player->isMoving = dx != 0 || dy != 0;

    player->x += dx * player->speed * deltaTime;
    player->y += dy * player->speed * deltaTime;

    // เดินขึ้น = เห็นด้านหลังตัวละคร
    if (dy < 0)
        player->direction = PLAYER_UP;
    else if (dy > 0)
        player->direction = PLAYER_DOWN;
    else if (dx < 0)
        player->direction = PLAYER_LEFT;
    else if (dx > 0)
        player->direction = PLAYER_RIGHT;

    player->facingLeft = player->direction == PLAYER_LEFT;

    if (player->direction != previousDirection)
    {
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }

    if (player->isMoving)
    {
        player->animTimer += deltaTime;

        while (player->animTimer >= 0.25f)
        {
            player->animTimer -= 0.25f;
            player->currentFrame = (player->currentFrame + 1) % 4;
        }
    }
    else
    {
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }
}