#ifndef UTILITIES_H
#define UTILITIES_H

#include "level_data.h"

#include <gba.h>
#include <gba_sound.h>

#define MONSTER_VRAM_ADDR 0x800
#define ENEMY_TILE_INDEX 129

#define FRAME_J_VRAM_ADDR 0x770
#define FRAME_J_TILE_INDEX 120

#define ARROW_VRAM_ADDR 0x620
#define ARROW_OAM_INDEX 16
#define ARROW_TILE_INDEX 98

#define FIRE_VRAM_ADDR 0x630
#define FIRE_OAM_INDEX (ARROW_OAM_INDEX + 1)
#define FIRE_TILE_INDEX 100

#define BULLET_VRAM_ADDR 0x610
#define BULLET_TILE_INDEX 97

#define PLAYER_OAM_INDEX 0
#define PLAYER_BULLETS_OAM_INDEX (PLAYER_OAM_INDEX +1)
#define PLAYER_MAX_ACTIVE_BULLETS 15

#define MAX_ACTIVE_ENEMIES 5
#define ENEMY_OAM_INDEX (FIRE_OAM_INDEX + FIRETILE_CNT)
#define ENEMY_DELAY 10

#define BAT_VRAM_ADDR 0x1000
#define BAT_OAM_INDEX (ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES)
#define BAT_TILE_INDEX 0x100

#define BULLET_SPEED 2
#define BULLET_DELAY 5

#define END_TILEMAP_OFFSET 1

void clear_oam();
void clear_bg();
void clear_screen();
void memcpy_hw(void *dst, const void *src, u16 n);


#endif
