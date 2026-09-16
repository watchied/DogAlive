/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef HEALTH_RGB565_H
#define HEALTH_RGB565_H
#include <stdint.h>

#define HEALTH_WIDTH 4
#define HEALTH_HEIGHT 6
#define HEALTH_PIXELS 24

/* Timeline frame 1 */
static const uint16_t health_front_health_1[] = {
    0x07E0, 0x07E0, 0x07E0, 0x61E7, 0x07E0, 0x07E0, 0x61E7, 0x61E7, 0x07E0, 0x07E0, 0x61E7, 0x59C6,
    0x07E0, 0x07E0, 0x59C6, 0x5186, 0x07E0, 0x07E0, 0x5186, 0x4965, 0x07E0, 0x07E0, 0x07E0, 0x4944,
};

/* Timeline frame 2 */
static const uint16_t health_health_1[] = {
    0x61E7, 0x61E7, 0x61E7, 0x61E7, 0x61E7, 0x61E7, 0x61E7, 0x61E7, 0x59C6, 0x59C6, 0x59C6, 0x59C6,
    0x5186, 0x5186, 0x5186, 0x5186, 0x4965, 0x4965, 0x4965, 0x4965, 0x4944, 0x4944, 0x4944, 0x4944,
};

/* Timeline frame 3 */
static const uint16_t health_back_health_1[] = {
    0x61E7, 0x07E0, 0x07E0, 0x07E0, 0x61E7, 0x61E7, 0x07E0, 0x07E0, 0x59C6, 0x61E7, 0x07E0, 0x07E0,
    0x5186, 0x59C6, 0x07E0, 0x07E0, 0x4965, 0x5186, 0x07E0, 0x07E0, 0x4944, 0x07E0, 0x07E0, 0x07E0,
};

#define HEALTH_FRAMES_COUNT 3
static const uint16_t * const health_frames[] = {
    health_front_health_1,
    health_health_1,
    health_back_health_1,
};
static const uint32_t health_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t health_frames_source_frames[] = { 1, 2, 3 };

#define HEALTH_FRONT_HEALTH_COUNT 1
static const uint16_t * const health_front_health[] = {
    health_front_health_1,
};
static const uint32_t health_front_health_duration_ms[] = { 100 };
static const uint32_t health_front_health_source_frames[] = { 1 };

#define HEALTH_HEALTH_COUNT 1
static const uint16_t * const health_health[] = {
    health_health_1,
};
static const uint32_t health_health_duration_ms[] = { 100 };
static const uint32_t health_health_source_frames[] = { 2 };

#define HEALTH_BACK_HEALTH_COUNT 1
static const uint16_t * const health_back_health[] = {
    health_back_health_1,
};
static const uint32_t health_back_health_duration_ms[] = { 100 };
static const uint32_t health_back_health_source_frames[] = { 3 };

#endif /* HEALTH_RGB565_H */
