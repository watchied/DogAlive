# Sound assets

## Chest opening

`floraphonic-wooden-trunk-latch-3-183946.mp3` is the user-supplied chest
recording. `scripts/make_chest_open_sfx.ps1` converts the full latch sequence
to `chest_open.wav` at a balanced level. It plays for ordinary chest openings;
trapped chests still use the trap warning cue.

## Slime King recordings

`floraphonic-goopy-slime-4-219777.mp3`,
`universfield-slime-impact-352473.mp3`,
`floraphonic-slime-splatter-1-220262.mp3`,
`soundreality-pop-sound-423716.mp3`, and
`floraphonic-slime-squish-5-218569.mp3` were supplied by the user.
`scripts/make_slime_king_sfx.js` uses their wet textures for the boss's
intro, damage, bubbles, dash, burrow, slams, phase change, lasers, beam,
death, and NPC transition. It writes mono 44.1 kHz PCM cues to
`assets/sfx/slime_king/` and an audition sequence to
`assets/sfx/slime_king_preview.wav`.

## Dungeon footsteps

`freesound_community-walking-46245.mp3` is the current user-supplied walking
recording. `scripts/make_dungeon_footsteps.ps1` cuts three distinct
footfalls and balances their levels into `dungeon_step_1.wav` through
`dungeon_step_3.wav`. The game alternates them only while the main character
walks or sprints in dungeon rooms. The outdoor entrance and epilogue keep the
previous step effects.

## Sword slash sounds

`daviddumaisaudio-sword-slash-and-swing-185432.mp3` is the source file supplied by the user.

`sword_slash.wav` is the charged attack clip: source time 0.15–0.70 seconds, 75% volume, short fades, mono 44.1 kHz 16-bit PCM.

`dragon-studio-sword-clashhit-393837.mp3` is the source supplied by the user for successful melee parries. `parry_clash.wav` uses source time 0.04–0.72 seconds, 65% volume and a short fade, converted to mono 44.1 kHz 16-bit PCM. It plays once when a Ghoul or Flesh Coffin melee slash is parried.

`sword-slash-attack-sound-fx_D_minor.wav` is the current user-supplied source for normal attacks. `d_minor_normal_slash.wav` retains the full 0.528-second sound, with a volume increase for audibility, converted to mono 44.1 kHz 16-bit PCM for the game.

`custom_normal_slash.wav` is the previous generated normal attack sound, made by `node scripts/generate_normal_slash.js` and kept for comparison.

`dragon-studio-sword-slice-2-393845.mp3` and `dragon_normal_slash.wav` are the most recent user-supplied normal attack option, kept for comparison.

`universfield-sword-blade-slicing-flesh-352708.mp3` and `normal_slash.wav` are another previous normal attack sound, kept for comparison.

Both clips start when the slash becomes visible 0.3 seconds into an attack. Their strongest parts land near the 0.4-second hit frame.

## Flesh Coffin recordings

`freesound_community-bones-and-flesh-movement-98677.mp3` and
`ragecore29-htf-blood-splatter-explode-478992.mp3` were supplied by the user.
`scripts/make_flesh_coffin_sfx.js` turns them into the mono PCM cues in
`assets/sfx/coffin/`. Awakening is based on the bones-and-flesh recording;
phase-two entry is based on the blood-splatter recording. The other boss cues
use shorter and differently processed cuts of those same two sources.
