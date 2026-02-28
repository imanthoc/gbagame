#ifndef UTILITIES_H
#define UTILITIES_H

#include <gba.h>

#define FONT_VRAM_ADDR 0x660
#define FONT_OAM_INDEX 102

#define BULLET_VRAM_ADDR 0x610
#define BULLET_OAM_INDEX 97

#define PLAYER_OAM_INDEX 0
#define PLAYER_BULLETS_OAM_INDEX 1
#define PLAYER_MAX_ACTIVE_BULLETS 15

#define ENEMY_OAM_INDEX 16
#define ENEMY_MAX_ACTIVE_BULLETS 3
#define ENEMY_BULLET_DELAY 100
#define ENEMY_DELAY 100

#define BULLET_SPEED 2
#define BULLET_DELAY 5

#define END_TILEMAP_OFFSET 1

void clear_oam();
void memcpy_hw(void *dst, const void *src, u16 n);


#endif
