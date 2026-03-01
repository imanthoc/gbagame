#ifndef ENGINE_H
#define ENGINE_H

#include <gba.h>

#include "Map.h"

void reset_engine(u8 current_lvl);
void tick_before_vblank();
void tick_vblank();

#endif
