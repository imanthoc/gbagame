#ifndef MAP_H
#define MAP_H

#include <gba.h>

#define MAP_WT 64
#define MAP_HT 20

extern const u16 *tilemap;
extern u16 *screen_block;
extern u16 scroll_ofs;
extern u8 window_ofs;


void init_lvl(void *scb, const u16 *tilemap);
void reset_window();
void draw_window();
void draw_end_window();
u8 scroll_right();
u8 scroll_left();

#endif
