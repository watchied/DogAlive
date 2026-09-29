#include <stdio.h>
#include "src/audio/slash_sfx.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Check failed at line %d: %s (%s)\n", \
                __LINE__, #condition, SDL_GetError()); \
        return 1; \
    } \
} while (0)

int main(void)
{
    if (!SDL_getenv("DOGALIVE_TEST_REAL_AUDIO"))
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    SlashSfx sfx = {0};
    CHECK(SlashSfx_Init(&sfx));
    CHECK(sfx.device != 0);
    CHECK(sfx.sampleBytes[SLASH_SFX_NORMAL] > 0);
    CHECK(sfx.sampleBytes[SLASH_SFX_CHARGED] > 0);
    CHECK(sfx.sampleBytes[SLASH_SFX_NORMAL] != sfx.sampleBytes[SLASH_SFX_CHARGED]);
    CHECK(sfx.parryStream && sfx.parryBytes > 0);
    CHECK(SDL_GetAudioStreamDevice(sfx.stream) == sfx.device);
    CHECK(SDL_GetAudioStreamDevice(sfx.parryStream) == sfx.device);
    // Hold the dummy device so queued samples remain available to inspect.
    CHECK(SDL_PauseAudioDevice(sfx.device));

    Player player;
    Player_Init(&player);
    player.isAttacking = true;
    player.currentFrame = 2;
    SlashSfx_Update(&sfx, &player, true);
    CHECK(!sfx.playedThisSwing);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == 0);

    player.currentFrame = 3;
    SlashSfx_Update(&sfx, &player, true);
    CHECK(sfx.playedThisSwing);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == (int)sfx.sampleBytes[SLASH_SFX_NORMAL]);
    SlashSfx_Update(&sfx, &player, true);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == (int)sfx.sampleBytes[SLASH_SFX_NORMAL]);

    player.parrySoundPending = true;
    SlashSfx_Update(&sfx, &player, true);
    CHECK(!player.parrySoundPending);
    CHECK(SDL_GetAudioStreamQueued(sfx.parryStream) == (int)sfx.parryBytes);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == (int)sfx.sampleBytes[SLASH_SFX_NORMAL]);
    SlashSfx_Update(&sfx, &player, true);
    CHECK(SDL_GetAudioStreamQueued(sfx.parryStream) == (int)sfx.parryBytes);

    player.isAttacking = false;
    SlashSfx_Update(&sfx, &player, true);
    player.isAttacking = true;
    player.chargedAttack = true;
    SlashSfx_Update(&sfx, &player, true);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == (int)sfx.sampleBytes[SLASH_SFX_CHARGED]);
    SlashSfx_Update(&sfx, &player, true);
    CHECK(SDL_GetAudioStreamQueued(sfx.stream) == (int)sfx.sampleBytes[SLASH_SFX_CHARGED]);

    SlashSfx_Close(&sfx);
    puts("Normal, charged, and successful-parry clips queue on their own audio streams");
    return 0;
}
