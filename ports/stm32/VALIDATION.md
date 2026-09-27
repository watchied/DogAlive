# Validation record

Validated on 2026-09-28, before installation into the gameboy project.

## Build

- STM32CubeIDE 2.2.0 headless managed Debug build: **0 errors, 0 warnings**.
- ARM GCC 14.3 independent build: links successfully.
- CubeIDE size report: text 1,455,004 bytes, data 268 bytes,
  bss/reservations 471,472 bytes. Flash is approximately 69.4% of 2 MiB.
  RAM including the 64 KiB heap and 8 KiB stack reservations is approximately
  90% of 512 KiB. Heap reservation is not a measurement of runtime usage.
- Debug and Release include/exclusion settings and local linker paths checked.
- No source under `Core/Inc/dogalive` is compiled twice by CubeIDE.
- Every converted sprite array passed an encode/decode equality assertion.

## Host checks

These checks run the port's renderer and game logic with mocked GPIO, display,
clock and audio. They are not physical hardware tests.

Passed: game boot/render preview, bow_input, bow_charges, charged_parry,
enemy_group, flesh_coffin, melee_timing, room_interactions, stages,
enemy_layouts, and story gameplay/background rendering.

The story test retains its gameplay assertions and substitutes the embedded
320x240 renderer for the original desktop contact-sheet export.

Five tests fail in both the port and the original PC code, at the same assertion:

| Test | Assertion location |
| --- | --- |
| potion_test | line 19: potion timer expectation |
| charge_attack_test | line 12: room interaction expectation |
| enemy_combat_test | line 33: blast/player overlap expectation |
| player_invincibility_test | line 19: damage/invincibility expectation |
| slime_king_test | line 138: boss damage expectation |

The existing tests and game tuning were not changed to hide these failures.
`host_check.py` intentionally returns a nonzero status while these remain.

## Hardware not yet verified

No board was flashed or physically tested. Display reset/rotation, SDMMC wiring,
actual button response, amplifier output, motor drive, sustained FPS, stack
high-water mark, and audio underruns require an on-board test. Audio PCM files
were generated, but their playback through MAX98357A has not been measured.
