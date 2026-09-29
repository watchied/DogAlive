#ifndef SLASH_SFX_H
#define SLASH_SFX_H

#include <SDL3/SDL.h>
#include "src/player/player.h"

#ifdef DOGALIVE_STM32_PLATFORM_H
// The STM32 port has its own I2S music driver and no SDL audio stream.
typedef struct { unsigned int device; } SlashSfx;
static inline bool SlashSfx_Init(SlashSfx *sfx) { (void)sfx; return true; }
static inline void SlashSfx_Update(SlashSfx *sfx, Player *player, bool active)
{ (void)sfx; (void)player; (void)active; }
static inline void SlashSfx_Close(SlashSfx *sfx) { (void)sfx; }
#else

// Both clips start on the visible slash at 0.3 s and peak near the 0.4 s hit.
#define SLASH_SFX_NORMAL_PATH "assets/sfx/d_minor_normal_slash.wav"
#define SLASH_SFX_CHARGED_PATH "assets/sfx/sword_slash.wav"
#define SLASH_SFX_PARRY_PATH "assets/sfx/parry_clash.wav"
enum { SLASH_SFX_NORMAL, SLASH_SFX_CHARGED, SLASH_SFX_COUNT };

typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioStream *stream;
    SDL_AudioStream *parryStream;
    Uint8 *samples[SLASH_SFX_COUNT];
    Uint32 sampleBytes[SLASH_SFX_COUNT];
    Uint8 *parrySamples;
    Uint32 parryBytes;
    bool audioInitialized;
    bool playedThisSwing;
    bool wasActive;
} SlashSfx;

static inline void SlashSfx_Close(SlashSfx *sfx)
{
    if (sfx->parryStream) SDL_DestroyAudioStream(sfx->parryStream);
    if (sfx->stream) SDL_DestroyAudioStream(sfx->stream);
    if (sfx->device) SDL_CloseAudioDevice(sfx->device);
    for (int i = 0; i < SLASH_SFX_COUNT; ++i) SDL_free(sfx->samples[i]);
    SDL_free(sfx->parrySamples);
    if (sfx->audioInitialized) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    *sfx = (SlashSfx){0};
}

static inline bool SlashSfx_Load(const char *name, SDL_AudioSpec *spec,
                                 Uint8 **samples, Uint32 *sampleBytes)
{
    if (SDL_LoadWAV(name, spec, samples, sampleBytes)) return true;
    // Explorer launches with bin/ as the working directory; use the executable path.
    char path[1024];
    SDL_snprintf(path, sizeof(path), "%s../%s", SDL_GetBasePath(), name);
    return SDL_LoadWAV(path, spec, samples, sampleBytes);
}

static inline bool SlashSfx_Init(SlashSfx *sfx)
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return false;
    sfx->audioInitialized = true;
    SDL_AudioSpec spec, chargedSpec;
    if (!SlashSfx_Load(SLASH_SFX_NORMAL_PATH, &spec,
                       &sfx->samples[SLASH_SFX_NORMAL], &sfx->sampleBytes[SLASH_SFX_NORMAL]) ||
        !SlashSfx_Load(SLASH_SFX_CHARGED_PATH, &chargedSpec,
                       &sfx->samples[SLASH_SFX_CHARGED], &sfx->sampleBytes[SLASH_SFX_CHARGED])) {
        SDL_Log("Could not load slash sound: %s", SDL_GetError());
        SlashSfx_Close(sfx);
        return false;
    }
    if (spec.format != chargedSpec.format || spec.channels != chargedSpec.channels ||
        spec.freq != chargedSpec.freq) {
        SDL_Log("Slash sounds must use the same PCM format");
        SlashSfx_Close(sfx);
        return false;
    }
    sfx->device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!sfx->device) { SlashSfx_Close(sfx); return false; }
    sfx->stream = SDL_CreateAudioStream(&spec, NULL);
    if (!sfx->stream || !SDL_BindAudioStream(sfx->device, sfx->stream)) {
        SlashSfx_Close(sfx);
        return false;
    }
    SDL_AudioSpec parrySpec;
    if (!SlashSfx_Load(SLASH_SFX_PARRY_PATH, &parrySpec,
                       &sfx->parrySamples, &sfx->parryBytes)) {
        SDL_Log("Could not load parry sound: %s", SDL_GetError());
        return true; // Missing parry audio must not silence ordinary slashes.
    }
    sfx->parryStream = SDL_CreateAudioStream(&parrySpec, NULL);
    if (!sfx->parryStream ||
        !SDL_BindAudioStream(sfx->device, sfx->parryStream)) {
        SDL_Log("Could not prepare parry sound: %s", SDL_GetError());
        if (sfx->parryStream) SDL_DestroyAudioStream(sfx->parryStream);
        sfx->parryStream = NULL;
    }
    return true;
}

static inline void SlashSfx_Update(SlashSfx *sfx, Player *player, bool active)
{
    // Keep the device running for menu cues, but stop gameplay clips on pause.
    if (!active) {
        if (sfx->wasActive) {
            if (sfx->stream) SDL_ClearAudioStream(sfx->stream);
            if (sfx->parryStream) SDL_ClearAudioStream(sfx->parryStream);
        }
        sfx->wasActive = false;
        player->parrySoundPending = false;
        return;
    }
    sfx->wasActive = true;
    if (player->parrySoundPending) {
        player->parrySoundPending = false;
        if (sfx->parryStream) {
            SDL_ClearAudioStream(sfx->parryStream);
            if (!SDL_PutAudioStreamData(sfx->parryStream,
                                        sfx->parrySamples, (int)sfx->parryBytes))
                SDL_Log("Parry sound playback failed: %s", SDL_GetError());
        }
    }
    if (!player->isAttacking) {
        sfx->playedThisSwing = false;
        return;
    }
    float attackTime = player->currentFrame * MELEE_ACTOR_FRAME_TIME + player->animTimer;
    if (!sfx->playedThisSwing && attackTime >= MELEE_SLASH_START_TIME) {
        sfx->playedThisSwing = true;
        if (sfx->stream) {
            int type = player->chargedAttack ? SLASH_SFX_CHARGED : SLASH_SFX_NORMAL;
            SDL_ClearAudioStream(sfx->stream);
            if (!SDL_PutAudioStreamData(sfx->stream, sfx->samples[type], (int)sfx->sampleBytes[type]))
                SDL_Log("Slash sound playback failed: %s", SDL_GetError());
        }
    }
}

#endif
#endif
