#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include <gba.h>

#include "Engine.h"

void reset_enemy_ai(u8 level_index);
void add_enemy_to_oam_next_free_slot(u8 _ti, u8 flip);
u8 handle_enemies(s8 scroll_state);
u8 check_extant_from_enemy(u16 x, u8 y);

#endif
