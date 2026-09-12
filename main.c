#include <SDL3/SDL.h>
#include <stdbool.h>
#include "game_core.h"
#include "player_sprites.h"
#include "eye_parasite_sprites.h"
#include "flesh_slime_sprites.h"
#include "bloodless_ghoul_sprites.h"
#include "bloodless_ghoul.h"

void Draw_Sprite_Simulated(SDL_Renderer *renderer, float x, float y, int w, int h, const uint16_t *sprite, bool flipX, bool flashWhite)
{
    int scale = 3;

    for (int py = 0; py < h; py++)
    {
        for (int px = 0; px < w; px++)
        {
            int srcX = flipX ? (w - 1 - px) : px;
            uint16_t color16 = sprite[py * w + srcX];

            if (color16 == 0x07E0)
                continue;

            uint8_t r = ((color16 >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((color16 >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (color16 & 0x1F) * 255 / 31;

            if (flashWhite)
                r = g = b = 255;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);

            SDL_FRect rect = {
                x + (px * scale),
                y + (py * scale),
                (float)scale,
                (float)scale};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("STM32 RPG Simulator", 320 * 2, 240 * 2, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Player player;
    Game_Init(&player);
    Ghoul ghoul;
    Ghoul_Init(&ghoul, 420.0f, 250.0f);

    bool running = true;
    uint64_t lastTime = SDL_GetTicks();

    while (running)
    {
        uint64_t currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (deltaTime > 0.05f)
            deltaTime = 0.05f;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN &&
                event.key.scancode == SDL_SCANCODE_R && !event.key.repeat)
            {
                Game_Init(&player);
                Ghoul_Init(&ghoul, 420.0f, 250.0f);
            }
        }

        const bool *keyboardState = SDL_GetKeyboardState(NULL);

        if (player.hp > 0)
        {
            Game_Update(&player, keyboardState, deltaTime);
            Ghoul_CheckPlayerAttack(&ghoul, &player);
        }
        Ghoul_Update(&ghoul, &player, deltaTime);
        if (player.hp == 0)
        {
            player.isMoving = false;
            player.isAttacking = false;
            player.currentFrame = 0;
        }
        SDL_SetWindowTitle(window, player.hp > 0
                                       ? "Ghoul demo | WASD: move | Space: attack | R: restart"
                                       : "Game over | R: restart");

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // ดึง Pointer ของภาพตาม Index เฟรมปัจจุบันที่อัปเดตจาก Game_Update
        const uint16_t *const *walkSprites;
        const uint16_t *const *attackSprites;
        const uint16_t *idleSprite;

        switch (player.direction)
        {
        case PLAYER_UP:
            walkSprites = undying_player_walk_back;
            attackSprites = undying_player_melee_back;
            idleSprite = undying_player_idle_back[0];
            break;

        case PLAYER_DOWN:
            walkSprites = undying_player_walk_front;
            attackSprites = undying_player_melee_front;
            idleSprite = undying_player_idle_front[0];
            break;

        case PLAYER_LEFT:
            walkSprites = undying_player_walk_left;
            attackSprites = undying_player_melee_left;
            idleSprite = undying_player_idle_left[0];
            break;

        default:
            walkSprites = undying_player_walk_right;
            attackSprites = undying_player_melee_right;
            idleSprite = undying_player_idle_right[0];
            break;
        }

        const uint16_t *currentSprite;

        if (player.isAttacking)
        {
            currentSprite = attackSprites[player.currentFrame];
        }
        else if (player.isMoving)
        {
            currentSprite = walkSprites[player.currentFrame];
        }
        else
        {
            currentSprite = idleSprite;
        }

        Draw_Sprite_Simulated(
            renderer,
            player.x,
            player.y,
            16,
            16,
            currentSprite,
            false, false);

        if (!ghoul.deathFinished)
        {
            Draw_Sprite_Simulated(
                renderer, ghoul.x, ghoul.y,
                BLOODLESS_GHOUL_MELEE_ENEMY_WIDTH,
                BLOODLESS_GHOUL_MELEE_ENEMY_HEIGHT,
                Ghoul_GetSprite(&ghoul),
                ghoul.direction == PLAYER_LEFT,
                ghoul.hitFlashTimer > 0.0f);
        }

        SDL_FRect hpBackground = {12, 12, 200, 12};
        SDL_FRect hpBar = {12, 12, hpBackground.w * player.hp / player.maxHP, 12};
        SDL_SetRenderDrawColor(renderer, 70, 20, 20, 255);
        SDL_RenderFillRect(renderer, &hpBackground);
        SDL_SetRenderDrawColor(renderer, 40, 210, 70, 255);
        SDL_RenderFillRect(renderer, &hpBar);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
