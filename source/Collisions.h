#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <gba.h>

#include "Map.h"

u8 can_move_left        (u16 x, u8 y);
u8 can_move_right       (u16 x, u8 y);
u8 can_move_down        (u16 x, u8 y);
u8 can_move_down_offs   (u16 x, u8 y, u8 offs);

#endif
