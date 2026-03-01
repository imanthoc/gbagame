#ifndef MAP_H
#define MAP_H

#include <gba.h>

#define MAP_WT 84
#define MAP_HT 20

extern u16 scroll_ofs;
extern u8 window_ofs;
extern u16 *screen_block;

void reset_lvl(void *scb, const u16 *tm, const u16 *pl, const unsigned int *tl, u16 tiles_len);
void reset_window();
void draw_window();
void draw_end_window();
u8 scroll_right();
u8 scroll_left();

#endif
