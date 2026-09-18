# Slime King (floor 1)

All balance values are in **`src/enemies/slime_king_config.h`**. Save changes, close the running game, and run `run_game.cmd` to rebuild.

| Setting | Default | Meaning |
|---|---:|---|
| `KING_MAX_HP` | 600 | Starting and maximum HP |
| `KING_PHASE_TWO_RATIO` | 0.5 | Switch to phase 2 at or below this fraction of maximum HP |
| `KING_ATTACK_REST` / `KING_PHASE_TWO_REST` | 1.2 / 0.85 s | Rest between skills |
| `KING_MOVE_SPEED` | 12 px/s | Approach speed while resting |
| `KING_INTRO_TIME` | 1.6 s | Total opening1 duration; increase to slow its frames |
| `KING_PHASE_TRANSITION_TIME` | 2.8 s | Total opening2 duration and phase-change invulnerability |
| `KING_DEATH_TIME` | 2 s | Total death animation duration before NPC conversion and exit unlock |
| `KING_BUBBLE_SPEED`, `KING_BUBBLE_DAMAGE`, `KING_BUBBLE_BOUNCES` | 75, 18, 8 | Bubble tuning |
| `KING_DASH_SPEED`, `KING_DASH_DAMAGE`, `KING_DASH_WINDUP`, `KING_DASH_TIME` | 250, 24, 0.65 s, 0.65 s | Dash tuning |
| `KING_SLAM_MIN` / `KING_SLAM_MAX` | 3 / 7 | Random number of slams per combo |
| `KING_SLAM_DAMAGE`, `KING_SLAM_RADIUS` | 25, 26 px | Landing damage area |
| `KING_BURROW_TIME`, `KING_SLAM_WARNING`, `KING_SLAM_FALL_TIME`, `KING_SLAM_RECOVERY` | 0.8 / 0.55 / 0.2 / 0.35 s | Slam timing |
| `KING_LASER_SPEED`, `KING_LASER_DAMAGE`, `KING_LASER_BOUNCES` | 115, 12, 2 | Eight-way laser tuning |
| `KING_RADIAL_COUNT`, `KING_RADIAL_WINDUP` | 8, 0.7 s | Radial volley |
| `KING_SPIRAL_COUNT`, `KING_SPIRAL_INTERVAL`, `KING_SPIRAL_WINDUP`, `KING_SPIRAL_BOUNCES` | 10, 0.12 s, 0.7 s, 2 | Rotating volley |
| `KING_BEAM_CHARGE_TIME`, `KING_BEAM_SPEED`, `KING_BEAM_DAMAGE` | 1.5 s, 1000 px/s, 45 | Charged beam |
| `KING_BEAM_INTERRUPT_DAMAGE` | 1 | Total direct-hit damage needed during a charge to cancel it |
| `KING_BEAM_INTERRUPT_STUN` | 1 s | Stun after interrupt |

Additional settings cover projectile sizes, lifetimes, intro/transition durations, collision body, and projectile capacity. Keep times/speeds positive, the phase ratio between 0 and 1, and projectile capacity large enough for the volleys. The rotating attack makes one full turn.

## Encounter

- Floor 1 contains the boss instead of regular enemies. `stageDefinitions` in `src/core/stages.h` has a final `slimeKing` flag; only floor 1 sets it to `true`.
- Phase 1: bubble, locked-direction dash, and burrow/slam combos.
- Phase 2: bubble and dash remain; the slam combo is replaced by radial lasers, rotating lasers, and the charged beam.
- Dash direction locks during its warning. Each slam locks a new player position when its warning circle appears. Move out of the circle before landing.
- The beam tracks the player while charging. Hit the boss with a sword, arrow, explosion, or reflected Slime shot to interrupt. Burn ticks damage the boss but do not interrupt charging.
- Player invincibility applies to all boss attacks. Fast attacks use swept collision checks. Projectiles vanish on their configured final wall contact (a corner counts as one contact), on hitting the player, or when lifetime expires.
- The boss is invulnerable during the intro and phase transition. All attack states, including burrow preparation, slam warning, and falling, accept damage at the boss's logical body position. Burns pause only during invulnerable states. The underground/airborne body does not push the player. Attacks clear when the boss changes phase or dies.
- The death animation must finish before the boss becomes the harmless NPC and the exit unlocks. Revisiting floor 1 restores the defeated NPC, not a new boss. Restart resets the encounter.
- Pausing freezes the boss, its projectiles, and attack timers.

## Assets

Uses the provided boss/NPC sprite tags, `king_slime_bubble.h`, `king_slime_laser.h`, and `king_slime_beam.h`. The beam export originally reused the laser's C identifiers; they are now prefixed `KING_SLIME_BEAM` / `king_slime_beam`. Preserve this unique prefix when exporting a replacement beam header.

## Tests

```powershell
gcc -std=c11 -Wall -Wextra -Werror tests/slime_king_test.c -o bin/slime_king_test.exe -I. -Iinclude -Llib -lSDL3
.\bin\slime_king_test.exe
```

Checks phase change, all skill families, bounce limits, high-speed collision, beam interrupts, arrow/burn damage, death transformation, and exit persistence.
