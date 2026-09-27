# Flesh Coffin rescue and ending

## Route
Slime King (room 4) now leads to Dungeon Entrance (Night), then the old room 6.
Its optional reward room remains above the Slime King.
The entrance is internal index 10. Its ground is walkable below y=192; the cave mouth is x=132..176, y>=164.
Walk into the mouth to proceed. The left foreground returns to the Slime King.
The numbered debug keys retain their old mappings; 0 still opens Flesh Coffin.

## Rescue
At 50% boss HP the health threshold is held, so a large hit cannot skip the cutscene.
Gameplay, projectiles, enemy AI and the player's controls freeze.
Three cinematic sword images restrain the player (they do not restore the destroyed sword HP pool).
The boss teleports nearby, charges, and the Dog rushes in to parry.
After a brief stun, combat resumes. The Dog is visual/helper state, never an enemy target or collision body.
It swoops toward the boss every 7 seconds and deals 30 damage on contact when the boss is vulnerable.
Death/checkpoint respawn restarts the unfinished encounter and rescue. Pausing freezes the sequence.

## Return and ending
After the boss's death animation the game displays Return Home.
Backtrack through the cleared rooms. Sprint is disabled; the Dog follows at player walking speed.
A subtle blue pulse overlays the scene. Reaching the entrance automatically begins dead_true_end.
After its exported frame timings finish, the Dog approaches, then the scene fades to black.
The six ED frames cross-fade over 25 seconds with Ending song.mp3.

## Knight epilogue
The background begins on the final ED frame immediately before the Interact tag.
Move the Knight using WASD/arrows. Approach the Dog on the left and press Space.
Then approach the grave and press Space again. Each fresh press plays the Knight Interact pose and advances one background frame.
After both interactions, remain still for 5 seconds: the camera pans to the sky and fades to white.
Ending song.mp3 restarts and loops on the white ending. R starts a new run.

## Tuning and checks
src/story/story_config.h: Dog damage/interval, cutscene durations, ED length, idle/pan/fade times, Dog/grave interaction locations.
src/story/story.h: scripted sequence, background texture loading and drawing.
src/core/stages.h: entrance route, portal and walkable boundaries.
src/audio/ending_music.h: ending song path and volume (Windows MP3 playback).
tests/story_test.c: 50% guard, input-lock state, rescue, helper damage, walking, entrance bounds, ED timing and interaction order.
tests/ending_music_test.c: muted real MP3 playback, loop, pause/resume, reset and cleanup; Windows audio requires running outside the tool sandbox.
