#ifndef ROOM_OBJECTS_H
#define ROOM_OBJECTS_H
#include <math.h>
#include "src/player/player.h"
#include "src/effects/explosion_animation.h"
#include "assets/sprites/map/checkpoint.h"
#include "assets/sprites/map/chest.h"

#define ROOM_INTERACT_RADIUS 30.0f
#define CHEST_TRAP_RADIUS 32.0f
#define ROOM_CHEST_CAPACITY 2
#define CHEST_BODY_WIDTH 24.0f
#define CHEST_BODY_HEIGHT 20.0f
#define CHEST_BODY_OFFSET_Y 4.0f
typedef enum { CHEST_CLOSED, CHEST_OPENING, CHEST_OPEN, CHEST_COLLECTED } ChestState;
typedef struct { float x, y; ArrowType arrow; bool trap; bool enchantBlade; } ChestDefinition;
typedef struct { bool checkpoint; float checkpointX, checkpointY; int chestCount; ChestDefinition chests[ROOM_CHEST_CAPACITY]; } RoomDefinition;
typedef struct { ChestState state; float timer, blastTimer; } RoomChest;
typedef struct { float checkpointTimer; bool checkpointActivated; RoomChest chests[ROOM_CHEST_CAPACITY]; } RoomObjects;

static inline SDL_FRect Room_ChestBody(const ChestDefinition *c)
{
    return (SDL_FRect){c->x - CHEST_BODY_WIDTH / 2, c->y + CHEST_BODY_OFFSET_Y - CHEST_BODY_HEIGHT / 2,
        CHEST_BODY_WIDTH, CHEST_BODY_HEIGHT};
}
// Resolve overlap after knockback, spawning, or movement. Open/empty chests remain solid.
static inline void Room_Collide(const RoomDefinition *room, Player *p)
{
    for (int i = 0; i < room->chestCount; ++i) {
        SDL_FRect b = Room_ChestBody(&room->chests[i]);
        float dx = p->x + ACTOR_HALF_SIZE - (b.x + b.w / 2);
        float dy = p->y + ACTOR_HALF_SIZE - (b.y + b.h / 2);
        float ox = (PLAYER_HITBOX_SIZE + b.w) / 2 - fabsf(dx);
        float oy = (PLAYER_HITBOX_SIZE + b.h) / 2 - fabsf(dy);
        if (ox <= 0 || oy <= 0) continue;
        if (ox < oy) p->x += dx < 0 ? -ox : ox;
        else p->y += dy < 0 ? -oy : oy;
    }
}
// Sweep each movement axis so sprinting cannot cross a chest in one frame.
static inline void Room_BlockMovement(const RoomDefinition *room, Player *p, float oldX, float oldY)
{
    float targetY = p->y;
    p->y = oldY;
    for (int i = 0; i < room->chestCount; ++i) {
        SDL_FRect b = Room_ChestBody(&room->chests[i]);
        SDL_FRect body = Player_Body(p);
        if (body.y >= b.y + b.h || body.y + body.h <= b.y) continue;
        if (oldX + PLAYER_HITBOX_OFFSET + body.w <= b.x && body.x + body.w > b.x) p->x = b.x - body.w - PLAYER_HITBOX_OFFSET;
        else if (oldX + PLAYER_HITBOX_OFFSET >= b.x + b.w && body.x < b.x + b.w) p->x = b.x + b.w - PLAYER_HITBOX_OFFSET;
    }
    p->y = targetY;
    for (int i = 0; i < room->chestCount; ++i) {
        SDL_FRect b = Room_ChestBody(&room->chests[i]);
        SDL_FRect body = Player_Body(p);
        if (body.x >= b.x + b.w || body.x + body.w <= b.x) continue;
        if (oldY + PLAYER_HITBOX_OFFSET + body.h <= b.y && body.y + body.h > b.y) p->y = b.y - body.h - PLAYER_HITBOX_OFFSET;
        else if (oldY + PLAYER_HITBOX_OFFSET >= b.y + b.h && body.y < b.y + b.h) p->y = b.y + b.h - PLAYER_HITBOX_OFFSET;
    }
    Room_Collide(room, p);
}

static inline bool Room_Near(const Player *p, float x, float y)
{
    float dx = p->x + ACTOR_HALF_SIZE - x, dy = p->y + ACTOR_HALF_SIZE - y;
    return dx * dx + dy * dy <= ROOM_INTERACT_RADIUS * ROOM_INTERACT_RADIUS;
}
static inline bool Room_TrapHits(const Player *p, const ChestDefinition *chest)
{
    float dx = chest->x - fmaxf(p->x + PLAYER_HITBOX_OFFSET, fminf(chest->x, p->x + PLAYER_HITBOX_OFFSET + PLAYER_HITBOX_SIZE));
    float dy = chest->y - fmaxf(p->y + PLAYER_HITBOX_OFFSET, fminf(chest->y, p->y + PLAYER_HITBOX_OFFSET + PLAYER_HITBOX_SIZE));
    return dx * dx + dy * dy <= CHEST_TRAP_RADIUS * CHEST_TRAP_RADIUS;
}
#endif
