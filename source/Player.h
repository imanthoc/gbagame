#ifndef PLAYER_H
#define PLAYER_H

#include <gba.h>

void reset_player();
void pl_advance_anim_before_vblank(u16 keys_held);
void pl_advance_anim_vblank();
void pl_tick_gravity(u8 trigger_jump);
void pl_handle_player_bullets(u16 keys_held);
void pl_move_bullets(s8 scroll_state);
void pl_add_bullet_to_oam();

void pl_set_y();
void pl_set_y_value(u8 y);

u8 pl_get_y();
u16 pl_get_x();


#endif
