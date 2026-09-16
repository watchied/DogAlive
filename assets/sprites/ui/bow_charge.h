/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef BOW_CHARGE_RGB565_H
#define BOW_CHARGE_RGB565_H
#include <stdint.h>

#define BOW_CHARGE_WIDTH 3
#define BOW_CHARGE_HEIGHT 3
#define BOW_CHARGE_PIXELS 9

/* Timeline frame 1 */
static const uint16_t bow_charge_frame_1[] = {
    0x07E0, 0x7B05, 0x7B05, 0x07E0, 0x07E0, 0x6284, 0x07E0, 0x5204, 0x5204,
};

/* Timeline frame 2 */
static const uint16_t bow_charge_frame_2[] = {
    0x7B05, 0x7B05, 0x7B05, 0x6284, 0x6284, 0x6284, 0x5204, 0x5204, 0x5204,
};

/* Timeline frame 3 */
static const uint16_t bow_charge_frame_3[] = {
    0x7B05, 0x7B05, 0x07E0, 0x6284, 0x07E0, 0x07E0, 0x5204, 0x5204, 0x07E0,
};

#define BOW_CHARGE_FRAMES_COUNT 3
static const uint16_t * const bow_charge_frames[] = {
    bow_charge_frame_1,
    bow_charge_frame_2,
    bow_charge_frame_3,
};
static const uint32_t bow_charge_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t bow_charge_frames_source_frames[] = { 1, 2, 3 };

#endif /* BOW_CHARGE_RGB565_H */
