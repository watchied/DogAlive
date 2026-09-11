#include "game_core.h"
#include <SDL3/SDL.h>

void Game_Init(Player *player) {
    player->x = 100.0f;
    player->y = 100.0f;
    player->speed = 120.0f;
    player->facingLeft = false;
    player->direction = PLAYER_RIGHT;
    
    player->currentFrame = 0;
    player->animTimer = 0.0f;
    player->isMoving = false;
    player->isMovingRight = false;
    player->isMovingLeft = false;
    player->isMovingUp = false;
    player->isMovingDown = false;
}

void Game_Update(Player *player, const bool *keyboardState, float deltaTime) {
    if (keyboardState == NULL) return;

    int dx = (keyboardState[SDL_SCANCODE_D] || keyboardState[SDL_SCANCODE_RIGHT])
           - (keyboardState[SDL_SCANCODE_A] || keyboardState[SDL_SCANCODE_LEFT]);
    int dy = (keyboardState[SDL_SCANCODE_S] || keyboardState[SDL_SCANCODE_DOWN])
           - (keyboardState[SDL_SCANCODE_W] || keyboardState[SDL_SCANCODE_UP]);
    PlayerDirection previousDirection = player->direction;

    player->isMovingRight = dx > 0;
    player->isMovingLeft = dx < 0;
    player->isMovingUp = dy < 0;
    player->isMovingDown = dy > 0;
    player->isMoving = dx != 0 || dy != 0;
    player->x += dx * player->speed * deltaTime;
    player->y += dy * player->speed * deltaTime;

    // Vertical movement takes priority for diagonal walking. Up shows the back.
    if (dy < 0) player->direction = PLAYER_UP;
    else if (dy > 0) player->direction = PLAYER_DOWN;
    else if (dx < 0) player->direction = PLAYER_LEFT;
    else if (dx > 0) player->direction = PLAYER_RIGHT;
    player->facingLeft = player->direction == PLAYER_LEFT;

    if (player->direction != previousDirection) {
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }

    if (player->isMoving) {
        player->animTimer += deltaTime;
        // Keep the existing walking speed of four animation frames per second.
        while (player->animTimer >= 0.25f) {
            player->animTimer -= 0.25f;
            player->currentFrame = (player->currentFrame + 1) % 4;
        }
    } else {
        // Keep the last facing direction when standing still.
        player->currentFrame = 0;
        player->animTimer = 0.0f;
    }
}
