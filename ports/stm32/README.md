# DogAlive on NUCLEO-F767ZI

This port uses the current DogAlive game logic, sprites, rooms, bosses and story.
The desktop SDL dependency is replaced by a small STM32 software renderer and
GPIO input adapter. No desktop SDL library is linked into the firmware.

## Build and run

Open `D:\watt\cube\gameboy` in STM32CubeIDE. Refresh the project, then use
Project > Clean and Build Project before Run/Debug. Both configurations use
`-Os`, the project-local linker script, and `Core/Inc/dogalive` as an include
directory. `Core/Inc/dogalive/**` is excluded from automatic source compilation:
`Core/Src/game.c` is the single compilation entry for its game sources.

The tested firmware is also supplied in `Firmware/gameboy.elf` and
`Firmware/gameboy.bin`. Flash starts at `0x08000000`. Hardware flashing has not
been performed by this task.

## Controls

| Button | Action |
| --- | --- |
| D-pad | Move/aim; double-tap a direction to sprint |
| A | Interact/melee; hold for charged attack when unlocked |
| B | Tap to shoot/use the selected potion; hold to cycle unlocked items |
| Start | Start game, pause/resume, or respawn on Game Over |
| Select | Return to menu while paused, dead, or victorious |
| A on pause/Game Over/victory | Start a new run |

There is no stage-skip control. Rooms change through the original exits and
progression rules. Inputs are sampled from SysTick with 15 ms debounce and an
event queue; short action presses are retained across display transfers.

## Hardware interface (current CubeMX pinout)

| Device | Connection |
| --- | --- |
| ILI9341 SPI1 | SCK PA5, MOSI PA7, MISO PA6 if used |
| LCD CS / D-C | PD14 / PD15 |
| LCD RESET | No GPIO assigned; keep RESET high through the module's reset/pull-up circuit. Driver sends software reset. Never leave RESET floating or held low. |
| Up / Down / Left / Right | PA3 / PC0 / PC3 / PF3 |
| A / B / Start / Select | PF5 / PF10 / PF15 / PE13 |
| Vibration MOSFET gate | PE4; 100 ms pulse on damage |
| MAX98357A | I2S2 WS PB12, BCLK PB13, DIN PB15 |
| SDMMC1 | D0 PC8, D1 PC9, D2 PC10, D3 PC11, CLK PC12, CMD PD2 |
| SD detect | PF14, active-low with pull-up |

Buttons connect to GND when pressed. This follows the saved gameboy CubeMX
configuration, not Lab8.2's SPI5 wiring. The display initialization/addressing
and RGB565 transfer implementation comes from Lab8.2's MIT-licensed
`ILI9341_STM32_Driver`. It uses polling SPI, not SPI DMA. CubeMX's SPI1 DMA
configuration remains available for future optimization.

The SD driver uses **SDMMC 4-bit**, not SPI. A socket exposing only SPI signals
cannot be used without a different SD driver and pin configuration. External
CMD/data pull-ups, amplifier power/mode and LCD backlight power remain hardware
requirements; software does not configure unassigned pins.

## Music and SD card

Copy the `DOGALIVE` folder inside `SDCard` to the root of an existing FAT32 card:

```
DOGALIVE/BOSS.PCM
DOGALIVE/ENDING.PCM
```

These files contain the original MP3 tracks converted to signed 16-bit
little-endian stereo PCM at **43831 Hz**, matching the current actual I2S clock,
with 35% software volume. They take about 23 MB together. PCM avoids an MP3
decoder on the MCU. Changing the I2S clock requires reconverting these files.
Audio streams through I2S circular DMA, with foreground SD reads and pause/loop
support. Gameplay and graphics do not require an SD card; missing music is
reported in the debugger variable `dogalive_audio_error`.

## Memory and performance

RGB565 sprite arrays are losslessly run-length encoded. The preparation script
checks every encoded array against the original values. All graphics stay in
internal Flash; there are no missing story assets on the SD card.

The renderer uses one 320x240 framebuffer, a 128x128 sprite decode cache, and
direct run decoding for full-screen story backgrounds. It transfers changed
8-row strips and keeps RGB565 byte order correct. The SPI clock stays at the
saved 6.75 Mbit/s; a full-screen transfer alone needs about 182 ms, so 60 FPS is
not promised. The original 50 ms simulation-delta cap remains; demanding
scenes may slow down and require hardware profiling.

Stack reservation is 8 KB; minimum heap reservation is 64 KB. These settings
are saved in CubeMX and both linker files. D-cache remains disabled; do not
enable it without adding DMA buffer cache maintenance. Watch
`dogalive_audio_underruns` while profiling audio and rendering together.

## Validation and remaining hardware checks

- STM32CubeIDE 2.2 Debug build: zero errors and warnings.
- Independent ARM GCC build links within 2 MB Flash / 512 KB RAM.
- Host simulation opens the embedded renderer and draws the game scene.
- Gameplay test results are recorded in `VALIDATION.md`; five old tests also
  fail against the original PC source at the identical assertion.
- Physical LCD orientation/reset, SD wiring, button wiring, sound, vibration,
  FPS, and audio continuity still require the actual board.

No code was flashed to hardware. A backup of the original target project is
created before installation.

## Files and regeneration

`Core/Src/game.c` includes the prepared game implementation. Rendering/input
are in `dogalive_platform.c`; music is in `dogalive_audio.c`. Main and SysTick
integration are inside CubeMX USER CODE blocks. The original broken display
driver and legacy game are replaced, with originals retained in the backup.

If regenerating CubeMX, preserve USER CODE and verify include path, source
exclusion and `-Os`. CubeMX can overwrite the bounded SD wait in
`FATFS/Target/sd_diskio.c`; restore it from the supplied copy if needed.

The source preparation/build tools live in the PC repository under
`ports/stm32/tools`. `prepare.py` refreshes the snapshot from PC sources;
`stage.py` assembles an isolated project; `build.py` builds that staging copy.
Do not rerun staging against an already installed project as a substitute for
merging changes: the tools are a record of this migration, not an updater.
