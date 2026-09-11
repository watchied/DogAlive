#include <SDL3/SDL.h>
#include <stdbool.h>
#include "game_core.h"
#include "sprites.h"

void Draw_Sprite_Simulated(SDL_Renderer *renderer, float x, float y, int w, int h, const uint16_t *sprite, bool flipX) {
    int scale = 3;

    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            int srcX = flipX ? (w - 1 - px) : px;
            uint16_t color16 = sprite[py * w + srcX];

            if (color16 == 0x07E0) continue;

            uint8_t r = ((color16 >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((color16 >> 5)  & 0x3F) * 255 / 63;
            uint8_t b = (color16         & 0x1F) * 255 / 31;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            
            SDL_FRect rect = {
                x + (px * scale),
                y + (py * scale),
                (float)scale,
                (float)scale
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("STM32 RPG Simulator", 320 * 2, 240 * 2, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Player player;
    Game_Init(&player);

    bool running = true;
    uint64_t lastTime = SDL_GetTicks();

    while (running) {
        uint64_t currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        const bool *keyboardState = SDL_GetKeyboardState(NULL);
        
        Game_Update(&player, keyboardState, deltaTime);

        SDL_SetRenderDrawColor(renderer,0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // ดึง Pointer ของภาพตาม Index เฟรมปัจจุบันที่อัปเดตจาก Game_Update
        const uint16_t *currentSprite = undying_player_walk_right[player.currentFrame];

        // วาดตัวละครด้วยภาพเฟรมปัจจุบัน
        Draw_Sprite_Simulated(renderer, player.x, player.y, 16, 16, currentSprite, player.facingLeft);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}