#include <assert.h>
#include <stdio.h>
#include "src/player/bow_input.h"
int main(void)
{
    Player p;
    Player_Init(&p);
    BowInput input = {0};
    BowInput_Press(&input, 1000);
    assert(BowInput_Release(&input, &p, 1100));
    assert(p.arrowType == ARROW_NORMAL);
    BowInput_Press(&input, 2000);
    BowInput_Update(&input, &p, 2400);
    assert(p.arrowType == ARROW_FIRE);
    BowInput_Update(&input, &p, 5000);
    assert(p.arrowType == ARROW_FIRE);
    assert(!BowInput_Release(&input, &p, 5001));
    BowInput_Press(&input, 6000);
    assert(!BowInput_Release(&input, &p, 6500));
    assert(p.arrowType == ARROW_EXPLOSIVE);
    BowInput_Press(&input, 7000);
    assert(!BowInput_Release(&input, &p, 7500));
    assert(p.arrowType == ARROW_NORMAL);
    BowInput_Press(&input, 8000);
    input = (BowInput){0}; // Pause/focus loss cancels the pending tap.
    assert(!BowInput_Release(&input, &p, 8100));
    puts("Bow tap/hold tests passed");
    return 0;
}
