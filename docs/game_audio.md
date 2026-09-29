# Game sound effects

The desktop game plays the previously selected recordings for normal slash,
charged slash, and successful melee parry. Flesh Coffin uses a separate set of
recorded effects derived from the user-supplied bones-and-flesh movement and
blood-splatter explosion MP3s. Slime King uses recorded goop, slime impact,
splatter, pop, and squish textures supplied by the user. Other cues are
generated in `src/audio/game_sfx.h`.

Dungeon footsteps use three short clips from the user-supplied walking
recording. They alternate at the player's walk or sprint cadence in dungeon
rooms. The outdoor entrance and epilogue continue to use the generated steps.
Run `scripts/make_dungeon_footsteps.ps1 -Ffmpeg <path-to-ffmpeg>` to recreate
the mono 44.1 kHz WAV clips from the MP3 in `assets/sfx/`.

Ordinary chests play the user-supplied wooden trunk latch recording when they
begin opening. `scripts/make_chest_open_sfx.ps1` converts its full sequence to
mono 44.1 kHz PCM. Trapped chests retain their trap warning.

`src/audio/game_sfx_events.h` watches completed game updates. It triggers cues
from changes in player actions and health, enemy and boss attacks, projectiles,
room objects, story scenes, and menus. Repeated events such as footsteps and
projectile volleys are limited so several enemies acting at once do not produce
an overwhelming stack of sounds. Up to 20 effects can overlap with the sword
recordings and existing music. The menu effects use the same SDL device, so a
pause or resume cue can still play while gameplay clips are stopped.

`scripts/make_flesh_coffin_sfx.js` produces the boss WAV files from the two
original MP3s. The awakening emphasizes bones and flesh; the phase-two entry
emphasizes the blood splatter. Other boss moves use distinct cuts, reversals,
speed changes, compression, layers, and low impacts from the same sources. All
boss clips are mono 44.1 kHz 16-bit PCM. `GameSfx_Init` loads them and retains
the generated effects as a fallback if a boss WAV is missing.

`assets/sfx/dark_fantasy_boss_preview.wav` plays a sequence of the exact boss
clips used by the game.

`scripts/make_slime_king_sfx.js` creates separate wet cues for each Slime King
move, including a dedicated hurt sound and phase-two laser warning. Short
projectile sounds stay brief so volleys remain clear. The generated effects
remain as a fallback if a recorded file is unavailable.
`assets/sfx/slime_king_preview.wav` plays a selection of the in-game cues.
Flesh Coffin's recorded effects are at half of their original amplitude
(about 6 dB lower); Slime King's are at 80% (about 2 dB lower). The levels
within each boss, their timing, and their music are unchanged.

The ending uses `Ending song.mp3` without the generated tonal transition swells
that previously played on return home, the ending slides, the sky fade, and
victory. Dog and grave interaction cues remain.

To adjust a sound, change its `SfxSpec` in `GameSfx_Spec`: duration is in seconds,
frequencies are in hertz, `noise` controls the texture, and `volume` scales
the output. `GameSfx_Observe` contains the timing for each game event.
The charge start cue begins after a 0.4-second hold, so quick presses for
normal slashes stay free of the charge sound. The full-charge cue still plays
when the charged attack becomes ready.

The STM32 build currently streams its two music tracks through a single I2S
channel. The new SDL effects are desktop-only; the STM32 audio interface stays
unchanged.
