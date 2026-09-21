# Enchant Blade and melee parry

The Enchant Blade chest is at (270, 65), in the upper-right of the room labelled STAGE 4 (room index 3). Open it with Space and press again to collect. The unlock survives checkpoint respawns; a new run resets it.

Before unlocking, Space keeps its immediate normal slash. After unlocking:

- Press and release Space before 2.5 seconds for a normal slash.
- Hold until the small bar above the player fills, then release for a charged slash.
- Charging and the released charged swing allow movement at 70% of normal walking speed; sprinting and bow firing are disabled. While charging, movement changes facing. After release, the swing keeps its facing and follows the moving player. Pausing, losing focus, dying, or changing rooms cancels a pending charge.
- The charged hitbox extends 64 pixels forward from the player's center and is 80 pixels across (rotated when facing up/down). It deals twice the current normal melee damage, once per target per swing. It uses the same slash frames, enlarged to match this area; damage still lands on the largest slash frame.
- Both slash types can parry a Ghoul slash when the two attack hitboxes overlap during the player's active parry window. A successful parry consumes that enemy slash and flashes the actors white. Merely holding a charge does not parry. Other enemy projectiles/explosions retain their existing rules.
- Space near room objects still interacts instead of starting a charge.

## Tuning

All player combat settings are near the top of `src/player/player.h`:

| Setting | Default | Meaning |
|---|---:|---|
| `PLAYER_CHARGE_TIME` | 2.5 | Hold time in seconds |
| `PLAYER_CHARGE_REACH` | 64 | Forward hitbox and effect length from the player's center |
| `PLAYER_CHARGE_WIDTH` | 80 | Hitbox and effect width across the swing |
| `PLAYER_CHARGE_MOVE_MULTIPLIER` | 0.7 | Movement speed while charging and swinging |
| `PLAYER_CHARGE_DAMAGE_MULTIPLIER` | 2 | Multiplier on current melee damage |
| `PLAYER_CHARGE_STAMINA_COST` | 15 | Cost on release of a charged swing |
| `PLAYER_MELEE_STAMINA_COST` | 15 | Cost of a normal swing |
| `PLAYER_PARRY_START_TIME` / `PLAYER_PARRY_END_TIME` | 0.3 / 0.5 | Parry window in seconds after the slash starts |
| `PLAYER_PARRY_FLASH_TIME` | 0.16 | Successful parry flash duration |

Stamina must cover the selected swing's cost. No stamina regenerates while charging. Keep the charge time and hitbox dimensions positive, and parry times within the 0.5-second swing.

Change the chest position in `roomDefinitions[3]` in `src/core/stages.h`. The final chest field, `enchantBlade`, grants this ability instead of an arrow type.
