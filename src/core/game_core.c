#include "src/core/game_core.h"
#include <SDL3/SDL.h>
#include <math.h>

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

    bool down[4] = {dy > 0, dy < 0, dx < 0, dx > 0};
    unsigned int held = 0;
    bool actionBusy = player->isShooting || player->isAttacking || attackPressed;
    for (int i = 0; i < 4; ++i) {
        unsigned int bit = 1u << i;
        player->tapRemaining[i] = fmaxf(0.0f, player->tapRemaining[i] - deltaTime);
        if (down[i]) held |= bit;
        if (actionBusy) {
            player->tapRemaining[i] = 0.0f;
        } else if (down[i] && !(player->moveWasDown & bit)) {
            if (player->tapRemaining[i] > 0.0f &&
                player->stamina >= PLAYER_SPRINT_MIN_STAMINA) {
                player->isSprinting = true;
                player->sprintButton = bit;
                player->tapRemaining[i] = 0.0f;
            } else {
                player->tapRemaining[i] = PLAYER_DOUBLE_TAP_TIME;
            }
        }
    }
    player->moveWasDown = held;
    if (actionBusy || !(held & player->sprintButton) || player->stamina <= 0.0f)
        player->isSprinting = false;

    float moveMultiplier = 1.0f;
    if (player->isSprinting) {
        // If stamina runs out mid-frame, sprint only for the affordable time.
        float sprintTime = fminf(deltaTime, player->stamina / PLAYER_STAMINA_DRAIN);
        if (deltaTime > 0.0f)
            moveMultiplier += (PLAYER_SPRINT_MULTIPLIER - 1.0f) * sprintTime / deltaTime;
        player->stamina = fmaxf(0.0f, player->stamina - PLAYER_STAMINA_DRAIN * deltaTime);
        if (player->stamina <= 0.0f)
            player->isSprinting = false;
    } else if (!actionBusy) {
        player->stamina = fminf(player->maxStamina,
            player->stamina + PLAYER_STAMINA_REGEN * deltaTime);
    }

    if (player->isShooting)
        return;

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
        while (player->animTimer >= MELEE_ACTOR_FRAME_TIME)
        {
            player->animTimer -= MELEE_ACTOR_FRAME_TIME;
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

    // D-pad aiming: keep the last direction when all movement buttons are released.
    if (dx != 0 || dy != 0) {
        float aimScale = (dx != 0 && dy != 0) ? 0.70710678f : 1.0f;
        player->aimX = dx * aimScale;
        player->aimY = dy * aimScale;
    }

    PlayerDirection previousDirection = player->direction;

    player->isMovingRight = dx > 0;
    player->isMovingLeft = dx < 0;
    player->isMovingUp = dy < 0;
    player->isMovingDown = dy > 0;
    player->isMoving = dx != 0 || dy != 0;

    float diagonalScale = (dx != 0 && dy != 0) ? 0.70710678f : 1.0f;
    player->x += dx * diagonalScale * player->speed * moveMultiplier * deltaTime;
    player->y += dy * diagonalScale * player->speed * moveMultiplier * deltaTime;

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

        float walkFrameTime = player->isSprinting ? 0.25f / PLAYER_SPRINT_MULTIPLIER : 0.25f;
        while (player->animTimer >= walkFrameTime)
        {
            player->animTimer -= walkFrameTime;
            player->currentFrame = (player->currentFrame + 1) % 4;
        }
    }
    else
    {
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }
}
