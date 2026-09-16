/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef STAMINA_BAR_RGB565_H
#define STAMINA_BAR_RGB565_H
#include <stdint.h>

#define STAMINA_BAR_WIDTH 4
#define STAMINA_BAR_HEIGHT 6
#define STAMINA_BAR_PIXELS 24

/* Timeline frame 1 */
static const uint16_t stamina_bar_front_stamina_bar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x07E0, 0x5269, 0x5269, 0x636E, 0x07E0, 0x4A48, 0x5B4D, 0x5B4D,
    0x07E0, 0x4208, 0x4AAA, 0x4AAA, 0x07E0, 0x41E7, 0x41E7, 0x4269, 0x39C7, 0x39C7, 0x39C7, 0x39C7,
};

/* Timeline frame 2 */
static const uint16_t stamina_bar_stamina_bar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x636E, 0x636E, 0x636E, 0x636E, 0x5B4D, 0x5B4D, 0x5B4D, 0x5B4D,
    0x4AAA, 0x4AAA, 0x4AAA, 0x4AAA, 0x4269, 0x4269, 0x4269, 0x4269, 0x39C7, 0x39C7, 0x39C7, 0x39C7,
};

/* Timeline frame 3 */
static const uint16_t stamina_bar_back_stamina_bar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x636E, 0x5269, 0x5269, 0x07E0, 0x5B4D, 0x5B4D, 0x4A48, 0x07E0,
    0x4AAA, 0x4AAA, 0x4208, 0x07E0, 0x4269, 0x41E7, 0x41E7, 0x07E0, 0x39C7, 0x39C7, 0x39C7, 0x39C7,
};

#define STAMINA_BAR_FRAMES_COUNT 3
static const uint16_t * const stamina_bar_frames[] = {
    stamina_bar_front_stamina_bar_1,
    stamina_bar_stamina_bar_1,
    stamina_bar_back_stamina_bar_1,
};
static const uint32_t stamina_bar_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t stamina_bar_frames_source_frames[] = { 1, 2, 3 };

#define STAMINA_BAR_FRONT_STAMINA_BAR_COUNT 1
static const uint16_t * const stamina_bar_front_stamina_bar[] = {
    stamina_bar_front_stamina_bar_1,
};
static const uint32_t stamina_bar_front_stamina_bar_duration_ms[] = { 100 };
static const uint32_t stamina_bar_front_stamina_bar_source_frames[] = { 1 };

#define STAMINA_BAR_STAMINA_BAR_COUNT 1
static const uint16_t * const stamina_bar_stamina_bar[] = {
    stamina_bar_stamina_bar_1,
};
static const uint32_t stamina_bar_stamina_bar_duration_ms[] = { 100 };
static const uint32_t stamina_bar_stamina_bar_source_frames[] = { 2 };

#define STAMINA_BAR_BACK_STAMINA_BAR_COUNT 1
static const uint16_t * const stamina_bar_back_stamina_bar[] = {
    stamina_bar_back_stamina_bar_1,
};
static const uint32_t stamina_bar_back_stamina_bar_duration_ms[] = { 100 };
static const uint32_t stamina_bar_back_stamina_bar_source_frames[] = { 3 };

#endif /* STAMINA_BAR_RGB565_H */
