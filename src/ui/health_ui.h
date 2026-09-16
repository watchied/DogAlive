#ifndef HEALTH_UI_H
#define HEALTH_UI_H

#include <SDL3/SDL.h>
#include <math.h>
#include "assets/sprites/ui/health.h"
#include "assets/sprites/ui/health_bar.h"
#include "src/player/player.h"
#include "assets/sprites/ui/stamina.h"
#include "assets/sprites/ui/stamina_bar.h"
#include "assets/sprites/ui/bow_charge.h"
#include "assets/sprites/ui/bow_bar.h"
#include "assets/sprites/ui/heart.h"

#define STAMINA_UI_WIDTH 80
#define BOW_UI_WIDTH 28

static inline void HUD_DrawPixel(SDL_Renderer *renderer, float x, float y, uint16_t color)
{
    if (color == 0x07E0) return;
    SDL_SetRenderDrawColor(renderer, ((color >> 11) & 31) * 255 / 31,
        ((color >> 5) & 63) * 255 / 63, (color & 31) * 255 / 31, 255);
    SDL_FRect pixel = {x, y, 1, 1};
    SDL_RenderFillRect(renderer, &pixel);
}

// Every 5 Max HP adds one 4-pixel middle tile, capped to fit the HUD.
#define HEALTH_UI_HP_PER_TILE 5
#define HEALTH_UI_MAX_MIDDLE_WIDTH 160

static inline int HealthUI_MiddleWidth(int maxHP)
{
    if (maxHP <= 0) return HEALTH_BAR_WIDTH;
    int tiles = maxHP / HEALTH_UI_HP_PER_TILE + (maxHP % HEALTH_UI_HP_PER_TILE != 0);
    int maxTiles = HEALTH_UI_MAX_MIDDLE_WIDTH / HEALTH_BAR_WIDTH;
    if (tiles > maxTiles) tiles = maxTiles;
    return tiles * HEALTH_BAR_WIDTH;
}

// Keep both end caps intact; repeat the middle without stretching any pixels.
static inline uint16_t HealthUI_Pixel(int x, int y, int width, int tileWidth,
                                      const uint16_t *front, const uint16_t *middle,
                                      const uint16_t *back)
{
    if (x < tileWidth) return front[y * tileWidth + x];
    if (x >= width - tileWidth) return back[y * tileWidth + x - (width - tileWidth)];
    return middle[y * tileWidth + (x - tileWidth) % tileWidth];
}

static inline SDL_FRect HealthUI_Draw(SDL_Renderer *renderer, float x, float y, const Player *p)
{
    int width = HealthUI_MiddleWidth(p->maxHP) + HEALTH_BAR_WIDTH * 2;
    float ratio = p->maxHP > 0 ? (float)p->hp / p->maxHP : 0;
    ratio = fmaxf(0, fminf(1, ratio));
    // The fill caps have two transparent outer columns on each side.
    int fillEnd = ratio > 0 ? 2 + (int)ceilf((width - 4) * ratio) : 0;
    for (int py = 0; py < HEALTH_BAR_HEIGHT; ++py) {
        for (int px = 0; px < width; ++px) {
            uint16_t color = HealthUI_Pixel(px, py, width, HEALTH_BAR_WIDTH,
                health_bar_front_empty_healthbar_1, health_bar_empty_healthbar_1,
                health_bar_back_empty_healthbar_1);
            // Fill sits one pixel below the empty-bar sprite.
            if (px < fillEnd && py >= 1 && py < 1 + HEALTH_HEIGHT) {
                uint16_t fill = HealthUI_Pixel(px, py - 1, width, HEALTH_WIDTH,
                    health_front_health_1, health_health_1, health_back_health_1);
                if (fill != 0x07E0) color = fill;
            }
            if (color == 0x07E0) continue;
            SDL_SetRenderDrawColor(renderer, ((color >> 11) & 31) * 255 / 31,
                ((color >> 5) & 63) * 255 / 63, (color & 31) * 255 / 31, 255);
            SDL_FRect pixel = {x + px, y + py, 1, 1};
            SDL_RenderFillRect(renderer, &pixel);
        }
    }
    return (SDL_FRect){x, y, (float)width, HEALTH_BAR_HEIGHT};
}

static inline SDL_FRect HUD_DrawArtBar(SDL_Renderer *renderer, float x, float y,
    int width, float ratio, const uint16_t *const *frame, int frameWidth, int frameHeight,
    const uint16_t *const *fill, int fillWidth, int fillHeight, int insetX, int emptyColumns)
{
    ratio = fmaxf(0, fminf(1, ratio));
    int innerWidth = width - insetX * 2;
    int end = ratio > 0 ? emptyColumns + (int)ceilf((innerWidth - emptyColumns * 2) * ratio) : 0;
    int insetY = (frameHeight - fillHeight) / 2;
    for (int py = 0; py < frameHeight; ++py) {
        for (int px = 0; px < width; ++px) {
            uint16_t color = HealthUI_Pixel(px, py, width, frameWidth, frame[0], frame[1], frame[2]);
            int fx = px - insetX, fy = py - insetY;
            if (fx >= 0 && fx < innerWidth && fx < end && fy >= 0 && fy < fillHeight) {
                uint16_t overlay = HealthUI_Pixel(fx, fy, innerWidth, fillWidth, fill[0], fill[1], fill[2]);
                if (overlay != 0x07E0) color = overlay;
            }
            HUD_DrawPixel(renderer, x + px, y + py, color);
        }
    }
    return (SDL_FRect){x, y, (float)width, (float)frameHeight};
}

static inline void HUD_Draw(SDL_Renderer *renderer, const Player *p)
{
    float ratio = p->maxHP > 0 ? (float)p->hp / p->maxHP : 0;
    int heartFrame = ratio <= 0 ? 4 : ratio <= 0.25f ? 3 : ratio <= 0.5f ? 2 : ratio <= 0.75f ? 1 : 0;
    SDL_FRect hp = HealthUI_Draw(renderer, 4 + HEART_WIDTH + 2,
        4 + (HEART_HEIGHT - HEALTH_BAR_HEIGHT) / 2, p);
    for (int y = 0; y < HEART_HEIGHT; ++y)
        for (int x = 0; x < HEART_WIDTH; ++x)
            HUD_DrawPixel(renderer, 4 + x, 4 + y, heart_frames[heartFrame][y * HEART_WIDTH + x]);

    SDL_FRect stamina = HUD_DrawArtBar(renderer, hp.x, hp.y + hp.h + 2,
        STAMINA_UI_WIDTH, p->maxStamina > 0 ? p->stamina / p->maxStamina : 0,
        stamina_bar_frames, STAMINA_BAR_WIDTH, STAMINA_BAR_HEIGHT,
        stamina_frames, STAMINA_WIDTH, STAMINA_HEIGHT, 0, 2);
    float ready = p->shootCooldown > 0 ? 1 - p->shootTimer / p->shootCooldown : 1;
    HUD_DrawArtBar(renderer, hp.x, stamina.y + stamina.h + 2, BOW_UI_WIDTH, ready,
        bow_bar_frames, BOW_BAR_WIDTH, BOW_BAR_HEIGHT,
        bow_charge_frames, BOW_CHARGE_WIDTH, BOW_CHARGE_HEIGHT, 1, 0);
}
#endif
