#ifndef ROOM_OBJECTS_DRAW_H
#define ROOM_OBJECTS_DRAW_H
#include "assets/sprites/map/broken_coffin.h"
#include "src/core/room_interactions.h"

static inline void Room_DrawSprite(SDL_Renderer *renderer, float cx, float cy,
    const uint16_t *sprite, int width, int height)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uint16_t c = sprite[y * width + x];
            if (c == 0x07E0) continue;
            SDL_SetRenderDrawColor(renderer, ((c >> 11) & 31) * 255 / 31,
                ((c >> 5) & 63) * 255 / 63, (c & 31) * 255 / 31, 255);
            SDL_FRect pixel = {cx - width / 2.0f + x, cy - height / 2.0f + y, 1, 1};
            SDL_RenderFillRect(renderer, &pixel);
        }
}

static inline void Room_Draw(SDL_Renderer *renderer, const StageProgress *s, const Player *p)
{
    const RoomDefinition *d = &roomDefinitions[s->index];
    const RoomObjects *r = &s->rooms[s->index];
    for(int i=0;i<d->objectCount;++i) {
        const RoomMapObject *o=&d->objects[i];
        if(o->width<=0 || o->height<=0) continue;
        int frame=o->frame;
        if(frame<0 || frame>=BROKEN_COFFIN_FRAMES_COUNT) frame=0;
        const uint16_t *pixels=broken_coffin_frames[frame];
        int width=(int)ceilf(o->width),height=(int)ceilf(o->height);
        float angle=(float)(o->angle*0.01745329252),cs=cosf(angle),sn=sinf(angle);
        for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
            uint16_t color=pixels[(y*BROKEN_COFFIN_HEIGHT/height)*BROKEN_COFFIN_WIDTH+x*BROKEN_COFFIN_WIDTH/width];
            if(color==0x07E0) continue;
            SDL_SetRenderDrawColor(renderer,((color>>11)&31)*255/31,((color>>5)&63)*255/63,(color&31)*255/31,255);
            float dx=x-width/2.0f+0.5f,dy=y-height/2.0f+0.5f;
            SDL_FRect pixel={o->x+dx*cs-dy*sn-0.5f,o->y+dx*sn+dy*cs-0.5f,1,1};
            SDL_RenderFillRect(renderer,&pixel);
        }
    }
    const char *prompt = NULL;
    if (d->checkpoint) {
        int frame = Explosion_Frame(r->checkpointTimer, checkpoint_frames_duration_ms, CHECKPOINT_FRAMES_COUNT);
        if (frame < 0) frame = r->checkpointActivated ? CHECKPOINT_FRAMES_COUNT - 1 : 0;
        Room_DrawSprite(renderer, d->checkpointX, d->checkpointY, checkpoint_frames[frame], CHECKPOINT_WIDTH, CHECKPOINT_HEIGHT);
        if (Room_Near(p, d->checkpointX, d->checkpointY))
            prompt = "Space: Set checkpoint";
    }
    for (int i = 0; i < d->chestCount; ++i) {
        const ChestDefinition *c = &d->chests[i];
        const RoomChest *chest = &r->chests[i];
        const uint16_t *loot = c->enchantBlade ? cheast_enchant_blade_chest[0] : c->arrow == ARROW_FIRE ? cheast_fire_arrow_chest[0] :
            c->arrow == ARROW_EXPLOSIVE ? cheast_bomb_arrow_chest[0] : cheast_arrow_chest[0];
        const uint16_t *sprite = c->trap ? cheast_trap_chest[0] : cheast_chest[0];
        if (chest->state == CHEST_OPENING) {
            int frame = Explosion_Frame(chest->timer, cheast_trap_chest_duration_ms, CHEAST_TRAP_CHEST_COUNT);
            if (c->trap) sprite = cheast_trap_chest[frame < 0 ? CHEAST_TRAP_CHEST_COUNT - 1 : frame];
            else sprite = frame <= 0 ? cheast_chest[0] : loot;
        } else if (chest->state == CHEST_COLLECTED) sprite = cheast_frame_3;
        else if (chest->state == CHEST_OPEN) sprite = loot;
        Room_DrawSprite(renderer, c->x, c->y, sprite, CHEAST_WIDTH, CHEAST_HEIGHT);
        int frame = Explosion_Frame(chest->blastTimer, arrow_explosion_8frames_dark_frames_duration_ms,
            ARROW_EXPLOSION_8FRAMES_DARK_FRAMES_COUNT);
        if (frame >= 0) Room_DrawSprite(renderer, c->x, c->y, arrow_explosion_8frames_dark_frames[frame],
            ARROW_EXPLOSION_8FRAMES_DARK_WIDTH, ARROW_EXPLOSION_8FRAMES_DARK_HEIGHT);
        if (!prompt && Room_Near(p, c->x, c->y)) {
            if (chest->state == CHEST_CLOSED) prompt = "Space: Open chest";
            else if (chest->state == CHEST_OPEN) prompt = c->enchantBlade ? "Space: Take enchant blade" : c->arrow == ARROW_FIRE ? "Space: Take fire arrows" :
                c->arrow == ARROW_EXPLOSIVE ? "Space: Take bomb arrows" : "Space: Take arrows";
        }
    }
    if (prompt && p->hp > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 235, 170, 255);
        SDL_RenderDebugText(renderer, 52, GAME_HEIGHT - 22, prompt);
    }
}
#endif
