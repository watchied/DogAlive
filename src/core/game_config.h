#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// World coordinates match the STM32 display. Only the PC window is enlarged.
#define GAME_WIDTH 320
#define GAME_HEIGHT 240
#define PC_WINDOW_SCALE 4
#define ACTOR_SIZE 16.0f
#define ACTOR_HALF_SIZE (ACTOR_SIZE / 2.0f)
#define MELEE_REACH (32.0f / 3.0f)

// Number of enemies per type (0 disables that type). Rebuild after changing.
#define GHOUL_COUNT 0
#define FLESH_SLIME_COUNT 8
#define EYE_PARASITE_COUNT 0
#define ENEMY_TYPE_CAPACITY 10
#define ENEMY_SPACING 26.0f // Preferred distance between enemy centers, in pixels.
#define ENEMY_SEPARATION_SPEED 35.0f // Maximum extra movement to avoid neighbors.
#if GHOUL_COUNT < 0 || GHOUL_COUNT > ENEMY_TYPE_CAPACITY || FLESH_SLIME_COUNT < 0 || FLESH_SLIME_COUNT > ENEMY_TYPE_CAPACITY || EYE_PARASITE_COUNT < 0 || EYE_PARASITE_COUNT > ENEMY_TYPE_CAPACITY
#error Enemy counts must be between 0 and ENEMY_TYPE_CAPACITY
#endif

#endif
