#ifndef GAME_SFX_EVENTS_H
#define GAME_SFX_EVENTS_H

#include "src/audio/game_sfx.h"
#include "src/story/story.h"

#ifdef DOGALIVE_STM32_PLATFORM_H
// The microcontroller currently reserves its I2S channel for streamed music.
typedef struct { bool unused; } GameSfxSnapshot;
typedef struct { GameSfx audio; } GameSfxSystem;
static inline GameSfxSnapshot GameSfx_Capture(int state, const StageProgress *stage,
    const Story *story, const Player *player, const EnemyGroup *enemies,
    const Projectile *arrows, const SlimeShot *shots)
{
    (void)state;(void)stage;(void)story;(void)player;(void)enemies;(void)arrows;(void)shots;
    return (GameSfxSnapshot){0};
}
static inline void GameSfx_Observe(GameSfxSystem *s, const GameSfxSnapshot *x, float dt)
{ (void)s;(void)x;(void)dt; }
#else

// Observe completed game updates. This keeps audio out of combat simulation and
// makes each cue fire once when the corresponding state actually changes.
#define GAME_SFX_CHARGE_START_DELAY 0.4f
typedef struct {
    int gameState, stage, storyMode, rescueStep, interaction;
    int hp, maxHP, potions, bowCharges, arrowType, shootingArrowType;
    unsigned int unlockedArrows;
    bool moving, sprinting, charging, chargeReady, shooting, arrowReleased;
    bool potionSelected, enchantBlade, deathStarted, dogSwoop, dogHit, knightMoving;
    bool attacking, attackHasHit;
    float potionTimer, stamina, chargeTimer;
    int ghoulCount, slimeCount, eyeCount;
    int ghoulHp[ENEMY_TYPE_CAPACITY], ghoulState[ENEMY_TYPE_CAPACITY], ghoulFrame[ENEMY_TYPE_CAPACITY];
    int slimeHp[ENEMY_TYPE_CAPACITY], slimeState[ENEMY_TYPE_CAPACITY];
    int eyeHp[ENEMY_TYPE_CAPACITY], eyeState[ENEMY_TYPE_CAPACITY];
    int kingState, kingPhase, kingHp, kingBubble, kingLaser, kingBeam, kingBounces, kingShotsFired;
    int coffinState, coffinPhase, coffinHp, swordHp, coffinCount, coffinShots, coffinReflected;
    bool coffinTrail, coffinTrailHit, swordsDropped;
    float coffinSlamEffect, coffinTimer, coffinSlashHitTime;
    int arrows[3], slimeShots, reflectedSlimeShots;
    float arrowExplosionPeak;
    int chestCount, chestState[ROOM_CHEST_CAPACITY];
    float chestBlast[ROOM_CHEST_CAPACITY], checkpointTimer;
} GameSfxSnapshot;

static inline GameSfxSnapshot GameSfx_Capture(int gameState, const StageProgress *stage,
    const Story *story, const Player *p, const EnemyGroup *g,
    const Projectile *arrows, const SlimeShot *slimeShots)
{
    GameSfxSnapshot x = {0};
    x.gameState=gameState;x.stage=stage->index;x.storyMode=story->mode;
    x.rescueStep=story->rescueStep;x.interaction=story->interaction;
    x.hp=p->hp;x.maxHP=p->maxHP;x.potions=p->healingPotions;
    x.bowCharges=p->bowCharges;x.arrowType=p->arrowType;
    x.shootingArrowType=p->shootingArrowType;x.unlockedArrows=p->unlockedArrows;
    x.moving=p->isMoving;x.sprinting=p->isSprinting;
    x.charging=p->isCharging;x.chargeTimer=p->chargeTimer;
    x.chargeReady=p->isCharging && p->chargeTimer>=PLAYER_CHARGE_TIME;
    x.shooting=p->isShooting;x.arrowReleased=p->arrowReleased;
    x.potionSelected=p->potionSelected;x.enchantBlade=p->enchantBlade;
    x.deathStarted=p->deathStarted;x.potionTimer=p->potionUseTimer;x.stamina=p->stamina;
    x.dogSwoop=story->dogSwoop>0;x.dogHit=story->dogHit;
    x.knightMoving=story->knightMoving;x.attacking=p->isAttacking;
    x.attackHasHit=p->attackHasHit;
    x.ghoulCount=g->ghoulCount;x.slimeCount=g->slimeCount;x.eyeCount=g->eyeCount;
    for(int i=0;i<g->ghoulCount;++i) {
        x.ghoulHp[i]=g->ghouls[i].hp;x.ghoulState[i]=g->ghouls[i].state;x.ghoulFrame[i]=g->ghouls[i].frame;
    }
    for(int i=0;i<g->slimeCount;++i) {
        x.slimeHp[i]=g->slimes[i].hp;x.slimeState[i]=g->slimes[i].state;
    }
    for(int i=0;i<g->eyeCount;++i) {
        x.eyeHp[i]=g->eyes[i].hp;x.eyeState[i]=g->eyes[i].state;
    }
    x.kingState=g->king.active ? g->king.state : -1;
    x.kingPhase=g->king.phase;x.kingHp=g->king.hp;x.kingShotsFired=g->king.shotsFired;
    for(int i=0;i<KING_SHOT_CAPACITY;++i) if(g->king.shots[i].active) {
        if(g->king.shots[i].type==KING_SHOT_BUBBLE) ++x.kingBubble;
        if(g->king.shots[i].type==KING_SHOT_LASER) ++x.kingLaser;
        if(g->king.shots[i].type==KING_SHOT_BEAM) ++x.kingBeam;
        x.kingBounces+=g->king.shots[i].bounces;
    }
    x.coffinState=g->coffin.active ? g->coffin.state : -1;
    x.coffinPhase=g->coffin.phase;x.coffinHp=g->coffin.hp;
    x.swordHp=g->coffin.swordHP;x.coffinCount=g->coffin.count;
    x.coffinTrail=g->coffin.trailActive;x.coffinTrailHit=g->coffin.trailHit;
    x.swordsDropped=g->coffin.swordsDropped;x.coffinSlamEffect=g->coffin.slamEffectTimer;
    x.coffinTimer=g->coffin.timer;
    x.coffinSlashHitTime=Coffin_SlashHitTime(&g->coffin);
    for(int i=0;i<COFFIN_SHOT_CAPACITY;++i) if(g->coffin.shots[i].active) {
        ++x.coffinShots;if(g->coffin.shots[i].reflected) ++x.coffinReflected;
    }
    for(int i=0;i<MAX_PROJECTILES;++i) {
        if(arrows[i].active) ++x.arrows[arrows[i].type];
        x.arrowExplosionPeak=fmaxf(x.arrowExplosionPeak,g->explosions[i].timer);
    }
    for(int i=0;i<SLIME_SHOT_CAPACITY;++i) if(slimeShots[i].active) {
        ++x.slimeShots;if(slimeShots[i].reflected) ++x.reflectedSlimeShots;
    }
    const RoomObjects *room=&stage->rooms[stage->index];
    x.chestCount=roomDefinitions[stage->index].chestCount;
    x.checkpointTimer=room->checkpointTimer;
    for(int i=0;i<x.chestCount;++i) {
        x.chestState[i]=room->chests[i].state;
        x.chestBlast[i]=room->chests[i].blastTimer;
    }
    return x;
}

typedef struct {
    GameSfx audio;
    GameSfxSnapshot last;
    bool hasSnapshot;
    float stepTimer;
    unsigned int stepNumber;
} GameSfxSystem;

static inline void GameSfx_Observe(GameSfxSystem *sys, const GameSfxSnapshot *c,
                                    float dt)
{
    if(!sys->hasSnapshot) {sys->last=*c;sys->hasSnapshot=true;return;}
    const GameSfxSnapshot *p=&sys->last;
#define PLAY(cue) GameSfx_Play(&sys->audio,(cue))
    if(c->gameState!=p->gameState) {
        if(c->gameState==2) PLAY(SFX_PAUSE);
        else if(p->gameState==2 && c->gameState==1) PLAY(SFX_RESUME);
        else if(c->gameState==0 || (p->gameState==0 && c->gameState==1)) PLAY(SFX_MENU_SELECT);
        else if(p->gameState==4 && c->gameState==1) PLAY(SFX_RESPAWN);
        else if(c->gameState==4) PLAY(SFX_GAME_OVER);
    }
    bool playing=c->gameState==1 && p->gameState==1;
    bool sameRoom=c->stage==p->stage;
    if(playing && !sameRoom) {
        PLAY(SFX_DOOR);
        if(c->stage==STAGE_ENTRANCE_ROOM && c->storyMode==STORY_NORMAL) PLAY(SFX_ENTRANCE);
        if(c->kingState==KING_INTRO) PLAY(SFX_KING_INTRO);
    }
    if(c->storyMode==STORY_RESCUE && p->storyMode!=STORY_RESCUE)
        PLAY(SFX_DOG_ARRIVAL);
    if(c->storyMode==STORY_RESCUE && c->rescueStep==3 && p->rescueStep!=3)
        PLAY(SFX_DOG_PARRY);
    if(c->storyMode==STORY_NORMAL && c->dogSwoop && !p->dogSwoop) PLAY(SFX_DOG_ARRIVAL);
    if(c->storyMode==STORY_NORMAL && c->dogHit && !p->dogHit) PLAY(SFX_DOG_ATTACK);
    if(c->storyMode==STORY_EPILOGUE && c->interaction>p->interaction)
        PLAY(c->interaction==1?SFX_DOG_INTERACT:SFX_GRAVE_INTERACT);
    if(playing && sameRoom) {
        if(c->hp<p->hp) {
            PLAY(SFX_PLAYER_HURT);
            if(c->hp<=0) PLAY(SFX_PLAYER_DEATH);
            else if(c->hp<=c->maxHP/4 && p->hp>p->maxHP/4) PLAY(SFX_LOW_HEALTH);
        }
        if(c->charging && c->chargeTimer>=GAME_SFX_CHARGE_START_DELAY &&
           (!p->charging || p->chargeTimer<GAME_SFX_CHARGE_START_DELAY))
            PLAY(SFX_CHARGE_START);
        if(c->chargeReady && !p->chargeReady) PLAY(SFX_CHARGE_READY);
        if(c->shooting && !p->shooting) PLAY(SFX_BOW_DRAW);
        if(c->arrowReleased && !p->arrowReleased) {
            PLAY(SFX_BOW_RELEASE);
            if(c->shootingArrowType==ARROW_FIRE) PLAY(SFX_FIRE_IGNITE);
        }
        if(c->potionSelected!=p->potionSelected || c->arrowType!=p->arrowType)
            PLAY(SFX_ITEM_SELECT);
        if(c->potionTimer>0 && p->potionTimer<=0) PLAY(SFX_POTION_DRINK);
        if(c->potionTimer<=0 && p->potionTimer>0 && c->hp>p->hp) PLAY(SFX_POTION_HEAL);
        if(c->checkpointTimer>p->checkpointTimer+.1f) PLAY(SFX_CHECKPOINT);
        if(c->enchantBlade && !p->enchantBlade) PLAY(SFX_ENCHANT);
        else if(c->unlockedArrows!=p->unlockedArrows) PLAY(SFX_PICKUP);
        for(int i=0;i<c->chestCount;++i) {
            if(c->chestState[i]==CHEST_OPENING && p->chestState[i]!=CHEST_OPENING)
                PLAY(roomDefinitions[c->stage].chests[i].trap?SFX_TRAP_WARNING:SFX_CHEST_OPEN);
            if(c->chestBlast[i]>p->chestBlast[i]+.1f) PLAY(SFX_TRAP_BLAST);
        }
        if(c->slimeShots>p->slimeShots) PLAY(SFX_SLIME_SHOT);
        if(c->reflectedSlimeShots>p->reflectedSlimeShots || c->coffinReflected>p->coffinReflected)
            PLAY(SFX_REFLECT);
        bool enemyDamaged=false;
        for(int i=0;i<c->ghoulCount && i<p->ghoulCount;++i) {
            if(c->ghoulState[i]==GHOUL_ATTACK && c->ghoulFrame[i]>=2 && p->ghoulFrame[i]<2)
                PLAY(SFX_ENEMY_SWING);
            if(c->ghoulHp[i]<p->ghoulHp[i]) {
                enemyDamaged=true;PLAY(c->ghoulHp[i]<=0?SFX_ENEMY_DEATH:SFX_ENEMY_HURT);
            }
        }
        for(int i=0;i<c->slimeCount && i<p->slimeCount;++i)
            if(c->slimeHp[i]<p->slimeHp[i]) {
                enemyDamaged=true;PLAY(c->slimeHp[i]<=0?SFX_SLIME_DEATH:SFX_SLIME_HURT);
            }
        for(int i=0;i<c->eyeCount && i<p->eyeCount;++i) {
            if(c->eyeState[i]==EYE_PARASITE_ATTACK && p->eyeState[i]!=EYE_PARASITE_ATTACK) PLAY(SFX_EYE_WARNING);
            if(c->eyeHp[i]<p->eyeHp[i]) {
                enemyDamaged=true;PLAY(c->eyeHp[i]<=0?SFX_EYE_DEATH:SFX_ENEMY_HURT);
            }
            if(c->eyeState[i]==EYE_PARASITE_DEAD && p->eyeState[i]==EYE_PARASITE_ATTACK) PLAY(SFX_EYE_BLAST);
        }
        if(c->kingState>=0) {
            if(c->kingHp<p->kingHp) { enemyDamaged=true;PLAY(SFX_KING_HURT); }
            if(c->kingState!=p->kingState) {
                switch(c->kingState) {
                case KING_DASH_READY: PLAY(SFX_DASH_WARNING);break;
                case KING_DASH: PLAY(SFX_DASH);break;
                case KING_BURROW: PLAY(SFX_BURROW);break;
                case KING_SLAM_MARK: PLAY(SFX_SLAM_WARNING);break;
                case KING_SLAM_LAND: PLAY(SFX_SLAM);break;
                case KING_PHASE_CHANGE: PLAY(SFX_KING_PHASE);break;
                case KING_RADIAL: case KING_SPIRAL: PLAY(SFX_LASER_WARNING);break;
                case KING_BEAM_CHARGE: PLAY(SFX_BEAM_CHARGE);break;
                case KING_BEAM_FIRE: PLAY(SFX_BEAM_FIRE);break;
                case KING_STUN: PLAY(SFX_BEAM_INTERRUPT);break;
                case KING_DEATH: PLAY(SFX_KING_DEATH);break;
                case KING_NPC: PLAY(SFX_KING_NPC);break;
                default: break;
                }
            }
            if(c->kingBubble>p->kingBubble) PLAY(SFX_BUBBLE_LAUNCH);
            if(c->kingBubble<p->kingBubble) PLAY(SFX_BUBBLE_POP);
            if(c->kingLaser>p->kingLaser || c->kingShotsFired>p->kingShotsFired) PLAY(SFX_LASER);
            if(c->kingBounces>p->kingBounces)
                PLAY(c->kingBubble?SFX_BUBBLE_BOUNCE:SFX_LASER_BOUNCE);
        }
        if(c->coffinState>=0) {
            if(c->coffinHp<p->coffinHp) { enemyDamaged=true;PLAY(SFX_ENEMY_HURT); }
            if(c->swordHp<p->swordHp) PLAY(SFX_SWORD_BLOCK);
            if(c->swordHp<=0 && p->swordHp>0) PLAY(SFX_SWORD_BREAK);
            if(c->coffinState!=p->coffinState) {
                switch(c->coffinState) {
                case FC_INTRO: PLAY(SFX_COFFIN_WAKE);break;
                case FC_EYE: PLAY(SFX_COFFIN_EYE);break;
                case FC_RELEASE: PLAY(SFX_SWORD_RELEASE);break;
                case FC_RETURN: PLAY(SFX_SWORD_RETURN);break;
                case FC_GROUND: PLAY(SFX_GROUND_WAVE);break;
                case FC_STABS: PLAY(SFX_STAB_WARNING);break;
                case FC_WALL: PLAY(SFX_WALL_BLADES);break;
                case FC_ORBIT: PLAY(SFX_ORBIT);break;
                case FC_PORTAL: case FC_TELEPORT: PLAY(SFX_TELEPORT);break;
                case FC_DASH: PLAY(SFX_COFFIN_DASH);break;
                case FC_SLASH: PLAY(SFX_STAB_WARNING);break;
                case FC_CHARGE: PLAY(SFX_COFFIN_CHARGE);break;
                case FC_CHARGED_SLASH: PLAY(SFX_STAB_WARNING);break;
                case FC_TRANSITION: PLAY(SFX_COFFIN_PHASE);break;
                case FC_STUN: PLAY(SFX_COFFIN_STUN);break;
                case FC_DEATH: PLAY(SFX_COFFIN_DEATH);break;
                default: break;
                }
            }
            if(c->coffinState==FC_GROUND && c->coffinCount>p->coffinCount) PLAY(SFX_GROUND_WAVE);
            if(c->coffinState==FC_STABS && c->coffinCount>p->coffinCount) { PLAY(SFX_STAB); PLAY(SFX_STAB_WARNING); }
            if((c->coffinState==FC_SLASH || c->coffinState==FC_CHARGED_SLASH) &&
               c->coffinTimer>=c->coffinSlashHitTime &&
               (p->coffinState!=c->coffinState || p->coffinTimer<c->coffinSlashHitTime))
                PLAY(c->coffinState==FC_CHARGED_SLASH?SFX_COFFIN_CHARGED_SLASH:SFX_COFFIN_SLASH);
            if(c->coffinSlamEffect>p->coffinSlamEffect+.1f) PLAY(SFX_COFFIN_SLAM);
            if(c->coffinTrail && !p->coffinTrail) PLAY(SFX_COFFIN_TRAIL_WARNING);
            if(c->coffinTrailHit && !p->coffinTrailHit) PLAY(SFX_TRAIL_STRIKE);
            if(c->coffinShots>p->coffinShots) PLAY(SFX_GROUND_WAVE);
        }
        if(c->swordHp<p->swordHp) enemyDamaged=true;
        if(enemyDamaged && c->attacking && c->attackHasHit && !p->attackHasHit)
            PLAY(SFX_SWORD_HIT);
        if(enemyDamaged) {
            if(c->arrows[ARROW_NORMAL]<p->arrows[ARROW_NORMAL]) PLAY(SFX_ARROW_HIT);
            if(c->arrows[ARROW_FIRE]<p->arrows[ARROW_FIRE]) PLAY(SFX_FIRE_HIT);
            if(c->arrows[ARROW_EXPLOSIVE]<p->arrows[ARROW_EXPLOSIVE]) PLAY(SFX_EXPLOSIVE_HIT);
        }
        if(c->arrowExplosionPeak>p->arrowExplosionPeak+.1f) PLAY(SFX_EXPLOSION);
    }
    if(c->gameState==1 && c->hp>0 &&
       ((c->storyMode==STORY_NORMAL && c->moving) ||
        (c->storyMode==STORY_EPILOGUE && c->knightMoving))) {
        sys->stepTimer-=dt;
        if(sys->stepTimer<=0) {
            if(c->storyMode==STORY_NORMAL && c->stage!=STAGE_ENTRANCE_ROOM)
                PLAY((SfxCue)(SFX_DUNGEON_STEP_1 + sys->stepNumber++ % 3));
            else
                PLAY(c->sprinting?SFX_RUN_STEP:SFX_STEP);
            sys->stepTimer=c->sprinting?.17f:.27f;
        }
    } else sys->stepTimer=0;
#undef PLAY
    sys->last=*c;
}

#endif
#endif
