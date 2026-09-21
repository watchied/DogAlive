#ifndef ROOM_INTERACTIONS_H
#define ROOM_INTERACTIONS_H
#include "src/core/stages.h"

// Return true to consume Space before the melee input sees it.
static inline bool Room_Interact(StageProgress *s, Player *p)
{
    if (p->hp <= 0 || p->isAttacking || p->isCharging || p->isShooting) return false;
    const RoomDefinition *d = &roomDefinitions[s->index];
    RoomObjects *r = &s->rooms[s->index];
    if (d->checkpoint && Room_Near(p, d->checkpointX, d->checkpointY)) {
        s->hasCheckpoint = true;
        s->checkpointStage = s->index;
        s->respawnX = p->x; s->respawnY = p->y;
        r->checkpointActivated = true;
        r->checkpointTimer = Explosion_Duration(checkpoint_frames_duration_ms, CHECKPOINT_FRAMES_COUNT);
        return true;
    }
    for (int i = 0; i < d->chestCount; ++i) {
        const ChestDefinition *c = &d->chests[i];
        RoomChest *chest = &r->chests[i];
        if (!Room_Near(p, c->x, c->y)) continue;
        if (chest->state == CHEST_CLOSED) {
            chest->state = CHEST_OPENING;
            chest->timer = Explosion_Duration(cheast_trap_chest_duration_ms, CHEAST_TRAP_CHEST_COUNT);
        } else if (chest->state == CHEST_OPEN && !c->trap) {
            if (c->enchantBlade) p->enchantBlade = true;
            else Player_UnlockArrow(p, c->arrow);
            chest->state = CHEST_COLLECTED;
        }
        return true;
    }
    return false;
}

static inline void Room_Update(StageProgress *s, Player *p, float dt)
{
    const RoomDefinition *d = &roomDefinitions[s->index];
    RoomObjects *r = &s->rooms[s->index];
    r->checkpointTimer = fmaxf(0, r->checkpointTimer - dt);
    for (int i = 0; i < d->chestCount; ++i) {
        RoomChest *chest = &r->chests[i];
        chest->blastTimer = fmaxf(0, chest->blastTimer - dt);
        if (chest->state != CHEST_OPENING) continue;
        chest->timer = fmaxf(0, chest->timer - dt);
        if (chest->timer > 0) continue;
        chest->state = CHEST_OPEN;
        if (d->chests[i].trap) {
            chest->state = CHEST_COLLECTED;
            chest->blastTimer = ARROW_EXPLOSION_TIME;
            // This trap is lethal even during the ordinary damage grace period.
            if (p->hp > 0 && Room_TrapHits(p, &d->chests[i])) {
                p->hp = 0;
                p->hitFlashTimer = PLAYER_HIT_FLASH_TIME;
            }
        }
    }
}

static inline void Stage_Respawn(StageProgress *s, Player *p, EnemyGroup *enemies,
    Projectile *arrows, SlimeShot *shots, RunningEffect *effect)
{
    s->saved[s->index] = *enemies;
    s->visited[s->index] = true;
    // Cleared rooms stay cleared. Unfinished fights restart at full strength.
    for (int i = 0; i < STAGE_COUNT; ++i) {
        if (s->visited[i] && Stage_EnemiesAlive(&s->saved[i]) > 0) s->visited[i] = false;
        for (int j = 0; j < roomDefinitions[i].chestCount; ++j) {
            RoomChest *c = &s->rooms[i].chests[j];
            if (c->state == CHEST_OPENING) *c = (RoomChest){0};
            c->blastTimer = 0;
        }
        s->rooms[i].checkpointTimer = 0;
    }
    Player previous = *p;
    Player_Init(p);
    p->enchantBlade = previous.enchantBlade;
    p->maxHP = previous.maxHP; p->hp = p->maxHP;
    p->maxStamina = previous.maxStamina; p->stamina = p->maxStamina;
    p->attackDamage = previous.attackDamage; p->arrowDamage = previous.arrowDamage;
    p->speed = previous.speed; p->arrowSpeed = previous.arrowSpeed;
    p->shootCooldown = previous.shootCooldown; p->arrowLifetime = previous.arrowLifetime;
    p->unlockedArrows = previous.unlockedArrows; p->arrowType = previous.arrowType;
    p->bowCharges = p->unlockedArrows ? PLAYER_BOW_MAX_CHARGES : 0;
    p->invincibilityTimer = PLAYER_INVINCIBILITY_TIME;
    s->completed = false;
    s->index = s->hasCheckpoint ? s->checkpointStage : STAGE_START_ROOM;
    Stage_Load(s->index, p, enemies, arrows, shots, effect);
    if (s->visited[s->index]) *enemies = s->saved[s->index];
    if (s->hasCheckpoint) { p->x = s->respawnX; p->y = s->respawnY; }
    Stage_ClampPlayer(p);
    Room_Collide(&roomDefinitions[s->index], p);
}
#endif
