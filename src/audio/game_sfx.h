#ifndef GAME_SFX_H
#define GAME_SFX_H

#include <SDL3/SDL.h>
#include <math.h>
#include <stdint.h>

// Game effects use recorded overrides where available and generated fallbacks.
// Sword and parry recordings are handled separately in slash_sfx.h.
typedef enum {
    SFX_STEP, SFX_RUN_STEP,
    SFX_DUNGEON_STEP_1, SFX_DUNGEON_STEP_2, SFX_DUNGEON_STEP_3,
    SFX_CHARGE_START, SFX_CHARGE_READY,
    SFX_SWORD_HIT, SFX_SWORD_BLOCK, SFX_REFLECT, SFX_PLAYER_HURT,
    SFX_LOW_HEALTH, SFX_PLAYER_DEATH, SFX_RESPAWN, SFX_BOW_DRAW,
    SFX_BOW_RELEASE, SFX_ARROW_HIT, SFX_FIRE_IGNITE, SFX_FIRE_HIT,
    SFX_EXPLOSIVE_HIT, SFX_EXPLOSION, SFX_ITEM_SELECT, SFX_POTION_DRINK,
    SFX_POTION_HEAL, SFX_ENEMY_SWING, SFX_ENEMY_HURT, SFX_ENEMY_DEATH,
    SFX_SLIME_SHOT, SFX_SLIME_HURT, SFX_SLIME_DEATH, SFX_EYE_WARNING,
    SFX_EYE_BLAST, SFX_EYE_DEATH, SFX_CHECKPOINT, SFX_CHEST_OPEN,
    SFX_PICKUP, SFX_ENCHANT, SFX_TRAP_WARNING, SFX_TRAP_BLAST,
    SFX_DOOR, SFX_ENTRANCE, SFX_KING_INTRO, SFX_KING_HURT, SFX_BUBBLE_LAUNCH,
    SFX_BUBBLE_BOUNCE, SFX_BUBBLE_POP, SFX_DASH_WARNING, SFX_DASH,
    SFX_BURROW, SFX_SLAM_WARNING, SFX_SLAM, SFX_KING_PHASE,
    SFX_LASER_WARNING, SFX_LASER, SFX_LASER_BOUNCE, SFX_BEAM_CHARGE, SFX_BEAM_FIRE,
    SFX_BEAM_INTERRUPT, SFX_KING_DEATH, SFX_KING_NPC, SFX_COFFIN_WAKE,
    SFX_COFFIN_EYE, SFX_SWORD_RELEASE, SFX_SWORD_RETURN, SFX_GROUND_WAVE,
    SFX_STAB_WARNING, SFX_STAB, SFX_WALL_BLADES, SFX_ORBIT,
    SFX_TELEPORT, SFX_COFFIN_SLAM, SFX_COFFIN_DASH,
    SFX_COFFIN_TRAIL_WARNING, SFX_TRAIL_STRIKE,
    SFX_COFFIN_SLASH, SFX_COFFIN_CHARGE, SFX_COFFIN_CHARGED_SLASH,
    SFX_SWORD_BREAK, SFX_COFFIN_PHASE, SFX_COFFIN_STUN, SFX_COFFIN_DEATH,
    SFX_DOG_ARRIVAL, SFX_DOG_PARRY, SFX_DOG_ATTACK,
    SFX_DOG_INTERACT, SFX_GRAVE_INTERACT,
    SFX_MENU_SELECT, SFX_PAUSE, SFX_RESUME, SFX_GAME_OVER,
    SFX_COUNT
} SfxCue;

#ifdef DOGALIVE_STM32_PLATFORM_H
typedef struct { bool unused; } GameSfx;
static inline bool GameSfx_Init(GameSfx *s, unsigned int device) { (void)s; (void)device; return true; }
static inline void GameSfx_Play(GameSfx *s, SfxCue cue) { (void)s; (void)cue; }
static inline void GameSfx_Close(GameSfx *s) { (void)s; }
#else

#define GAME_SFX_VOICES 20
#define GAME_SFX_RATE 44100
typedef enum {
    SFX_FOOT, SFX_FLESH, SFX_METAL, SFX_RUMBLE, SFX_WIND,
    SFX_DREAD, SFX_WOOD, SFX_FLAME, SFX_WET, SFX_UI
} SfxFamily;
typedef struct {
    float seconds, startHz, endHz, noise, volume;
    SfxFamily family;
} SfxSpec;
typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioStream *voices[GAME_SFX_VOICES];
    Uint64 voiceUntil[GAME_SFX_VOICES];
    Sint16 *samples[SFX_COUNT];
    int bytes[SFX_COUNT];
    bool recorded[SFX_COUNT];
    Uint64 lastPlayed[SFX_COUNT];
    bool ownsDevice;
    bool audioInitialized;
} GameSfx;

static inline SfxSpec GameSfx_Spec(SfxCue cue)
{
    // Low, layered textures replace the old single-oscillator arcade bleeps.
    // Flesh Coffin's cues lean on wet impacts, breath, iron and sub-bass.
    SfxSpec s = {.seconds=.23f, .startHz=170, .endHz=65,
                 .noise=.58f, .volume=.32f, .family=SFX_FLESH};
    if (cue == SFX_STEP || cue == SFX_RUN_STEP ||
        (cue >= SFX_DUNGEON_STEP_1 && cue <= SFX_DUNGEON_STEP_3)) {
        s=(SfxSpec){cue==SFX_RUN_STEP?.08f:.105f,95,46,.74f,.19f,SFX_FOOT};
    } else if (cue==SFX_CHARGE_START || cue==SFX_CHARGE_READY ||
               cue==SFX_BEAM_CHARGE || cue==SFX_COFFIN_CHARGE ||
               cue==SFX_KING_PHASE || cue==SFX_COFFIN_PHASE || cue==SFX_ENCHANT) {
        s=(SfxSpec){.72f,82,105,.25f,.30f,SFX_DREAD};
        if(cue==SFX_CHARGE_READY) {s.seconds=.34f;s.startHz=115;s.endHz=125;}
        if(cue==SFX_BEAM_CHARGE || cue==SFX_COFFIN_CHARGE) {s.seconds=1.1f;s.startHz=64;s.endHz=98;s.volume=.37f;}
        if(cue==SFX_KING_PHASE || cue==SFX_COFFIN_PHASE) {s.seconds=1.25f;s.startHz=56;s.endHz=72;s.volume=.41f;}
    } else if (cue==SFX_SWORD_HIT || cue==SFX_SWORD_BLOCK || cue==SFX_REFLECT ||
               cue==SFX_BOW_RELEASE || cue==SFX_ARROW_HIT || cue==SFX_EXPLOSIVE_HIT ||
               cue==SFX_ENEMY_SWING || cue==SFX_STAB || cue==SFX_SWORD_RELEASE ||
               cue==SFX_SWORD_RETURN || cue==SFX_WALL_BLADES || cue==SFX_COFFIN_SLASH ||
               cue==SFX_COFFIN_CHARGED_SLASH || cue==SFX_SWORD_BREAK ||
               cue==SFX_DOG_PARRY || cue==SFX_BEAM_INTERRUPT) {
        s=(SfxSpec){.32f,580,150,.51f,.34f,SFX_METAL};
        if(cue==SFX_BOW_RELEASE || cue==SFX_ARROW_HIT) {s.seconds=.18f;s.startHz=410;s.volume=.26f;}
        if(cue==SFX_ENEMY_SWING || cue==SFX_COFFIN_SLASH) {s.seconds=.36f;s.startHz=500;s.noise=.72f;}
        if(cue==SFX_COFFIN_CHARGED_SLASH) {s.seconds=.55f;s.startHz=720;s.endHz=100;s.volume=.42f;}
        if(cue==SFX_SWORD_RELEASE || cue==SFX_SWORD_RETURN)
            {s.seconds=.48f;s.startHz=480;s.endHz=105;s.noise=.7f;}
        if(cue==SFX_SWORD_BREAK) {s.seconds=.84f;s.startHz=650;s.endHz=90;s.noise=.75f;s.volume=.43f;}
    } else if (cue==SFX_PLAYER_HURT || cue==SFX_ENEMY_HURT || cue==SFX_KING_HURT || cue==SFX_SLIME_HURT ||
               cue==SFX_SLIME_DEATH || cue==SFX_EYE_DEATH || cue==SFX_EYE_BLAST ||
               cue==SFX_DOG_ATTACK || cue==SFX_COFFIN_STUN) {
        s=(SfxSpec){.29f,180,54,.63f,.34f,SFX_FLESH};
        if(cue==SFX_EYE_BLAST) {s.seconds=.5f;s.startHz=120;s.volume=.39f;}
    } else if (cue==SFX_PLAYER_DEATH || cue==SFX_ENEMY_DEATH || cue==SFX_KING_DEATH ||
               cue==SFX_COFFIN_DEATH || cue==SFX_GAME_OVER || cue==SFX_COFFIN_WAKE) {
        s=(SfxSpec){.72f,90,35,.55f,.36f,SFX_RUMBLE};
        if(cue==SFX_KING_DEATH || cue==SFX_COFFIN_DEATH || cue==SFX_COFFIN_WAKE)
            {s.seconds=1.35f;s.startHz=70;s.endHz=26;s.volume=.44f;}
        if(cue==SFX_COFFIN_WAKE)
            {s=(SfxSpec){1.15f,46,82,.71f,.45f,SFX_FLESH};}
        if(cue==SFX_COFFIN_DEATH)
            {s=(SfxSpec){1.5f,96,22,.81f,.52f,SFX_FLESH};}
    } else if (cue==SFX_EXPLOSION || cue==SFX_TRAP_BLAST || cue==SFX_SLAM ||
               cue==SFX_COFFIN_SLAM || cue==SFX_TRAIL_STRIKE || cue==SFX_BEAM_FIRE) {
        s=(SfxSpec){.68f,115,29,.82f,.44f,SFX_RUMBLE};
        if(cue==SFX_BEAM_FIRE) {s.seconds=.9f;s.startHz=125;s.noise=.54f;}
    } else if (cue==SFX_LOW_HEALTH || cue==SFX_EYE_WARNING || cue==SFX_TRAP_WARNING ||
               cue==SFX_DASH_WARNING || cue==SFX_SLAM_WARNING || cue==SFX_LASER_WARNING || cue==SFX_STAB_WARNING ||
               cue==SFX_COFFIN_EYE) {
        s=(SfxSpec){.55f,105,132,.59f,.26f,SFX_WIND};
        if(cue==SFX_COFFIN_EYE) {s.seconds=.75f;s.startHz=72;s.endHz=116;s.volume=.34f;}
        if(cue==SFX_LOW_HEALTH) {s.seconds=.4f;s.startHz=85;s.endHz=72;}
    } else if (cue==SFX_BOW_DRAW || cue==SFX_DASH || cue==SFX_COFFIN_DASH ||
               cue==SFX_BURROW || cue==SFX_GROUND_WAVE || cue==SFX_ORBIT ||
               cue==SFX_TELEPORT || cue==SFX_DOG_ARRIVAL) {
        s=(SfxSpec){.43f,260,65,.75f,.3f,SFX_WIND};
        if(cue==SFX_BOW_DRAW) {s.seconds=.3f;s.startHz=120;s.endHz=185;s.noise=.42f;}
        if(cue==SFX_BURROW || cue==SFX_TELEPORT) {s.seconds=.65f;s.startHz=145;s.endHz=48;}
        if(cue==SFX_COFFIN_DASH || cue==SFX_GROUND_WAVE) {s.seconds=.5f;s.volume=.37f;}
    } else if (cue==SFX_SLIME_SHOT || cue==SFX_BUBBLE_LAUNCH || cue==SFX_BUBBLE_BOUNCE ||
               cue==SFX_BUBBLE_POP) {
        s=(SfxSpec){.24f,195,70,.58f,.29f,SFX_WET};
        if(cue==SFX_BUBBLE_BOUNCE) {s.seconds=.13f;s.startHz=155;s.endHz=93;}
        if(cue==SFX_BUBBLE_POP) {s.seconds=.16f;s.startHz=125;s.endHz=38;s.noise=.75f;}
    } else if (cue==SFX_LASER || cue==SFX_LASER_BOUNCE) {
        s=(SfxSpec){cue==SFX_LASER?.28f:.14f,340,95,.43f,.29f,SFX_METAL};
    } else if (cue==SFX_FIRE_IGNITE || cue==SFX_FIRE_HIT) {
        s=(SfxSpec){.35f,130,57,.85f,.31f,SFX_FLAME};
    } else if (cue==SFX_CHECKPOINT || cue==SFX_RESPAWN ||
               cue==SFX_KING_NPC || cue==SFX_ENTRANCE) {
        s=(SfxSpec){.72f,112,93,.16f,.29f,SFX_DREAD};
    } else if (cue==SFX_POTION_DRINK || cue==SFX_CHEST_OPEN || cue==SFX_DOOR ||
               cue==SFX_DOG_INTERACT || cue==SFX_GRAVE_INTERACT) {
        s=(SfxSpec){.28f,150,60,.7f,.29f,SFX_WOOD};
        if(cue==SFX_DOOR) {s.seconds=.52f;s.startHz=100;s.endHz=43;}
    } else if (cue==SFX_POTION_HEAL || cue==SFX_PICKUP || cue==SFX_ITEM_SELECT ||
               cue==SFX_MENU_SELECT || cue==SFX_PAUSE || cue==SFX_RESUME) {
        s=(SfxSpec){.17f,175,103,.3f,.19f,SFX_UI};
        if(cue==SFX_POTION_HEAL || cue==SFX_PICKUP) {s.seconds=.33f;s.family=SFX_DREAD;s.startHz=120;s.endHz=96;}
    } else if (cue==SFX_KING_INTRO) {
        s=(SfxSpec){1.2f,65,51,.32f,.38f,SFX_DREAD};
    }
    // Small variations prevent repeated actions from sounding machine-identical.
    float detune = 1.0f + ((int)cue % 7 - 3) * .018f;
    s.startHz *= detune; s.endHz *= detune;
    return s;
}

static inline bool GameSfx_Make(GameSfx *s, SfxCue cue)
{
    SfxSpec spec = GameSfx_Spec(cue);
    int count = (int)(spec.seconds * GAME_SFX_RATE);
    Sint16 *pcm = SDL_malloc((size_t)count * sizeof(Sint16));
    if (!pcm) return false;
    uint32_t rng = 0x9e3779b9u ^ ((uint32_t)cue * 747796405u);
    float phase=0, subPhase=0, metalPhaseA=0, metalPhaseB=0;
    float fastNoise=0, midNoise=0, slowNoise=0;
    float echoA[2048]={0}, echoB[4096]={0};
    bool swell = spec.family==SFX_DREAD || cue==SFX_COFFIN_WAKE || cue==SFX_COFFIN_EYE ||
                 cue==SFX_EYE_WARNING || cue==SFX_TRAP_WARNING ||
                 cue==SFX_SLAM_WARNING || cue==SFX_STAB_WARNING ||
                 cue==SFX_DASH_WARNING || cue==SFX_TELEPORT;
    for (int i = 0; i < count; ++i) {
        float u = (float)i / (float)count;
        float hz = spec.startHz * powf(spec.endHz/spec.startHz,u);
        phase += hz / GAME_SFX_RATE;
        subPhase += hz * .48f / GAME_SFX_RATE;
        metalPhaseA += hz * 2.37f / GAME_SFX_RATE;
        metalPhaseB += hz * 3.91f / GAME_SFX_RATE;
        rng = rng * 1664525u + 1013904223u;
        float noise = ((rng >> 8) / 8388607.5f) - 1.0f;
        fastNoise=fastNoise*.62f+noise*.38f;
        midNoise=midNoise*.965f+noise*.035f;
        slowNoise=slowNoise*.997f+noise*.003f;
        float hiss=fastNoise-midNoise;
        float grit=midNoise-slowNoise;
        float body=sinf(phase*6.2831853f);
        float sub=sinf(subPhase*6.2831853f);
        float iron=sinf(metalPhaseA*6.2831853f)*.63f+
                   sinf(metalPhaseB*6.2831853f)*.37f;
        float envelope=swell ? sinf(3.14159265f*u) :
            fminf(1.0f,u*110.0f)*powf(1.0f-u,1.65f);
        if(spec.family==SFX_FOOT || spec.family==SFX_UI)
            envelope*=powf(1.0f-u,2.0f);
        float texture=hiss*.65f+grit*.55f;
        float dry=0;
        switch(spec.family) {
        case SFX_FOOT:
            dry=sub*.34f+grit*.9f+hiss*.25f;
            break;
        case SFX_FLESH: {
            float gargle=sinf(phase*6.2831853f+sinf(i*.0019f)*.82f);
            dry=(sub*.38f+gargle*.18f)*(1.0f-spec.noise)+
                (grit*.7f+hiss*.38f)*spec.noise;
            break;
        }
        case SFX_METAL:
            dry=(iron*.8f+body*.22f)*(1.0f-spec.noise)+
                (hiss*.9f+grit*.25f)*spec.noise;
            break;
        case SFX_RUMBLE:
            dry=(sub*.9f+body*.2f)*(1.0f-spec.noise*.55f)+
                (grit*.76f+hiss*.24f*fmaxf(0,1-u*7))*spec.noise;
            break;
        case SFX_WIND:
            dry=(grit*.72f+hiss*.7f)*spec.noise+
                (sub*.35f+body*.16f)*(1.0f-spec.noise);
            break;
        case SFX_DREAD:
            dry=(body*.43f+sub*.4f+iron*.17f)*(1.0f-spec.noise*.5f)+
                grit*.43f*spec.noise;
            break;
        case SFX_WOOD:
            dry=(sub*.48f+grit*.95f+hiss*.2f)*(1.0f-u*.4f);
            break;
        case SFX_FLAME:
            dry=hiss*.83f+grit*.45f+sub*.16f;
            break;
        case SFX_WET:
            dry=(sinf(phase*6.2831853f+sinf(i*.0031f)*1.2f)*.57f+
                sub*.25f)*(1.0f-spec.noise)+texture*spec.noise;
            break;
        case SFX_UI:
            dry=sub*.48f+body*.2f+grit*.34f;
            break;
        }
        dry*=envelope;
        float echo=echoA[i%2048]*.23f+echoB[i%4096]*.14f;
        float room=(spec.family==SFX_FOOT || spec.family==SFX_UI)? .04f :
                   (spec.family==SFX_DREAD || cue==SFX_COFFIN_DEATH)?.48f:.27f;
        echoA[i%2048]=dry+echo*.22f;
        echoB[i%4096]=dry+echo*.15f;
        float sample=(dry+echo*room)*spec.volume;
        sample=sample/(1.0f+fabsf(sample)*.6f);
        pcm[i]=(Sint16)(fmaxf(-1.0f,fminf(1.0f,sample))*32767.0f);
    }
    s->samples[cue] = pcm;
    s->bytes[cue] = count * (int)sizeof(Sint16);
    return true;
}

static inline const char *GameSfx_RecordingPath(SfxCue cue)
{
    switch(cue) {
    case SFX_DUNGEON_STEP_1: return "assets/sfx/dungeon_step_1.wav";
    case SFX_DUNGEON_STEP_2: return "assets/sfx/dungeon_step_2.wav";
    case SFX_DUNGEON_STEP_3: return "assets/sfx/dungeon_step_3.wav";
    case SFX_CHEST_OPEN: return "assets/sfx/chest_open.wav";
    case SFX_KING_INTRO: return "assets/sfx/slime_king/intro.wav";
    case SFX_KING_HURT: return "assets/sfx/slime_king/hurt.wav";
    case SFX_BUBBLE_LAUNCH: return "assets/sfx/slime_king/bubble_launch.wav";
    case SFX_BUBBLE_BOUNCE: return "assets/sfx/slime_king/bubble_bounce.wav";
    case SFX_BUBBLE_POP: return "assets/sfx/slime_king/bubble_pop.wav";
    case SFX_DASH_WARNING: return "assets/sfx/slime_king/dash_warning.wav";
    case SFX_DASH: return "assets/sfx/slime_king/dash.wav";
    case SFX_BURROW: return "assets/sfx/slime_king/burrow.wav";
    case SFX_SLAM_WARNING: return "assets/sfx/slime_king/slam_warning.wav";
    case SFX_SLAM: return "assets/sfx/slime_king/slam.wav";
    case SFX_KING_PHASE: return "assets/sfx/slime_king/phase.wav";
    case SFX_LASER_WARNING: return "assets/sfx/slime_king/laser_warning.wav";
    case SFX_LASER: return "assets/sfx/slime_king/laser.wav";
    case SFX_LASER_BOUNCE: return "assets/sfx/slime_king/laser_bounce.wav";
    case SFX_BEAM_CHARGE: return "assets/sfx/slime_king/beam_charge.wav";
    case SFX_BEAM_FIRE: return "assets/sfx/slime_king/beam_fire.wav";
    case SFX_BEAM_INTERRUPT: return "assets/sfx/slime_king/beam_interrupt.wav";
    case SFX_KING_DEATH: return "assets/sfx/slime_king/death.wav";
    case SFX_KING_NPC: return "assets/sfx/slime_king/npc.wav";
    case SFX_COFFIN_WAKE: return "assets/sfx/coffin/awakening.wav";
    case SFX_COFFIN_EYE: return "assets/sfx/coffin/eye_warning.wav";
    case SFX_SWORD_RELEASE: return "assets/sfx/coffin/sword_release.wav";
    case SFX_SWORD_RETURN: return "assets/sfx/coffin/sword_return.wav";
    case SFX_GROUND_WAVE: return "assets/sfx/coffin/ground_wave.wav";
    case SFX_STAB_WARNING: return "assets/sfx/coffin/stab_warning.wav";
    case SFX_STAB: return "assets/sfx/coffin/stab.wav";
    case SFX_WALL_BLADES: return "assets/sfx/coffin/wall_blades.wav";
    case SFX_ORBIT: return "assets/sfx/coffin/orbit.wav";
    case SFX_TELEPORT: return "assets/sfx/coffin/teleport.wav";
    case SFX_COFFIN_SLAM: return "assets/sfx/coffin/slam.wav";
    case SFX_COFFIN_DASH: return "assets/sfx/coffin/dash.wav";
    case SFX_COFFIN_TRAIL_WARNING: return "assets/sfx/coffin/trail_warning.wav";
    case SFX_TRAIL_STRIKE: return "assets/sfx/coffin/trail_strike.wav";
    case SFX_COFFIN_SLASH: return "assets/sfx/coffin/slash.wav";
    case SFX_COFFIN_CHARGE: return "assets/sfx/coffin/charge.wav";
    case SFX_COFFIN_CHARGED_SLASH: return "assets/sfx/coffin/charged_slash.wav";
    case SFX_SWORD_BLOCK: return "assets/sfx/coffin/sword_block.wav";
    case SFX_SWORD_BREAK: return "assets/sfx/coffin/sword_break.wav";
    case SFX_COFFIN_PHASE: return "assets/sfx/coffin/phase_two.wav";
    case SFX_COFFIN_STUN: return "assets/sfx/coffin/stun.wav";
    case SFX_COFFIN_DEATH: return "assets/sfx/coffin/death.wav";
    default: return NULL;
    }
}

static inline bool GameSfx_LoadRecording(GameSfx *s, SfxCue cue, const char *path)
{
    SDL_AudioSpec spec;
    Uint8 *pcm=NULL;
    Uint32 bytes=0;
    if(!SDL_LoadWAV(path,&spec,&pcm,&bytes)) {
        char fullPath[1024];
        SDL_snprintf(fullPath,sizeof(fullPath),"%s../%s",SDL_GetBasePath(),path);
        if(!SDL_LoadWAV(fullPath,&spec,&pcm,&bytes)) return false;
    }
    if(spec.format!=SDL_AUDIO_S16 || spec.channels!=1 ||
       spec.freq!=GAME_SFX_RATE || bytes==0) {
        SDL_free(pcm);
        SDL_SetError("Recorded sound must be mono 44.1 kHz 16-bit PCM: %s",path);
        return false;
    }
    SDL_free(s->samples[cue]);
    s->samples[cue]=(Sint16*)pcm;
    s->bytes[cue]=(int)bytes;
    s->recorded[cue]=true;
    return true;
}

static inline void GameSfx_Close(GameSfx *s)
{
    for (int i = 0; i < GAME_SFX_VOICES; ++i)
        if (s->voices[i]) SDL_DestroyAudioStream(s->voices[i]);
    for (int i = 0; i < SFX_COUNT; ++i) SDL_free(s->samples[i]);
    if (s->ownsDevice && s->device) SDL_CloseAudioDevice(s->device);
    if (s->audioInitialized) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    *s = (GameSfx){0};
}

static inline bool GameSfx_Init(GameSfx *s, SDL_AudioDeviceID device)
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return false;
    s->audioInitialized = true;
    s->device = device;
    if (!s->device) {
        s->device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
        s->ownsDevice = true;
    }
    if (!s->device) { GameSfx_Close(s); return false; }
    SDL_AudioSpec spec = {SDL_AUDIO_S16, 1, GAME_SFX_RATE};
    for (int i = 0; i < GAME_SFX_VOICES; ++i) {
        s->voices[i] = SDL_CreateAudioStream(&spec, NULL);
        if (!s->voices[i] || !SDL_BindAudioStream(s->device, s->voices[i])) {
            GameSfx_Close(s); return false;
        }
    }
    for (int i = 0; i < SFX_COUNT; ++i)
        if (!GameSfx_Make(s, (SfxCue)i)) { GameSfx_Close(s); return false; }
    for (int i = 0; i < SFX_COUNT; ++i) {
        const char *path=GameSfx_RecordingPath((SfxCue)i);
        if(path && !GameSfx_LoadRecording(s,(SfxCue)i,path))
            SDL_Log("Could not load recorded sound %s: %s",path,SDL_GetError());
    }
    return true;
}

static inline void GameSfx_Play(GameSfx *s, SfxCue cue)
{
    if (!s->device || cue < 0 || cue >= SFX_COUNT || !s->samples[cue]) return;
    Uint64 now = SDL_GetTicks();
    // Several enemies can perform the same action on one frame; one cue is clear.
    if (s->lastPlayed[cue] && now - s->lastPlayed[cue] < 55) return;
    s->lastPlayed[cue] = now ? now : 1;
    int choice = 0;
    for (int i = 0; i < GAME_SFX_VOICES; ++i)
        if (s->voiceUntil[i] <= now) { choice = i; break; }
        else if (s->voiceUntil[i] < s->voiceUntil[choice]) choice = i;
    SDL_ClearAudioStream(s->voices[choice]);
    if (!SDL_PutAudioStreamData(s->voices[choice], s->samples[cue], s->bytes[cue]))
        SDL_Log("Effect playback failed: %s", SDL_GetError());
    s->voiceUntil[choice] = now + (Uint64)s->bytes[cue] * 1000 /
                                 (sizeof(Sint16) * GAME_SFX_RATE);
}
#endif
#endif
