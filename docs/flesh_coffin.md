# Flesh Coffin: test encounter

The final-boss prototype is temporarily on the route **starting checkpoint → Flesh Coffin → Slime King**. Its room is labelled `COFFIN TEST`; the original room IDs and STAGE 4 Enchant Blade chest remain unchanged. The normal later route and Slime King's optional reward room still work.

The boss waits in the upper-right corner on opening1 frame 1. Approaching within 85 pixels starts the introduction. The HP bar appears after the introduction ends. The exit stays locked until the death animation finishes. Backtracking is allowed; an unfinished fight resets on checkpoint respawn, while a defeated boss stays defeated.

## Phase 1

Boss HP is 1000. The three deployed swords share a separate 400 HP pool. Melee, arrows, explosion damage and fire burns can damage the deployed swords. Damaging them never subtracts boss HP. During phase 1, all damage hitting the boss is redirected to the sword pool while it has HP, including reflected projectiles and fire burns. The hit that breaks the swords does not spill into boss HP; subsequent hits damage the boss normally. The smaller lower bar and SWORDS current / maximum label show their shared HP throughout phase 1.

During phase 1, the boss walks towards the player during idle rest windows and while detached swords are executing skills (Ground Pin, Targeted Stabs, Wall Split), stopping when within melee range (36 pixels). When walking with swords attached to its back, it animates using `flesh_coffin_wak_sword`; when swords are deployed or destroyed, it animates using `flesh_coffin_wak_nosword`.

Every skill starts with the eye-light animation. Skills 1–3 deploy swords using sword_up and retrieve them with sword_appear.

1. Ground Pin: three interior positions; each sword emits three eight-way waves, then returns.
2. Targeted Stabs: nine sequential stabs from random cardinal directions. Each stab locks the player's position during a short visible line warning.
3. Wall Split: the three swords occupy three distinct randomly selected arena edges. Each sword launches three mini blades from randomized, separated positions along its edge (nine total), straight across the room. `COFFIN_WALL_RAYS` controls the count per sword. Red-outlined blades from this attack **cannot be reflected**.
4. Portal Slam: locks a landing spot at the end of the portal animation, drops from above, hits a 25-pixel circle, and emits nine mini blades. After the sword pool is destroyed, this is the only remaining phase-1 skill and emits no blades.

Slash through reflectable mini blades during the player's active slash window to send them toward the boss. Reflections use the player's arrow damage, matching reflected Slime shots. Reflected shots are white and cannot hurt the player. Mini blades expire at arena edges or at their configured lifetime.

## Phase 2

At 75% HP or below, all active attacks clear and opening2 plays over 8.4 seconds. The boss is invulnerable during introduction, phase transition and portal/teleport disappearance. Ordinary windups and the stun window remain damageable.

1. Dash: skips the eye animation, locks direction and crosses the arena quickly. A red trail remains; two seconds after the dash ends it deals damage once along the swept path. The phase2Attack1_1 asset plays along the path for 0.4 seconds (its exported 1 ms frame durations are deliberately overridden).
2. Front Slash: up/right uses swordFront; down/left uses swordBack. The outlined area indicates the locked hitbox.
3. Teleport Strikes: one to three random-direction reappearances with a slash after each teleport.
4. Teleport Slam: teleport, overhead drop, circular landing hit, then six random blood_dust patches. Each patch lasts 10 seconds and deals 5 damage every 0.5 seconds of contact. Overlapping patches share one damage tick and respect player invincibility. They clear on boss death; pausing freezes them.
5. Charged Slash: 3.5-second charge, teleport in front of the player's facing direction, a slower parryable slash, then 3.5 seconds of immobile stun whether the slash lands, misses or is parried.

Phase-2 sword slashes can be parried by normal or charged player slashes. A parry cancels damage in both directions for that player swing against the boss, as with Ghoul. It does not cancel burns or independent projectiles. The trail, slams and phase-1 flying swords are not melee parries.

## Tuning and assets

Charged slashes use their own `COFFIN_CHARGED_REACH` (96) and `COFFIN_CHARGED_WIDTH` (64). Their damage, effect and parry overlap all use this area. Normal slash dimensions remain separate. Phase-two slashes, teleport-slash preparation, and slam landing areas are shown in translucent red on the ground.

Teleport Strikes now pause in the phase-two idle pose after reappearing. `COFFIN_TELEPORT_SLASH_PAUSE` controls this pause (0.6 seconds); the boss holds its position and facing before starting the slash. Stun holds frame 2 of the stun animation throughout the stun window. Charged slashes use `assets/sprites/effects/charge-slash.h`, resampled to the configured slash size, with the peak effect aligned to the damage time.

The dash locks its destination at attack start and ends there. It leaves damaging blood dust every `COFFIN_DASH_DUST_SPACING` pixels. The delayed trail flash is drawn as a line rather than repeated copies of the boss sprite.

Portal/teleport animations play before the drop. Falling and landing recovery use the idle sprite of the current phase (with/without swords in phase 1). Blood dust is drawn on the floor beneath the actors using `blood_dust.h`. Adjust `COFFIN_DUST_COUNT`, `COFFIN_DUST_LIFETIME`, `COFFIN_DUST_RADIUS`, `COFFIN_DUST_DAMAGE`, and `COFFIN_DUST_TICK_TIME` in the boss config.

All balance values are in `src/enemies/flesh_coffin_config.h`: HP, phase ratio, activation distance, animation durations, projectile speed/damage/lifetime, wave/stab counts, telegraph windows, dash/trail parameters, slash geometry and damage, teleport distance/count, charge and stun durations. Use positive durations and valid count ranges. Sprite selection is in `src/ui/flesh_coffin_draw.h`; behavior is in `src/enemies/flesh_coffin.h`.

The main sprite sheet is `assets/sprites/enemies/flesh coffin(boss).h`. Swords and mini blades use `blood_spellblade.h` and `mini_blood_spellblade.h`. Room placement is controlled by `STAGE_COFFIN_ROOM`, `Stage_Next`, `Stage_Previous`, `Stage_BackSide` and `Stage_Load` in `src/core/stages.h`.

Run `run_game.cmd` after editing settings. The dedicated regression test is `tests/flesh_coffin_test.c`; compile it with the same include/library flags as the other SDL tests.

## Orbit and close slash tuning

Phase 1 can release three swords into an evenly spaced orbit for 5 seconds, dealing contact damage before returning. Tune COFFIN_ORBIT_RADIUS, COFFIN_ORBIT_TIME, COFFIN_ORBIT_SPEED and COFFIN_ORBIT_DAMAGE. While swords are deployed or destroyed, walking stops at COFFIN_UNARMED_STOP_DISTANCE (88 pixels); attached swords use COFFIN_WALK_STOP_DISTANCE (36).

In phase 2, an idle boss within slash reach chooses a stationary frontal slash. Its windup lasts COFFIN_CLOSE_SLASH_HIT_TIME (1 second), with total duration COFFIN_CLOSE_SLASH_TIME. Teleport slashes retain their timing. All seven animation frames play; COFFIN_SLASH_HIT_FRAME selects the zero-based impact frame (5). The slash effect uses the same impact timing.