#include <SDL3/SDL.h>
#include <stdbool.h>
#include "src/core/game_core.h"
#include "assets/sprites/player/player_sprites.h"
#include "assets/sprites/enemies/eye_parasite_sprites.h"
#include "assets/sprites/enemies/flesh_slime_sprites.h"
#include "assets/sprites/enemies/bloodless_ghoul_sprites.h"
#include "src/enemies/bloodless_ghoul.h"
#include "assets/sprites/projectiles/arrow_sprites.h"
#include "src/combat/projectile.h"
#include "src/effects/running_effect.h"
#include "src/combat/enemy_group.h"
#include "src/ui/health_ui.h"
#include "src/core/stages.h"
#include "src/player/bow_input.h"

typedef enum {
    GAME_MENU,      // หน้าก่อนเริ่มเกม
    GAME_PLAYING,   // กำลังเล่น
    GAME_PAUSED,    // หยุดเกม
    GAME_VICTORY,
    GAME_OVER      // ผู้เล่นตาย
} GameState;

// Placeholder UI: replace this function with your own menu/pause/game-over art.
static void Draw_GameUI(SDL_Renderer *renderer, GameState state)
{
    if (state == GAME_PLAYING) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, state == GAME_MENU ? 255 : 190);
    SDL_FRect screen = {0, 0, GAME_WIDTH, GAME_HEIGHT};
    SDL_RenderFillRect(renderer, &screen);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const char *title = state == GAME_MENU ? "DOG ALIVE" :
                        state == GAME_PAUSED ? "PAUSED" : state == GAME_VICTORY ? "DUNGEON CLEARED" : "GAME OVER";
    const char *action = state == GAME_MENU ? "Enter: Start" :
                         state == GAME_PAUSED ? "Esc: Resume" : "R: Restart";
    SDL_RenderDebugText(renderer, (GAME_WIDTH - strlen(title) * 8.0f) / 2, 88, title);
    SDL_RenderDebugText(renderer, (GAME_WIDTH - strlen(action) * 8.0f) / 2, 112, action);
    if (state == GAME_MENU) {
        SDL_RenderDebugText(renderer, 32, 200, "5 STAGES - CLEAR TO EXIT");
        SDL_RenderDebugText(renderer, 48, 144, "WASD / Arrows: Move");
        SDL_RenderDebugText(renderer, 48, 160, "Space: Slash  J: Shoot");
        SDL_RenderDebugText(renderer, 16, 176, "Tap J:Shot  Hold J:Arrow type");
    } else {
        SDL_RenderDebugText(renderer, 104, 136, "M: Main menu");
    }
}

static void Reset_Game(StageProgress *stage, Player *p, EnemyGroup *enemies,
                       Projectile *arrows, SlimeShot *shots, RunningEffect *effect)
{
    Game_Init(p);
    *stage = (StageProgress){0};
    uint32_t seed = (uint32_t)SDL_GetPerformanceCounter();
    for (int i = 0; i < STAGE_COUNT; ++i) Floor_Generate(stage->floors[i], &seed);
    Stage_Load(0, p, enemies, arrows, shots, effect);
}
void Draw_Sprite_Simulated(SDL_Renderer *renderer, float x, float y, int w, int h, const uint16_t *sprite, bool flipX, bool flashWhite, bool flipY, bool flashPink)
{

    for (int py = 0; py < h; py++)
    {
        for (int px = 0; px < w; px++)
        {
            int srcX = flipX ? (w - 1 - px) : px;
            int srcY = flipY ? (h - 1 - py) : py;
            uint16_t color16 = sprite[srcY * w + srcX];

            if (color16 == 0x07E0)
                continue;

            uint8_t r = ((color16 >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((color16 >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (color16 & 0x1F) * 255 / 31;

            if (flashWhite)
                r = g = b = 255;

            if (flashPink) {
                r = 255;
                g = 190;
                b = 190;
            }
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);

            SDL_FRect rect = {
                x + px,
                y + py,
                1.0f,
                1.0f};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

static SDL_Texture *CreateArrowTexture(SDL_Renderer *renderer, const uint16_t *sprite)
{
    SDL_Surface *surface = SDL_CreateSurface(
        ARROW_PROJECTILE_WIDTH, ARROW_PROJECTILE_HEIGHT, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
        return NULL;
    for (int y = 0; y < surface->h; ++y) {
        Uint8 *row = (Uint8 *)surface->pixels + y * surface->pitch;
        for (int x = 0; x < surface->w; ++x) {
            uint16_t color = sprite[y * surface->w + x];
            Uint8 *pixel = row + x * 4;
            pixel[0] = ((color >> 11) & 31) * 255 / 31;
            pixel[1] = ((color >> 5) & 63) * 255 / 63;
            pixel[2] = (color & 31) * 255 / 31;
            pixel[3] = color == 0x07E0 ? 0 : 255;
        }
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    }
    return texture;
}

static void Draw_Projectiles(SDL_Renderer *renderer, SDL_Texture *const *textures,
                             const Projectile *shots)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        const Projectile *b = &shots[i];
        if (!b->active)
            continue;
        double angle = atan2(b->vy, b->vx) * 180.0 / 3.14159265358979323846;
        // The shaft is on source row 7: rotate around its center, not the canvas center.
        SDL_FPoint pivot = {8.0f, 7.5f};
        SDL_FRect destination = {b->x - pivot.x, b->y - pivot.y,
            ARROW_PROJECTILE_WIDTH, ARROW_PROJECTILE_HEIGHT};
        SDL_Texture *texture = textures[b->type];
        SDL_RenderTextureRotated(renderer, texture, NULL, &destination,
                                 angle, &pivot, SDL_FLIP_NONE);
    }

}

static void Draw_ArrowEffects(SDL_Renderer *renderer, EnemyGroup *g)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 150, 35, 140);
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        const ArrowExplosion *e = &g->explosions[i];
        if (e->timer <= 0) continue;
        for (int row = -(int)EXPLOSIVE_ARROW_RADIUS; row < (int)EXPLOSIVE_ARROW_RADIUS; ++row) {
            float dy = row + 0.5f;
            float half = sqrtf(EXPLOSIVE_ARROW_RADIUS * EXPLOSIVE_ARROW_RADIUS - dy * dy);
            SDL_FRect span = {e->x - half, e->y + row, half * 2, 1};
            SDL_RenderFillRect(renderer, &span);
        }
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    EnemyTarget targets[ENEMY_TYPE_CAPACITY * 3];
    int n = EnemyGroup_Targets(g, targets);
    for (int i = 0; i < n; ++i) {
        EnemyTarget t = targets[i];
        if (t.hp <= 0 || g->burns[t.type][t.index].ticksLeft <= 0) continue;
        SDL_SetRenderDrawColor(renderer, 255, 115, 25, 255);
        for (int k = 0; k < 3; ++k) {
            float height = 3 + ((int)(g->burns[t.type][t.index].timer * 20) + k) % 3;
            SDL_FRect flame = {t.body.x + 3 + k * 4, t.body.y - height, 2, height};
            SDL_RenderFillRect(renderer, &flame);
        }
    }
}

static void Update_PlayerDeath(Player *p, float dt)
{
    if (p->hp > 0 || p->deathFinished)
        return;
    if (!p->deathStarted) {
        p->deathStarted = true;
        p->deathFrame = 0;
        p->deathTimer = 0.0f;
        p->isMoving = p->isSprinting = p->isAttacking = p->isShooting = false;
        p->isMovingUp = p->isMovingDown = p->isMovingLeft = p->isMovingRight = false;
        return; // Show the first death frame before advancing its timer.
    }
    p->deathTimer += dt;
    while (p->deathTimer >= undying_player_dead_duration_ms[p->deathFrame] / 1000.0f) {
        p->deathTimer -= undying_player_dead_duration_ms[p->deathFrame] / 1000.0f;
        if (p->deathFrame + 1 >= UNDYING_PLAYER_DEAD_COUNT) {
            p->deathFinished = true;
            p->deathTimer = 0.0f;
            break;
        }
        p->deathFrame++;
    }
}

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("STM32 RPG Simulator", GAME_WIDTH * PC_WINDOW_SCALE, GAME_HEIGHT * PC_WINDOW_SCALE, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = window ? SDL_CreateRenderer(window, NULL) : NULL;
    if (renderer && !SDL_SetRenderLogicalPresentation(renderer, GAME_WIDTH, GAME_HEIGHT,
                                                      SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        SDL_Log("Could not set logical display: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *arrowTexture[3] = {0};
    if (renderer) {
        arrowTexture[0] = CreateArrowTexture(renderer, arrow_projectile_arrow[0]);
        arrowTexture[1] = CreateArrowTexture(renderer, arrow_projectile_fire_arrow[0]);
        arrowTexture[2] = CreateArrowTexture(renderer, arrow_projectile_bomb_arrow[0]);
    }
    if (!arrowTexture[0] || !arrowTexture[1] || !arrowTexture[2]) {
        for (int i = 0; i < 3; ++i) SDL_DestroyTexture(arrowTexture[i]);
        SDL_Log("Could not create arrow renderer/texture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *floorAtlas = Floor_CreateAtlas(renderer);
    if (!floorAtlas) {
        SDL_Log("Could not create floor atlas: %s", SDL_GetError());
        for (int i = 0; i < 3; ++i) SDL_DestroyTexture(arrowTexture[i]);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return 1;
    }
    Projectile projectiles[MAX_PROJECTILES] = {0};
    RunningEffect runningEffect = {0};

    Player player;
    EnemyGroup enemies;
    StageProgress stage;
    BowInput bowInput = {0};
    SlimeShot slimeShots[SLIME_SHOT_CAPACITY] = {0};
    Reset_Game(&stage, &player, &enemies, projectiles, slimeShots, &runningEffect);
    GameState gameState = GAME_MENU;

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
        bool shootRequested = false;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
                bowInput = (BowInput){0};
                shootRequested = false;
            }
            if (event.type == SDL_EVENT_KEY_UP && event.key.scancode == SDL_SCANCODE_J &&
                gameState == GAME_PLAYING && player.hp > 0) {
                uint64_t time = event.key.timestamp ? event.key.timestamp / 1000000 : SDL_GetTicks();
                shootRequested |= BowInput_Release(&bowInput, &player, time);
            }
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
            {
                SDL_Scancode key = event.key.scancode;
                if (gameState == GAME_PLAYING && player.hp > 0 && key == SDL_SCANCODE_J)
                    BowInput_Press(&bowInput, event.key.timestamp ? event.key.timestamp / 1000000 : SDL_GetTicks());
                if ((gameState == GAME_MENU && key == SDL_SCANCODE_RETURN) ||
                    (gameState != GAME_MENU && key == SDL_SCANCODE_R)) {
                    Reset_Game(&stage, &player, &enemies, projectiles, slimeShots, &runningEffect);
                    bowInput = (BowInput){0}; shootRequested = false;
                    gameState = GAME_PLAYING;
                    deltaTime = 0;
                } else if (key == SDL_SCANCODE_ESCAPE) {
                    bowInput = (BowInput){0}; shootRequested = false;
                    if (gameState == GAME_PLAYING) gameState = GAME_PAUSED;
                    else if (gameState == GAME_PAUSED) {
                        gameState = GAME_PLAYING;
                        deltaTime = 0;
                    }
                } else if (key == SDL_SCANCODE_M &&
                           (gameState == GAME_PAUSED || gameState == GAME_OVER || gameState == GAME_VICTORY)) {
                    gameState = GAME_MENU;
                }
            }
        }

        const bool *keyboardState = SDL_GetKeyboardState(NULL);
        if (gameState != GAME_PLAYING || player.hp <= 0 || !(SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) {
            bowInput = (BowInput){0}; shootRequested = false;
        } else BowInput_Update(&bowInput, &player, SDL_GetTicks());

        // Freeze all simulation timers and actors while a menu is open.
        if (gameState == GAME_PLAYING) {
            player.collisionGraceTimer = fmaxf(0.0f, player.collisionGraceTimer - deltaTime);
            player.invincibilityTimer = fmaxf(0.0f, player.invincibilityTimer - deltaTime);
            player.hitFlashTimer = fmaxf(0.0f, player.hitFlashTimer - deltaTime);
            if (player.hp > 0)
            {
                Game_Update(&player, keyboardState, deltaTime);
                Stage_ClampPlayer(&player);
                SlimeShots_CheckPlayerAttack(slimeShots, &player);
                EnemyGroup_Melee(&enemies, &player);
                Projectiles_Recharge(&player, deltaTime);
                EnemyGroup_UpdateArrowEffects(&enemies, deltaTime);
                EnemyGroup_Arrows(&enemies, projectiles, deltaTime);
                Projectiles_UpdateShooting(projectiles, &player, deltaTime);
                if ((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) &&
                    shootRequested) {
                    Projectiles_Shoot(projectiles, &player);
                }
            }
            EnemyGroup_Update(&enemies, &player, slimeShots, deltaTime);
            if (player.hp == 0)
            {
                Projectiles_Reset(projectiles);
                memset(slimeShots, 0, sizeof(slimeShots));
                player.isMoving = false;
                player.isSprinting = false;
                player.isAttacking = false;
                player.isShooting = false;
                Update_PlayerDeath(&player, deltaTime);
            }
            RunningEffect_Update(&runningEffect, &player, deltaTime);
            int previousStage = stage.index;
            Stage_Update(&stage, &player, &enemies, projectiles, slimeShots, &runningEffect);
            if (stage.index != previousStage) bowInput = (BowInput){0};
            if (stage.completed) gameState = GAME_VICTORY;
            if (player.deathFinished) gameState = GAME_OVER;
        }
        SDL_SetWindowTitle(window, gameState == GAME_MENU ? "Dog Alive | Enter: Start" :
            gameState == GAME_PAUSED ? "Paused | Esc: Resume | R: Restart | M: Main menu" :
            gameState == GAME_VICTORY ? "Dungeon cleared | R: Restart | M: Main menu" :
            gameState == GAME_OVER ? "Game over | R: Restart | M: Main menu" :
            "WASD/Arrows: move/aim | Space: slash | Tap J: shoot | Hold J: arrow type | Esc: pause | R: restart");

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (gameState != GAME_MENU) {
            Stage_Draw(renderer, floorAtlas, &stage, &enemies);
            for (int i = 0; i < RUNNING_EFFECT_CAPACITY; ++i) {
                const RunningPuff *puff = &runningEffect.puffs[i];
                if (puff->active)
                    Draw_Sprite_Simulated(renderer, puff->x, puff->y,
                        RUNNING_WIDTH, RUNNING_HEIGHT, RunningEffect_GetSprite(puff),
                        puff->flipX, false, puff->flipY, false);
            }

            // ดึง Pointer ของภาพตาม Index เฟรมปัจจุบันที่อัปเดตจาก Game_Update
            const uint16_t *const *walkSprites;
            const uint16_t *const *attackSprites;
            const uint16_t *const *shootSprites;
            const uint16_t *idleSprite;

            switch (player.direction)
            {
            case PLAYER_UP:
                walkSprites = undying_player_walk_back;
                attackSprites = undying_player_melee_back;
                shootSprites = undying_player_range_back;
                idleSprite = undying_player_idle_back[0];
                break;

            case PLAYER_DOWN:
                walkSprites = undying_player_walk_front;
                attackSprites = undying_player_melee_front;
                shootSprites = undying_player_range_front;
                idleSprite = undying_player_idle_front[0];
                break;

            case PLAYER_LEFT:
                walkSprites = undying_player_walk_left;
                attackSprites = undying_player_melee_left;
                shootSprites = undying_player_range_left;
                idleSprite = undying_player_idle_left[0];
                break;

            default:
                walkSprites = undying_player_walk_right;
                attackSprites = undying_player_melee_right;
                shootSprites = undying_player_range_right;
                idleSprite = undying_player_idle_right[0];
                break;
            }

            const uint16_t *currentSprite;

            if (player.hp <= 0)
            {
                currentSprite = undying_player_dead[player.deathFrame];
            }
            else if (player.isShooting)
            {
                currentSprite = shootSprites[player.currentFrame];
            }
            else if (player.isAttacking)
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
                false, false, false, player.hitFlashTimer > 0.0f);

            for (int enemyIndex = 0; enemyIndex < enemies.ghoulCount; ++enemyIndex) {
            const Ghoul ghoul = enemies.ghouls[enemyIndex];
            if (!ghoul.deathFinished)
            {
                Draw_Sprite_Simulated(
                    renderer, ghoul.x, ghoul.y,
                    BLOODLESS_GHOUL_MELEE_ENEMY_WIDTH,
                    BLOODLESS_GHOUL_MELEE_ENEMY_HEIGHT,
                    Ghoul_GetSprite(&ghoul),
                    ghoul.direction == PLAYER_LEFT,
                    ghoul.hitFlashTimer > 0.0f, false, false);
            }

            }
            for (int enemyIndex = 0; enemyIndex < enemies.slimeCount; ++enemyIndex) {
            const FleshSlime slime = enemies.slimes[enemyIndex];
            if (!slime.deathFinished)
                Draw_Sprite_Simulated(renderer, slime.x, slime.y,
                    FLESH_SLIME_RANGE_ENEMY_WIDTH, FLESH_SLIME_RANGE_ENEMY_HEIGHT,
                    FleshSlime_GetSprite(&slime), slime.direction == PLAYER_LEFT,
                    slime.hitFlashTimer > 0, false, false);
            }
            for (int enemyIndex = 0; enemyIndex < enemies.eyeCount; ++enemyIndex) {
            const EyeParasite eye = enemies.eyes[enemyIndex];
            if (eye.state == EYE_PARASITE_ATTACK || eye.explosionTimer > 0) {
                float cx = eye.x + ACTOR_HALF_SIZE, cy = eye.y + ACTOR_HALF_SIZE;
                float radius = EYE_PARASITE_BLAST_RADIUS;
                if (eye.explosionTimer > 0) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 255, 100, 40, 150);
                    for (int row = -(int)radius; row < (int)radius; ++row) {
                        float dy = row + 0.5f;
                        float halfWidth = sqrtf(radius * radius - dy * dy);
                        SDL_FRect span = {cx - halfWidth, cy + row, halfWidth * 2, 1};
                        SDL_RenderFillRect(renderer, &span);
                    }
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                }
                SDL_SetRenderDrawColor(renderer, 255, 180, 60, 255);
                SDL_FPoint ring[65];
                for (int i = 0; i <= 64; ++i) {
                    float angle = i * 6.28318530718f / 64;
                    ring[i] = (SDL_FPoint){cx + cosf(angle) * radius, cy + sinf(angle) * radius};
                }
                SDL_RenderLines(renderer, ring, 65);
            }
            if (!eye.deathFinished)
                Draw_Sprite_Simulated(renderer, eye.x, eye.y,
                    EYE_PARASITE_BOMB_ENEMY_WIDTH, EYE_PARASITE_BOMB_ENEMY_HEIGHT,
                    EyeParasite_GetSprite(&eye), eye.direction == PLAYER_LEFT,
                    eye.hitFlashTimer > 0, false, false);
            }
            for (int i = 0; i < SLIME_SHOT_CAPACITY; ++i) {
                const SlimeShot *s = &slimeShots[i];
                if (s->active)
                    // The visible bullet is centered at (8, 10) in its 16x16 canvas.
                    Draw_Sprite_Simulated(renderer, s->x - 8, s->y - 10,
                        SLIME_BULLET_WIDTH, SLIME_BULLET_HEIGHT, slime_bullet_frame_1,
                        false, s->hitFlashTimer > 0, false, false);
            }
            Draw_Projectiles(renderer, arrowTexture, projectiles);
            Draw_ArrowEffects(renderer, &enemies);

            HUD_Draw(renderer, &player);
        }
        Draw_GameUI(renderer, gameState);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(floorAtlas);
    for (int i = 0; i < 3; ++i) SDL_DestroyTexture(arrowTexture[i]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
