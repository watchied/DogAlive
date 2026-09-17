# Dog Alive

game for microcontroler project

how to play:

1.just build

2.type command "./bin/game.exe" or just click game.exe

## Bow controls

- Tap and release `J` to fire once. Hold `J` for 0.4 seconds to cycle Normal -> Fire -> Explosive once per hold, without firing. Tune `BOW_TYPE_HOLD_MS` in `src/player/bow_input.h`.
- The bow stores 3 charges and restores one every 2 seconds.
- Normal arrow costs 1 charge.
- Fire arrow costs 2 charges. Normal impact damage plus 1.5x burn damage over 3 seconds (about 2.5x total, rounded to whole HP).
- Explosive arrow costs 3 charges. Normal damage to the direct target plus 1.75x damage to all enemies touching the 24px blast radius, including the direct target. Explodes on enemy impact.
- Repeated fire hits add their remaining burn damage together and distribute it over another 3 seconds.
- The HUD shows the selected icon, 3 charge bars, and the selected arrow's cost. Switching during a shot affects the next shot.
- Tune recharge time, shooting animation speed, and elemental damage in `src/player/player.h`.
- Slime projectile reflection still uses normal arrow damage.

Test the bow system from the project root:

```powershell
gcc -std=c11 -Wall -Wextra -Werror tests/bow_charges_test.c -o bin/bow_charges_test.exe -I. -Iinclude -Llib -lSDL3
.\bin\bow_charges_test.exe
```
