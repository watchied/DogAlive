#ifndef ORDINARY_SLIME_H
#define ORDINARY_SLIME_H
#include "src/enemies/flesh_slime.h"
typedef FleshSlime Slime;
static inline void Slime_Init(Slime *s,float x,float y) { FleshSlime_Init(s,x,y);s->ordinarySlime=true; }
#endif
