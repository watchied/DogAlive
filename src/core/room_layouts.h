#ifndef ROOM_LAYOUTS_H
#define ROOM_LAYOUTS_H
// Included by stages.h after the room IDs are defined.
// Change each row below to arrange props; coordinates are visual centers.
// For custom size/collision, replace COFFIN_PROP with a designated initializer:
// {.x=80,.y=40,.frame=2,.width=48,.height=48,.angle=90,
//  .solid=true,.hitWidth=24,.hitHeight=20,.hitOffsetX=0,.hitOffsetY=8}
// Set solid=false for decoration only. Keep objectCount equal to the number of rows.
// COFFIN_PROP(variant, centerX, centerY, clockwiseDegrees)
#define COFFIN_PROP(variant,cx,cy,degrees) { .x=(cx),.y=(cy),.frame=(variant),.angle=(degrees), .width=64,.height=64,.solid=true,.hitWidth=48,.hitHeight=48 }
// Object positions are centers in world pixels. Trap chests never grant an arrow.
static const RoomDefinition roomDefinitions[STAGE_COUNT] = {
    [STAGE_START_ROOM] = {.checkpoint = true, .checkpointX = 100, .checkpointY = 120,
        .chestCount = 1, .chests = {{200, 120, ARROW_NORMAL, false, false}}},
    [STAGE_REWARD_ROOM] = {.checkpoint = true, .checkpointX = 160, .checkpointY = 175,
        .chestCount = 2, .chests = {{100, 95, ARROW_FIRE, false, false}, {220, 95, ARROW_NORMAL, true, false}}},
    [5] = {.chestCount = 1, .chests = {{270, 65, ARROW_NORMAL, false, true}}},
    [STAGE_COFFIN_ROOM] = {.objectCount=10, .objects={
        COFFIN_PROP(0,48,28,0), COFFIN_PROP(1,104,14,0), COFFIN_PROP(2,160,28,0),
        COFFIN_PROP(3,216,28,0), COFFIN_PROP(4,272,28,0),
        COFFIN_PROP(5,48,212,0), COFFIN_PROP(6,104,212,0), COFFIN_PROP(7,160,212,0),
        COFFIN_PROP(0,216,212,0), COFFIN_PROP(1,140,14,0)
    }}
};

#undef COFFIN_PROP
#endif
