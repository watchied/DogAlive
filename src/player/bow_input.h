#ifndef BOW_INPUT_H
#define BOW_INPUT_H
#include <stdint.h>
#include "src/player/player.h"

#define BOW_TYPE_HOLD_MS 400
typedef struct { bool down, switched; uint64_t pressedAt; } BowInput;
static inline void BowInput_Press(BowInput *input, uint64_t now)
{
    if (!input->down) *input = (BowInput){.down = true, .pressedAt = now};
}
static inline void BowInput_Update(BowInput *input, Player *p, uint64_t now)
{
    if (input->down && !input->switched && now - input->pressedAt >= BOW_TYPE_HOLD_MS) {
        p->arrowType = (ArrowType)(((int)p->arrowType + 1) % 3);
        input->switched = true;
    }
}
static inline bool BowInput_Release(BowInput *input, Player *p, uint64_t now)
{
    if (!input->down) return false;
    BowInput_Update(input, p, now);
    bool shoot = !input->switched;
    *input = (BowInput){0};
    return shoot;
}
#endif
