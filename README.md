# Dog Alive

game for microcontroler project

# how to play:

1.just build

2.type command "./bin/game.exe" or just click game.exe

## controls

- Start with 2 healing potions. Hold J to cycle through unlocked arrows and the potion; tap J to use the selected item. Drinking takes 2 seconds: movement is reduced to 70% and sprinting is disabled, then 50 HP is restored (capped at max HP). Pausing freezes the timer; dying cancels the pending heal. Full HP does not consume a potion. Remaining potions persist on checkpoint respawn; a new run restores the starting count. Tune `PLAYER_START_POTIONS`, `PLAYER_POTION_HEAL`, `PLAYER_POTION_USE_TIME`, and `PLAYER_POTION_MOVE_MULTIPLIER` in `src/player/player.h`.

- w a s d to move and double tap to run(can not run and turn back to run)
- press spacebar near a checkpoint/chest to interact; elsewhere it attacks
- j to fire arrow and hold j to change arrow type
- Enter on Game Over respawns at the latest checkpoint; R starts a new run

## Slime King boss

For testing, the route now passes through **Flesh Coffin** before Slime King. Approach the dormant boss in the upper-right corner to begin. See [Flesh Coffin settings and mechanics](docs/flesh_coffin.md).

Start in the checkpoint room, then enter the two-phase Slime King encounter. Defeat it and wait for the NPC transformation to open two exits: the top door leads to a reward room with another checkpoint, a fire-arrow chest, and a trap chest; the right door continues to the next combat room.
have 2 phase

You start without arrows. Collect standard arrows from the chest in the starting room before the boss. Open a loot chest with Space, then press Space again after its animation to collect the arrow type. Holding J cycles only unlocked types. Chests block movement even after being emptied.

See [checkpoint and chest settings](docs/room_interactions.md) for room positions, chest contents, trap radius, and respawn behavior.
