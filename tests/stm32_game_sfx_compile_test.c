#include <SDL3/SDL.h>
#include "src/audio/game_sfx_events.h"

int main(void)
{
    GameSfxSystem s = {0};
    GameSfxSnapshot x = {0};
    GameSfx_Observe(&s, &x, 0);
    return 0;
}
