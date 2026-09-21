/* Aseprite RGB565 export. Row-major: index = y * WIDTH + x. */
/* Tag arrays contain selected frames only; one loop, tag repeats ignored. */
/* Alpha=0 uses configured hex; partial alpha blends over black. */
#ifndef MINI_BLOOD_SPELLBLADE_RGB565_H
#define MINI_BLOOD_SPELLBLADE_RGB565_H
#include <stdint.h>

#define MINI_BLOOD_SPELLBLADE_WIDTH 8
#define MINI_BLOOD_SPELLBLADE_HEIGHT 8
#define MINI_BLOOD_SPELLBLADE_PIXELS 64

/* Timeline frame 1 */
static const uint16_t mini_blood_spellblade_frame_1[] = {
    0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xA041,
    0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xA041, 0xA041, 0xA041, 0x07E0, 0x07E0, 0x07E0,
    0x07E0, 0x07E0, 0x07E0, 0xB041, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xB841,
    0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0xC041, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
    0x07E0, 0x07E0, 0x07E0, 0xD041, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
    0x07E0, 0x07E0, 0x07E0, 0x07E0,
};

#define MINI_BLOOD_SPELLBLADE_FRAMES_COUNT 1
static const uint16_t * const mini_blood_spellblade_frames[] = {
    mini_blood_spellblade_frame_1,
};
static const uint32_t mini_blood_spellblade_frames_duration_ms[] = { 100 };
static const uint32_t mini_blood_spellblade_frames_source_frames[] = { 1 };

#endif /* MINI_BLOOD_SPELLBLADE_RGB565_H */
