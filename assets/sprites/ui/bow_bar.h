/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef BOW_BAR_RGB565_H
#define BOW_BAR_RGB565_H
#include <stdint.h>

#define BOW_BAR_WIDTH 4
#define BOW_BAR_HEIGHT 5
#define BOW_BAR_PIXELS 20

/* Timeline frame 1 */
static const uint16_t bow_bar_frame_1[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x5269, 0x5269, 0x6B4C, 0x6B4C, 0x07E0, 0x07E0, 0x4A48, 0x62EA,
    0x4208, 0x4208, 0x5289, 0x5289, 0x41E7, 0x39C7, 0x39C7, 0x39C7,
};

/* Timeline frame 2 */
static const uint16_t bow_bar_frame_2[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x6B4C, 0x6B4C, 0x6B4C, 0x6B4C, 0x62EA, 0x62EA, 0x62EA, 0x62EA,
    0x5289, 0x5289, 0x5289, 0x5289, 0x39C7, 0x39C7, 0x39C7, 0x39C7,
};

/* Timeline frame 3 */
static const uint16_t bow_bar_frame_3[] = {
    0x5AAA, 0x5AAA, 0x5AAA, 0x5AAA, 0x6B4C, 0x6B4C, 0x5269, 0x5269, 0x62EA, 0x4A48, 0x07E0, 0x07E0,
    0x5289, 0x5289, 0x4208, 0x4208, 0x39C7, 0x39C7, 0x39C7, 0x41E7,
};

#define BOW_BAR_FRAMES_COUNT 3
static const uint16_t * const bow_bar_frames[] = {
    bow_bar_frame_1,
    bow_bar_frame_2,
    bow_bar_frame_3,
};
static const uint32_t bow_bar_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t bow_bar_frames_source_frames[] = { 1, 2, 3 };

#endif /* BOW_BAR_RGB565_H */
