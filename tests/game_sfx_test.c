#include <stdio.h>
#include "src/audio/game_sfx_events.h"
#include "src/audio/slash_sfx.h"

#define CHECK(x) do { if(!(x)) { \
    fprintf(stderr,"line %d: %s: %s\n",__LINE__,#x,SDL_GetError()); return 1; \
} } while(0)

int main(void)
{
    if(!SDL_getenv("DOGALIVE_TEST_REAL_AUDIO"))
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER,"dummy");
    static GameSfxSystem s;
    CHECK(GameSfx_Init(&s.audio,0));
    CHECK(s.audio.device!=0);
    for(int i=0;i<SFX_COUNT;++i) {
        CHECK(s.audio.samples[i]!=NULL && s.audio.bytes[i]>0);
        if(GameSfx_RecordingPath((SfxCue)i)) CHECK(s.audio.recorded[i]);
        bool audible=false;
        for(int n=0;n<s.audio.bytes[i]/2;++n)
            if(s.audio.samples[i][n]>500 || s.audio.samples[i][n]<-500) {
                audible=true;break;
            }
        CHECK(audible);
    }
    const SfxCue slimeKingCues[]={
        SFX_KING_INTRO,SFX_KING_HURT,SFX_BUBBLE_LAUNCH,SFX_BUBBLE_BOUNCE,
        SFX_BUBBLE_POP,SFX_DASH_WARNING,SFX_DASH,SFX_BURROW,
        SFX_SLAM_WARNING,SFX_SLAM,SFX_KING_PHASE,SFX_LASER_WARNING,
        SFX_LASER,SFX_LASER_BOUNCE,SFX_BEAM_CHARGE,SFX_BEAM_FIRE,
        SFX_BEAM_INTERRUPT,SFX_KING_DEATH,SFX_KING_NPC
    };
    for(unsigned int i=0;i<sizeof(slimeKingCues)/sizeof(slimeKingCues[0]);++i)
        CHECK(s.audio.recorded[slimeKingCues[i]]);
    for(int i=0;i<GAME_SFX_VOICES;++i)
        CHECK(SDL_GetAudioStreamDevice(s.audio.voices[i])==s.audio.device);
    CHECK(SDL_PauseAudioDevice(s.audio.device));
    GameSfx_Play(&s.audio,SFX_MENU_SELECT);
    CHECK(s.audio.lastPlayed[SFX_MENU_SELECT]!=0);
    bool queued=false;
    for(int i=0;i<GAME_SFX_VOICES;++i)
        if(SDL_GetAudioStreamQueued(s.audio.voices[i])>0) queued=true;
    CHECK(queued);

    GameSfxSnapshot before={0}, after={0};
    before.gameState=after.gameState=1;
    before.stage=after.stage=0;
    before.storyMode=after.storyMode=STORY_NORMAL;
    before.hp=after.hp=100;
    before.maxHP=after.maxHP=100;
    s.last=before;s.hasSnapshot=true;
    after.moving=true;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DUNGEON_STEP_1]!=0);
    CHECK(s.audio.lastPlayed[SFX_STEP]==0);
    s.stepTimer=0;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DUNGEON_STEP_2]!=0);
    s.stepTimer=0;
    after.stage=STAGE_ENTRANCE_ROOM;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_STEP]!=0);
    s.stepTimer=0;
    after.stage=0;
    after.sprinting=true;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DUNGEON_STEP_3]!=0);
    CHECK(s.audio.lastPlayed[SFX_RUN_STEP]==0);
    s.stepTimer=0;
    s.audio.lastPlayed[SFX_STEP]=0;
    after.storyMode=STORY_EPILOGUE;
    after.moving=false;
    after.sprinting=false;
    after.knightMoving=true;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_STEP]!=0);
    after=before;
    s.last=before;
    s.stepTimer=0;
    after.hp=20;
    after.charging=true;
    after.chargeTimer=.10f;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_PLAYER_HURT]!=0);
    CHECK(s.audio.lastPlayed[SFX_LOW_HEALTH]!=0);
    CHECK(s.audio.lastPlayed[SFX_CHARGE_START]==0);
    after.charging=false;
    after.chargeTimer=0;
    after.attacking=true;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_CHARGE_START]==0);
    after.attacking=false;
    after.charging=true;
    after.chargeTimer=.10f;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_CHARGE_START]==0);
    after.chargeTimer=GAME_SFX_CHARGE_START_DELAY;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_CHARGE_START]!=0);
    after.chargeTimer=PLAYER_CHARGE_TIME;
    after.chargeReady=true;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_CHARGE_READY]!=0);
    before=after;
    after.charging=false;
    after.chargeReady=false;
    after.chargeTimer=0;
    after.stage=STAGE_COFFIN_ROOM;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DOOR]!=0);
    before=after;
    before.coffinState=after.coffinState=FC_SLASH;
    before.coffinTimer=.20f;
    after.coffinTimer=.55f;
    after.coffinSlashHitTime=.50f;
    s.last=before;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_COFFIN_SLASH]!=0);
    before=after;
    after.arrowExplosionPeak=ARROW_EXPLOSION_TIME;
    after.chestBlast[0]=ARROW_EXPLOSION_TIME;
    after.chestCount=1;
    s.last=before;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_EXPLOSION]!=0);
    CHECK(s.audio.lastPlayed[SFX_TRAP_BLAST]!=0);
    before=after;
    after.storyMode=STORY_RESCUE;
    after.rescueStep=0;
    s.last=before;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DOG_ARRIVAL]!=0);
    before=after;
    after.rescueStep=3;
    s.last=before;
    GameSfx_Observe(&s,&after,.016f);
    CHECK(s.audio.lastPlayed[SFX_DOG_PARRY]!=0);

    GameSfxSnapshot chestClosed={0}, chestOpening={0};
    chestClosed.gameState=1;
    chestClosed.stage=STAGE_START_ROOM;
    chestClosed.storyMode=STORY_NORMAL;
    chestClosed.hp=chestClosed.maxHP=100;
    chestClosed.chestCount=1;
    chestOpening=chestClosed;
    chestOpening.chestState[0]=CHEST_OPENING;
    s.last=chestClosed;
    GameSfx_Observe(&s,&chestOpening,.016f);
    CHECK(s.audio.recorded[SFX_CHEST_OPEN]);
    CHECK(s.audio.lastPlayed[SFX_CHEST_OPEN]!=0);

    // A trapped chest still warns instead of playing the latch.
    chestClosed.stage=STAGE_REWARD_ROOM;
    chestClosed.chestCount=2;
    chestOpening=chestClosed;
    chestOpening.chestState[1]=CHEST_OPENING;
    s.last=chestClosed;
    Uint64 chestPlayed=s.audio.lastPlayed[SFX_CHEST_OPEN];
    GameSfx_Observe(&s,&chestOpening,.016f);
    CHECK(s.audio.lastPlayed[SFX_TRAP_WARNING]!=0);
    CHECK(s.audio.lastPlayed[SFX_CHEST_OPEN]==chestPlayed);

    GameSfxSnapshot kingBefore={0}, kingAfter={0};
    kingBefore.gameState=1;
    kingBefore.stage=STAGE_BOSS_ROOM;
    kingBefore.storyMode=STORY_NORMAL;
    kingBefore.hp=kingBefore.maxHP=100;
    kingBefore.kingState=KING_IDLE;
    kingBefore.kingHp=KING_MAX_HP;
    kingAfter=kingBefore;
    kingAfter.kingHp-=20;
    kingAfter.kingState=KING_RADIAL;
    s.last=kingBefore;
    GameSfx_Observe(&s,&kingAfter,.016f);
    CHECK(s.audio.lastPlayed[SFX_KING_HURT]!=0);
    CHECK(s.audio.lastPlayed[SFX_LASER_WARNING]!=0);
    kingBefore=kingAfter;
    kingAfter.kingState=KING_PHASE_CHANGE;
    s.last=kingBefore;
    GameSfx_Observe(&s,&kingAfter,.016f);
    CHECK(s.audio.lastPlayed[SFX_KING_PHASE]!=0);

    // Story transitions and victory should leave the ending song unobscured.
    GameSfxSnapshot ending={0};
    ending.gameState=1;
    ending.stage=0;
    ending.storyMode=STORY_NORMAL;
    ending.hp=ending.maxHP=100;
    s.last=ending;
    Uint64 playedBeforeEnding[SFX_COUNT];
    SDL_memcpy(playedBeforeEnding,s.audio.lastPlayed,sizeof(playedBeforeEnding));
    const int endingModes[]={STORY_RETURN,STORY_SLIDES,STORY_EPILOGUE,STORY_SKY,STORY_WHITE};
    for(int mode=0;mode<5;++mode) {
        ending.storyMode=endingModes[mode];
        GameSfx_Observe(&s,&ending,.016f);
        for(int cue=0;cue<SFX_COUNT;++cue)
            CHECK(s.audio.lastPlayed[cue]==playedBeforeEnding[cue]);
    }
    ending.gameState=3;
    GameSfx_Observe(&s,&ending,.016f);
    for(int cue=0;cue<SFX_COUNT;++cue)
        CHECK(s.audio.lastPlayed[cue]==playedBeforeEnding[cue]);

    // Reentering the outside area on the return journey must not add a swell.
    ending.gameState=1;
    ending.stage=0;
    ending.storyMode=STORY_RETURN;
    s.last=ending;
    ending.stage=STAGE_ENTRANCE_ROOM;
    Uint64 entranceBefore=s.audio.lastPlayed[SFX_ENTRANCE];
    GameSfx_Observe(&s,&ending,.016f);
    CHECK(s.audio.lastPlayed[SFX_ENTRANCE]==entranceBefore);

    GameSfx_Close(&s.audio);
    SlashSfx slash={0};
    GameSfx shared={0};
    CHECK(SlashSfx_Init(&slash));
    CHECK(GameSfx_Init(&shared,slash.device));
    CHECK(SDL_GetAudioStreamDevice(shared.voices[0])==slash.device);
    CHECK(shared.recorded[SFX_COFFIN_WAKE] && shared.recorded[SFX_COFFIN_PHASE]);
    GameSfx_Play(&shared,SFX_COFFIN_PHASE);
    CHECK(shared.lastPlayed[SFX_COFFIN_PHASE]!=0);
    GameSfx_Close(&shared);
    SlashSfx_Close(&slash);
    puts("All generated and recorded cues are audible and game events queue their effects");
    return 0;
}
