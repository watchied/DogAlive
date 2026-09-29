#include <assert.h>
#include <stdio.h>
#include "src/enemies/bloodless_ghoul.h"
#include "src/enemies/flesh_coffin.h"

int main(void)
{
    Player player;
    Player_Init(&player);
    player.x = 100; player.y = 100;
    player.direction = PLAYER_RIGHT;
    player.isAttacking = true;
    player.currentFrame = PLAYER_ATTACK_HIT_FRAME;

    Ghoul ghoul;
    Ghoul_Init(&ghoul, 120, 100);
    ghoul.direction = PLAYER_LEFT;
    ghoul.state = GHOUL_ATTACK;
    ghoul.timer = GHOUL_ATTACK_WINDUP + 0.4f;
    assert(!player.parrySoundPending);
    assert(Ghoul_TryParry(&ghoul, &player));
    assert(player.parrySoundPending);
    player.parrySoundPending = false;
    assert(Ghoul_TryParry(&ghoul, &player)); // Cached result, no second sound.
    assert(!player.parrySoundPending);

    FleshCoffin coffin;
    Coffin_Init(&coffin);
    coffin.phase = 2; coffin.x = 100; coffin.y = 100;
    coffin.direction = PLAYER_RIGHT;
    player.x = 120; player.y = 92; player.direction = PLAYER_LEFT;
    Coffin_Enter(&coffin, FC_CHARGED_SLASH);
    coffin.timer = COFFIN_SLASH_PARRY_START + 0.01f;
    assert(!Coffin_Parry(&coffin, &player)); // Normal swing cannot parry charged slash.
    assert(!player.parrySoundPending);
    player.chargedAttack = true;
    assert(Coffin_Parry(&coffin, &player));
    assert(player.parrySoundPending);
    player.parrySoundPending = false;
    assert(Coffin_Parry(&coffin, &player)); // Cached result, no second sound.
    assert(!player.parrySoundPending);

    Player_ShowParry(&player, 100, 100); // Scripted story effect is not a melee parry.
    assert(!player.parrySoundPending);
    puts("Only new successful Ghoul and Coffin melee parries trigger the sound");
    return 0;
}
