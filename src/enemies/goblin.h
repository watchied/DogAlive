#ifndef GOBLIN_H
#define GOBLIN_H
#include "src/enemies/bloodless_ghoul.h"
typedef Ghoul Goblin;
static inline void Goblin_Init(Goblin *g,float x,float y) { Ghoul_Init(g,x,y);g->goblin=true; }
#endif
