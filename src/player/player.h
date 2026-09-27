#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "src/core/game_config.h"
#include "src/effects/melee_timing.h"

#define PLAYER_START_X 100.0f
#define PLAYER_START_Y 100.0f
// Starting equipment: 0 = locked, 1 = owned. Applied when starting a new run.
#define PLAYER_START_NORMAL_ARROW 0
#define PLAYER_START_FIRE_ARROW 0
#define PLAYER_START_EXPLOSIVE_ARROW 0
#define PLAYER_START_ENCHANT_BLADE 1
#define PLAYER_DEFAULT_SPEED 40.0f
#define PLAYER_MAX_HP 10000
#define PLAYER_HITBOX_SIZE 13.0f
#define PLAYER_HITBOX_OFFSET ((ACTOR_SIZE - PLAYER_HITBOX_SIZE) / 2.0f)
#define PLAYER_START_POTIONS 2
#define PLAYER_POTION_HEAL 60
#define PLAYER_POTION_USE_TIME 1.5f
#define PLAYER_POTION_MOVE_MULTIPLIER 0.7f
#define PLAYER_COLLISION_GRACE_TIME 0.5f
#define PLAYER_HIT_FLASH_TIME 0.16f
#define PLAYER_INVINCIBILITY_TIME 0.3f
#define PLAYER_HIT_KNOCKBACK 2.0f
#define PLAYER_MAX_STAMINA 120
#define PLAYER_SPRINT_MULTIPLIER 1.7f
#define PLAYER_STAMINA_DRAIN 20.0f
#define PLAYER_MELEE_REACH 12.0f // Melee reach in world pixels; affects attacks and slime parries.
#define PLAYER_STAMINA_REGEN 30.0f
#define PLAYER_SPRINT_MIN_STAMINA 20.0f
#define PLAYER_DOUBLE_TAP_TIME 0.25f
#define PLAYER_ATTACK_DAMAGE 3000
#define PLAYER_MELEE_STAMINA_COST 15.0f // Stamina spent once when starting a slash.
#define PLAYER_CHARGE_TIME 1.5f
#define PLAYER_CHARGE_REACH 64.0f // Forward distance from the player's center.
#define PLAYER_CHARGE_WIDTH 80.0f // Width perpendicular to the facing direction.
#define PLAYER_CHARGE_MOVE_MULTIPLIER 0.7f
#define PLAYER_CHARGE_DAMAGE_MULTIPLIER 2.0f
#define PLAYER_CHARGE_STAMINA_COST 15.0f
#define PLAYER_PARRY_START_TIME 0.25f
#define PLAYER_PARRY_END_TIME 0.6f
#define PLAYER_PARRY_FLASH_TIME 0.16f
#define PLAYER_PARRY_EFFECT_TIME 0.6f
#define PLAYER_ARROW_DAMAGE 20
#define PLAYER_ARROW_SPEED 200.0f
#define PLAYER_SHOOT_COOLDOWN 3.0f
#define PLAYER_BOW_MAX_CHARGES 3
#define FIRE_ARROW_BURN_MULTIPLIER 1.5f
#define FIRE_ARROW_TICKS 6
#define FIRE_ARROW_TICK_TIME 0.5f
#define EXPLOSIVE_ARROW_MULTIPLIER 1.75f
#define EXPLOSIVE_ARROW_RADIUS 24.0f
#define PLAYER_ARROW_LIFETIME 1.5f
#define PLAYER_SHOOT_FRAME_COUNT 5
#define PLAYER_SHOOT_RELEASE_FRAME (PLAYER_SHOOT_FRAME_COUNT / 2)
#define PLAYER_SHOOT_FRAME_TIME 0.05f
#define PLAYER_ATTACK_HIT_FRAME MELEE_HIT_FRAME

typedef enum
{
    PLAYER_DOWN,
    PLAYER_UP,
    PLAYER_LEFT,
    PLAYER_RIGHT
} PlayerDirection;

typedef enum { ARROW_NORMAL, ARROW_FIRE, ARROW_EXPLOSIVE } ArrowType;
static inline int Arrow_ChargeCost(ArrowType type) { return (int)type + 1; }

typedef struct
{
    float x;
    float y;
    float speed;
    int hp;
    bool deathStarted;
    bool deathFinished;
    int deathFrame;
    float deathTimer;
    float hitFlashTimer;
    bool chargedParryEffect;
    float parryEffectTimer, parryEffectX, parryEffectY;
    float collisionGraceTimer;
    float invincibilityTimer;
    int maxHP;
    int attackDamage;
    int arrowDamage;
    float stamina;
    float maxStamina;
    bool isSprinting;
    bool walkOnly;
    unsigned int moveWasDown;
    unsigned int sprintButton;
    float tapRemaining[4];
    float arrowSpeed;
    float shootCooldown;
    float arrowLifetime;
    float shootTimer;
    int bowCharges;
    int healingPotions;
    float potionUseTimer;
    bool potionSelected;
    unsigned int unlockedArrows; // One bit per ArrowType; a new run starts without a bow.
    ArrowType arrowType;
    ArrowType shootingArrowType; // Snapshot so switching cannot change an already paid shot.
    bool isShooting;
    bool arrowReleased;
    float aimX, aimY;
    bool facingLeft;
    PlayerDirection direction;
    

    // สำหรับระบบอนิเมชั่น
    int currentFrame;   // เฟรมปัจจุบัน (0, 1, 2, 3)
    float animTimer;    // ตัวนับเวลาสลับเฟรม
    bool isMoving;      // กำลังเคลื่อนที่อยู่หรือไม่
    bool isMovingRight; // กำลังเคลื่อนที่ไปทางขวาหรือไม่
    bool isMovingLeft;  // กำลังเคลื่อนที่ไปทางซ้ายหรือไม่
    bool isMovingUp;    // กำลังเคลื่อนที่ขึ้นหรือไม่
    bool isMovingDown;  // กำลังเคลื่อนที่ลงหรือไม่
    bool isAttacking;
    bool attackWasDown;
    bool attackHasHit;
    unsigned int meleeSwingId;
    bool enchantBlade;
    bool isCharging;
    bool chargedAttack;
    float chargeTimer;
} Player;

static inline void Player_ShowParry(Player *p, float enemyX, float enemyY)
{
    p->parryEffectTimer = PLAYER_PARRY_EFFECT_TIME;
    p->chargedParryEffect = p->chargedAttack;
    p->parryEffectX = (p->x + ACTOR_HALF_SIZE + enemyX) / 2;
    p->parryEffectY = (p->y + ACTOR_HALF_SIZE + enemyY) / 2;
}

static inline SDL_FRect Player_Body(const Player *p)
{
    return (SDL_FRect){p->x + PLAYER_HITBOX_OFFSET, p->y + PLAYER_HITBOX_OFFSET,
        PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE};
}
static inline SDL_FRect Player_ExpandedBody(const Player *p, float x, float y)
{
    SDL_FRect b = Player_Body(p);
    b.x -= x; b.y -= y; b.w += 2*x; b.h += 2*y;
    return b;
}

// Player melee hitbox extends from the front edge of the body.
static inline SDL_FRect Player_AttackBox(const Player *p)
{
    if (p->chargedAttack) {
        float cx = p->x + ACTOR_HALF_SIZE, cy = p->y + ACTOR_HALF_SIZE;
        switch (p->direction) {
        case PLAYER_UP: return (SDL_FRect){cx - PLAYER_CHARGE_WIDTH / 2, cy - PLAYER_CHARGE_REACH, PLAYER_CHARGE_WIDTH, PLAYER_CHARGE_REACH};
        case PLAYER_DOWN: return (SDL_FRect){cx - PLAYER_CHARGE_WIDTH / 2, cy, PLAYER_CHARGE_WIDTH, PLAYER_CHARGE_REACH};
        case PLAYER_LEFT: return (SDL_FRect){cx - PLAYER_CHARGE_REACH, cy - PLAYER_CHARGE_WIDTH / 2, PLAYER_CHARGE_REACH, PLAYER_CHARGE_WIDTH};
        default: return (SDL_FRect){cx, cy - PLAYER_CHARGE_WIDTH / 2, PLAYER_CHARGE_REACH, PLAYER_CHARGE_WIDTH};
        }
    }
    switch (p->direction)
    {
    case PLAYER_UP:
        return (SDL_FRect){p->x, p->y - PLAYER_MELEE_REACH, ACTOR_SIZE, PLAYER_MELEE_REACH};
    case PLAYER_DOWN:
        return (SDL_FRect){p->x, p->y + ACTOR_SIZE, ACTOR_SIZE, PLAYER_MELEE_REACH};
    case PLAYER_LEFT:
        return (SDL_FRect){p->x - PLAYER_MELEE_REACH, p->y, PLAYER_MELEE_REACH, ACTOR_SIZE};
    default:
        return (SDL_FRect){p->x + ACTOR_SIZE, p->y, PLAYER_MELEE_REACH, ACTOR_SIZE};
    }
}

static inline void Player_Init(Player *player)
{
    *player = (Player){
        .x = PLAYER_START_X,
        .y = PLAYER_START_Y,
        .speed = PLAYER_DEFAULT_SPEED,
        .hp = PLAYER_MAX_HP,
        .maxHP = PLAYER_MAX_HP,
        .stamina = PLAYER_MAX_STAMINA,
        .maxStamina = PLAYER_MAX_STAMINA,
        .attackDamage = PLAYER_ATTACK_DAMAGE,
        .arrowDamage = PLAYER_ARROW_DAMAGE,
        .arrowSpeed = PLAYER_ARROW_SPEED,
        .shootCooldown = PLAYER_SHOOT_COOLDOWN,
        .bowCharges = 0,
        .healingPotions = PLAYER_START_POTIONS,
        .potionSelected = true,
        .arrowLifetime = PLAYER_ARROW_LIFETIME,
        .aimX = 1.0f,
        .aimY = 0.0f,
        .direction = PLAYER_RIGHT};
    player->unlockedArrows = (PLAYER_START_NORMAL_ARROW ? 1u << ARROW_NORMAL : 0) |
        (PLAYER_START_FIRE_ARROW ? 1u << ARROW_FIRE : 0) |
        (PLAYER_START_EXPLOSIVE_ARROW ? 1u << ARROW_EXPLOSIVE : 0);
    player->enchantBlade = PLAYER_START_ENCHANT_BLADE != 0;
    if (player->unlockedArrows) {
        player->potionSelected = false;
        player->bowCharges = PLAYER_BOW_MAX_CHARGES;
        // Select the first owned type, even when normal arrows are disabled.
        for (int type = ARROW_NORMAL; type <= ARROW_EXPLOSIVE; ++type)
            if (player->unlockedArrows & (1u << type)) {
                player->arrowType = (ArrowType)type;
                break;
            }
    }
}

static inline int Player_MeleeDamage(const Player *p)
{
    return (int)(p->attackDamage * (p->chargedAttack ? PLAYER_CHARGE_DAMAGE_MULTIPLIER : 1.0f));
}
static inline bool Player_ParryActive(const Player *p)
{
    float time = p->currentFrame * MELEE_ACTOR_FRAME_TIME + p->animTimer;
    return p->hp > 0 && p->isAttacking && time >= PLAYER_PARRY_START_TIME && time < PLAYER_PARRY_END_TIME;
}

static inline bool Player_HasArrow(const Player *p, ArrowType type)
{
    return (p->unlockedArrows & (1u << type)) != 0;
}

static inline void Player_UnlockArrow(Player *p, ArrowType type)
{
    if (Player_HasArrow(p, type)) return;
    p->unlockedArrows |= 1u << type;
    p->arrowType = type;
    p->potionSelected = false;
    p->bowCharges = PLAYER_BOW_MAX_CHARGES;
    p->shootTimer = 0;
}

static inline bool Player_UsePotion(Player *p)
{
    if(p->hp<=0 || p->hp>=p->maxHP || p->healingPotions<=0 || p->potionUseTimer>0 || p->isAttacking || p->isCharging || p->isShooting) return false;
    --p->healingPotions;
    p->potionUseTimer = PLAYER_POTION_USE_TIME;
    p->isSprinting = false;
    return true;
}

static inline void Player_UpdatePotion(Player *p, float dt)
{
    if (p->hp <= 0) { p->potionUseTimer = 0; return; }
    if (p->potionUseTimer <= 0 || dt <= 0) return;
    p->potionUseTimer -= dt;
    if (p->potionUseTimer <= 0) {
        p->potionUseTimer = 0;
        p->hp = p->maxHP-p->hp < PLAYER_POTION_HEAL ? p->maxHP : p->hp+PLAYER_POTION_HEAL;
    }
}

#endif
