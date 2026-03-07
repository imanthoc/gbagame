#ifndef MAP_H
#define MAP_H

#include <gba.h>
#include "level_data.h"

#define MAP_HT 20

extern u16 scroll_ofs;
extern u8 window_ofs;
extern u16 *screen_block;
extern const u16 lvl_widths[LVL_CNT];

void reset_lvl(u8 current_lvl, void *scb);

void advance_fire_anim();
u8 check_extant_from_fire(u16 x, u8 y);

void reset_window();
void draw_window();
void draw_end_window();

u8 map_can_scroll_right();
u8 map_can_scroll_left();

u8 map_scroll_right();
u8 map_scroll_left();

#endif
