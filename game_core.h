#ifndef GAME_CORE_H
#define GAME_CORE_H

#include "player.h"

void Game_Init(Player *player);
void Game_Update(Player *player, const bool *keyboardState, float deltaTime);

#endif
