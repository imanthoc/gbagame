#ifndef ENGINE_H
#define ENGINE_H

#include <gba.h>

#include "Map.h"

extern OBJATTR shadow_oam[128];

void reset_engine(u8 current_lvl);
void tick();
void fade_in();
void fade_out();
void black_screen();
void unblack_screen();
void copy_shadow_oam_dma();
void copy_shadow_oam_cpu();

#endif
