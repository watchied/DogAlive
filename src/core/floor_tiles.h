#ifndef FLOOR_TILES_H
#define FLOOR_TILES_H
#include <SDL3/SDL.h>
#include "src/core/game_config.h"
#include "assets/sprites/map/floor.h"

#define FLOOR_PLAIN_PERCENT 60
#define FLOOR_COLUMNS ((GAME_WIDTH + MAP_OBJECT_WIDTH - 1) / MAP_OBJECT_WIDTH)
#define FLOOR_ROWS ((GAME_HEIGHT + MAP_OBJECT_HEIGHT - 1) / MAP_OBJECT_HEIGHT)

static inline uint32_t Floor_Random(uint32_t *seed)
{
    *seed ^= *seed << 13; *seed ^= *seed >> 17; *seed ^= *seed << 5;
    return *seed;
}
static inline void Floor_Generate(uint8_t tiles[FLOOR_ROWS][FLOOR_COLUMNS], uint32_t *seed)
{
    if (!*seed) *seed = 0x91A7B35Du;
    for (int y = 0; y < FLOOR_ROWS; ++y)
        for (int x = 0; x < FLOOR_COLUMNS; ++x)
            tiles[y][x] = Floor_Random(seed) % 100 < FLOOR_PLAIN_PERCENT ? 0 :
                1 + Floor_Random(seed) % (MAP_OBJECT_FLOOR_COUNT - 1);
}
static inline SDL_Texture *Floor_CreateAtlas(SDL_Renderer *renderer)
{
    SDL_Surface *s = SDL_CreateSurface(MAP_OBJECT_WIDTH * MAP_OBJECT_FLOOR_COUNT,
        MAP_OBJECT_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    for (int i = 0; i < MAP_OBJECT_FLOOR_COUNT; ++i)
        for (int y = 0; y < MAP_OBJECT_HEIGHT; ++y)
            for (int x = 0; x < MAP_OBJECT_WIDTH; ++x) {
                uint16_t c = map_object_floor[i][y * MAP_OBJECT_WIDTH + x];
                Uint8 *pixel = (Uint8 *)s->pixels + y * s->pitch + (i * MAP_OBJECT_WIDTH + x) * 4;
                pixel[0] = ((c >> 11) & 31) * 255 / 31;
                pixel[1] = ((c >> 5) & 63) * 255 / 63;
                pixel[2] = (c & 31) * 255 / 31;
                pixel[3] = c == 0x07E0 ? 0 : 255;
            }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, s);
    SDL_DestroySurface(s);
    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    return texture;
}
static inline void Floor_Draw(SDL_Renderer *renderer, SDL_Texture *atlas,
                               const uint8_t tiles[FLOOR_ROWS][FLOOR_COLUMNS])
{
    for (int y = 0; y < FLOOR_ROWS; ++y)
        for (int x = 0; x < FLOOR_COLUMNS; ++x) {
            SDL_FRect src = {tiles[y][x] * MAP_OBJECT_WIDTH, 0, MAP_OBJECT_WIDTH, MAP_OBJECT_HEIGHT};
            SDL_FRect dst = {x * MAP_OBJECT_WIDTH, y * MAP_OBJECT_HEIGHT, MAP_OBJECT_WIDTH, MAP_OBJECT_HEIGHT};
            SDL_RenderTexture(renderer, atlas, &src, &dst);
        }
}
#endif
