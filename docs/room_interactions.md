# Checkpoints and chests

The route has seven rooms. The starting checkpoint room leads to Slime King, whose right door leads to the four existing combat rooms. Its top door leads to an optional reward room; that room's bottom door returns to the boss. Both boss exits stay locked through its death animation. Backtracking remains available.

## Controls and progress

- Space near a checkpoint sets the player's current position as the respawn point and replays all six activation frames. It can be used repeatedly, including after returning from another room.
- The starting room has a separate chest containing standard arrows. Checkpoints only set the respawn point. New runs start with no arrow types and zero charges. Recharge and shooting require an unlocked type; holding J skips locked types.
- Space opens a chest. After the opening animation, a second press collects its arrow type and fills the bow charges. Collected chests remain empty.
- Trap chests use the red-keyhole trap sprite when closed. All chests remain solid after opening/collecting; the player cannot walk through their base.
- The reward room contains a checkpoint, fire-arrow chest, and trap chest. The trap explodes once after its opening animation and kills the player inside a circular 32-pixel radius, even during normal hit invincibility. It uses the arrow explosion effect. Escaping the radius before the blast avoids the damage.
- On Game Over, Enter respawns at the latest checkpoint, or the starting room if none was activated. HP, stamina, and available bow charges refill. Unlocked arrow types, collected chests, spent traps, and cleared rooms persist. Unfinished enemy encounters reset. R starts a completely new run.
- Space interaction takes priority over a new melee attack and does not spend melee stamina. Release and press again for another interaction.
- Progress is stored for the current run only, not saved to disk.

## Configuration

In `src/core/stages.h`:

- `stageDefinitions`: enemy counts and door directions.
- `roomDefinitions`: checkpoint positions and chest contents. Positions are object centers in world pixels.
- Chest entries use `{x, y, arrowType, trap}`. Use `ARROW_NORMAL`, `ARROW_FIRE`, or `ARROW_EXPLOSIVE` with `false` for a loot chest; `true` creates a trap and ignores the arrow type.
- `STAGE_START_ROOM`, `STAGE_BOSS_ROOM`, and `STAGE_REWARD_ROOM` identify the three special rooms.

In `src/core/room_objects.h`, adjust `ROOM_INTERACT_RADIUS`, `CHEST_TRAP_RADIUS`, `ROOM_CHEST_CAPACITY`, and the `CHEST_BODY_*` collision dimensions. The default chest body is 24 × 20 pixels, with its center 4 pixels below the art center.

Art uses `assets/sprites/map/checkpoint.h` and `chest.h` at native pixel size. The chest export has closed/open poses, not a full multi-frame lid animation: normal chests transition from the closed pose to their loot pose; trap chests play the two exported trap frames. Collected chests use the empty open pose (`cheast_frame_3`).

Rebuild with `run_game.cmd` after changing settings.
