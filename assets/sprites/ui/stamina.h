/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef STAMINA_RGB565_H
#define STAMINA_RGB565_H
#include <stdint.h>

#define STAMINA_WIDTH 4
#define STAMINA_HEIGHT 4
#define STAMINA_PIXELS 16

/* Timeline frame 1 */
static const uint16_t stamina_front_stamina_1[] = {
    0x07E0, 0x07E0, 0x07E0, 0x53AF, 0x07E0, 0x07E0, 0x4B6E, 0x4B4E, 0x07E0, 0x07E0, 0x430C, 0x3AAB,
    0x07E0, 0x07E0, 0x07E0, 0x328A,
};

/* Timeline frame 2 */
static const uint16_t stamina_stamina_1[] = {
    0x53AF, 0x53AF, 0x53AF, 0x53AF, 0x4B4E, 0x4B4E, 0x4B4E, 0x4B4E, 0x3AAB, 0x3AAB, 0x3AAB, 0x3AAB,
    0x328A, 0x328A, 0x328A, 0x328A,
};

/* Timeline frame 3 */
static const uint16_t stamina_back_stamina_1[] = {
    0x53AF, 0x07E0, 0x07E0, 0x07E0, 0x4B4E, 0x4B6E, 0x07E0, 0x07E0, 0x3AAB, 0x430C, 0x07E0, 0x07E0,
    0x328A, 0x07E0, 0x07E0, 0x07E0,
};

#define STAMINA_FRAMES_COUNT 3
static const uint16_t * const stamina_frames[] = {
    stamina_front_stamina_1,
    stamina_stamina_1,
    stamina_back_stamina_1,
};
static const uint32_t stamina_frames_duration_ms[] = { 100, 100, 100 };
static const uint32_t stamina_frames_source_frames[] = { 1, 2, 3 };

#define STAMINA_FRONT_STAMINA_COUNT 1
static const uint16_t * const stamina_front_stamina[] = {
    stamina_front_stamina_1,
};
static const uint32_t stamina_front_stamina_duration_ms[] = { 100 };
static const uint32_t stamina_front_stamina_source_frames[] = { 1 };

#define STAMINA_STAMINA_COUNT 1
static const uint16_t * const stamina_stamina[] = {
    stamina_stamina_1,
};
static const uint32_t stamina_stamina_duration_ms[] = { 100 };
static const uint32_t stamina_stamina_source_frames[] = { 2 };

#define STAMINA_BACK_STAMINA_COUNT 1
static const uint16_t * const stamina_back_stamina[] = {
    stamina_back_stamina_1,
};
static const uint32_t stamina_back_stamina_duration_ms[] = { 100 };
static const uint32_t stamina_back_stamina_source_frames[] = { 3 };

#endif /* STAMINA_RGB565_H */
