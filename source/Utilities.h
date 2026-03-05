#ifndef UTILITIES_H
#define UTILITIES_H

#include <gba.h>

#define FRAME_J_VRAM_ADDR 0x770
#define FRAME_J_TILE_INDEX 120

#define ARROW_VRAM_ADDR 0x620
#define ARROW_OAM_INDEX 25
#define ARROW_TILE_INDEX 98

#define FIRE_VRAM_ADDR 0x630
#define FIRE_OAM_INDEX 20
#define FIRE_TILE_INDEX 100

#define BULLET_VRAM_ADDR 0x610
#define BULLET_OAM_INDEX 97

#define PLAYER_OAM_INDEX 0
#define PLAYER_BULLETS_OAM_INDEX 1
#define PLAYER_MAX_ACTIVE_BULLETS 15

#define ENEMY_OAM_INDEX 16
#define ENEMY_DELAY 10

#define BULLET_SPEED 2
#define BULLET_DELAY 5

#define END_TILEMAP_OFFSET 1

void clear_oam();
void clear_bg();
void clear_screen();
void memcpy_hw(void *dst, const void *src, u16 n);


#endif
