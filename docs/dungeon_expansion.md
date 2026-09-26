# Dungeon rooms and enemy variants

The existing starting checkpoint is retained. Keyboard shortcuts while playing:
1: starting checkpoint; 2: dirt room 1 (2 goblins, 1 slime); 3: dirt room 2 (3 goblins, 2 slimes); 4: Slime King; 5: reward branch; 6-9: existing combat rooms; 0: final Flesh Coffin.
Shortcuts preserve room progress, reset travelling projectiles and work independently of exit locks.
The normal route skips room 5; enter that reward branch through the Slime King room's top door after defeating him.
Flesh Coffin is the final gate before victory.

Goblin uses the Ghoul combat state machine with its own sprites and a 0.75x original melee reach. Bloodless Ghoul now has 1.25x original reach. Slime uses the ranged state machine and existing slime_bullet, with its own sprites and no blood trail. Variant flags are saved with each room; damage, arrows, burns, parry, separation and death use the shared combat pipeline.

Flesh Slime leaves blood dust every FLESH_SLIME_DUST_SPACING pixels of walking. Dust uses the Coffin dust lifetime/damage/tick settings, a shared contact timer to prevent stacking, and expires even after the enemy dies.

All floor tiles come from map_object arrays in assets/sprites/map/map_ground.h. The exported grass tag is named map_object_glass. Grass is restricted to the border of dirt rooms; interiors select dirt variant 1 with 60% probability. Random layouts remain stable for the run.

The final room has ten fixed broken_coffin objects, five per row. Their positions and solid rectangles are configured in src/core/room_layouts.h, roomDefinitions[STAGE_COFFIN_ROOM]. Player movement uses swept collision; enemy bodies also resolve against these blocks. Objects render at 32 by 32.

Charged parry snapshots its type when successful and draws hit_spark at 1.5x size, even after the charged swing ends.


## Editing room objects

Open src/core/room_layouts.h. COFFIN_PROP(variant, x, y) places a solid 32x32 coffin centered at x,y. Variant is zero-based (0-7). Each entry can instead use:
{.x=80,.y=40,.frame=2,.width=48,.height=48,.angle=90,
 .solid=true,.hitWidth=24,.hitHeight=20,.hitOffsetX=0,.hitOffsetY=8}
Angles rotate the art clockwise; collision dimensions remain aligned to the map axes, so swap hitWidth/hitHeight when needed.
Set solid=false for decoration. Keep objectCount equal to the number of entries (maximum 32).
Checkpoint positions and chest contents/positions are in the same file.

Grass now uses fixed edge variants (top/bottom/left/right) with the corner sprite rotated for each corner. Only interior dirt is randomized.
Ordinary Slime shots use slime_bullet_slime; Flesh Slime shots use slime_bullet_flesh_slime. The projectile retains its visual type after reflection.
