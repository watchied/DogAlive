#ifndef FLOOR_TILES_H
#define FLOOR_TILES_H
#include <SDL3/SDL.h>
#include "src/core/game_config.h"
#include "assets/sprites/map/map_ground.h"

#define FLOOR_PLAIN_PERCENT 60
// Night tint for dirt/grass floors only. 255 = original brightness per channel.
#define FLOOR_NIGHT_ENABLED 1
#define FLOOR_NIGHT_RED 85
#define FLOOR_NIGHT_GREEN 110
#define FLOOR_NIGHT_BLUE 170
#define FLOOR_ATLAS_COUNT (MAP_OBJECT_FLOOR_COUNT+MAP_OBJECT_DIRT_COUNT+MAP_OBJECT_GLASS_COUNT)
static inline const uint16_t *Floor_Pixels(int i) {
    if(i<MAP_OBJECT_FLOOR_COUNT) return map_object_floor[i];
    i-=MAP_OBJECT_FLOOR_COUNT;
    if(i<MAP_OBJECT_DIRT_COUNT) return map_object_dirt[i];
    return map_object_glass[i-MAP_OBJECT_DIRT_COUNT];
}
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
static inline void Floor_GenerateDirt(uint8_t tiles[FLOOR_ROWS][FLOOR_COLUMNS],uint32_t *seed) {
    if(!*seed) *seed=0x91A7B35Du;
    for(int y=0;y<FLOOR_ROWS;++y) for(int x=0;x<FLOOR_COLUMNS;++x) {
        bool border=x==0 || y==0 || x==FLOOR_COLUMNS-1 || y==FLOOR_ROWS-1;
        uint32_t grass=(x==0 || x==FLOOR_COLUMNS-1) && (y==0 || y==FLOOR_ROWS-1)?4:
            y==0?0:y==FLOOR_ROWS-1?1:x==0?2:3;
        tiles[y][x]=border?MAP_OBJECT_FLOOR_COUNT+MAP_OBJECT_DIRT_COUNT+grass:
            MAP_OBJECT_FLOOR_COUNT+(Floor_Random(seed)%100<FLOOR_PLAIN_PERCENT?0:1+Floor_Random(seed)%(MAP_OBJECT_DIRT_COUNT-1));
    }
}
static inline SDL_Texture *Floor_CreateAtlas(SDL_Renderer *renderer)
{
    SDL_Surface *s = SDL_CreateSurface(MAP_OBJECT_WIDTH * FLOOR_ATLAS_COUNT,
        MAP_OBJECT_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    if (!s) return NULL;
    for (int i = 0; i < FLOOR_ATLAS_COUNT; ++i)
        for (int y = 0; y < MAP_OBJECT_HEIGHT; ++y)
            for (int x = 0; x < MAP_OBJECT_WIDTH; ++x) {
                uint16_t c = Floor_Pixels(i)[y * MAP_OBJECT_WIDTH + x];
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
            double angle=0;
            if(tiles[y][x]==MAP_OBJECT_FLOOR_COUNT+MAP_OBJECT_DIRT_COUNT+4)
                angle=y==0?(x==0?0:90):(x==0?270:180);
            SDL_RenderTextureRotated(renderer,atlas,&src,&dst,angle,NULL,SDL_FLIP_NONE);
        }
}
#endif
