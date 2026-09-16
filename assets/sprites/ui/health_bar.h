/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef HEALTH_BAR_RGB565_H
#define HEALTH_BAR_RGB565_H
#include <stdint.h>

#define HEALTH_BAR_WIDTH 4
#define HEALTH_BAR_HEIGHT 8
#define HEALTH_BAR_PIXELS 32

/* Timeline frame 1 */
static const uint16_t health_bar_front_empty_healthbar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x07E0, 0x528A, 0x528A, 0x6ACB, 0x07E0, 0x4A28, 0x6ACB, 0x62AA,
    0x07E0, 0x4208, 0x62AA, 0x5228, 0x07E0, 0x41E7, 0x5228, 0x5208, 0x07E0, 0x39C7, 0x5208, 0x4A08,
    0x07E0, 0x39C7, 0x39C7, 0x4A07, 0x39A6, 0x39A6, 0x39A6, 0x39A6,
};

/* Timeline frame 2 */
static const uint16_t health_bar_empty_healthbar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x6ACB, 0x6ACB, 0x6ACB, 0x6ACB, 0x62AA, 0x62AA, 0x62AA, 0x62AA,
    0x5228, 0x5228, 0x5228, 0x5228, 0x5208, 0x5208, 0x5208, 0x5208, 0x4A08, 0x4A08, 0x4A08, 0x4A08,
    0x4A07, 0x4A07, 0x4A07, 0x4A07, 0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA,
};

/* Timeline frame 3 */
static const uint16_t health_bar_back_empty_healthbar_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x6ACB, 0x528A, 0x528A, 0x07E0, 0x62AA, 0x6ACB, 0x4A28, 0x07E0,
    0x5228, 0x62AA, 0x4208, 0x07E0, 0x5208, 0x5228, 0x41E7, 0x07E0, 0x4A08, 0x5208, 0x39C7, 0x07E0,
    0x4A07, 0x39C7, 0x39C7, 0x07E0, 0x39A6, 0x39A6, 0x39A6, 0x39A6,
};

#define HEALTH_BAR_FRAMES_COUNT 3
static const uint16_t * const health_bar_frames[] = {
    health_bar_front_empty_healthbar_1,
    health_bar_empty_healthbar_1,
    health_bar_back_empty_healthbar_1,
};
static const uint32_t health_bar_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t health_bar_frames_source_frames[] = { 1, 2, 3 };

#define HEALTH_BAR_FRONT_EMPTY_HEALTHBAR_COUNT 1
static const uint16_t * const health_bar_front_empty_healthbar[] = {
    health_bar_front_empty_healthbar_1,
};
static const uint32_t health_bar_front_empty_healthbar_duration_ms[] = { 100 };
static const uint32_t health_bar_front_empty_healthbar_source_frames[] = { 1 };

#define HEALTH_BAR_EMPTY_HEALTHBAR_COUNT 1
static const uint16_t * const health_bar_empty_healthbar[] = {
    health_bar_empty_healthbar_1,
};
static const uint32_t health_bar_empty_healthbar_duration_ms[] = { 100 };
static const uint32_t health_bar_empty_healthbar_source_frames[] = { 2 };

#define HEALTH_BAR_BACK_EMPTY_HEALTHBAR_COUNT 1
static const uint16_t * const health_bar_back_empty_healthbar[] = {
    health_bar_back_empty_healthbar_1,
};
static const uint32_t health_bar_back_empty_healthbar_duration_ms[] = { 100 };
static const uint32_t health_bar_back_empty_healthbar_source_frames[] = { 3 };

#endif /* HEALTH_BAR_RGB565_H */
