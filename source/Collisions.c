#include "Collisions.h"

#include "agb.h"

u8 can_move_left ( u16 _x, u8 _y)
{
    u8 x = _x + scroll_ofs + 1;

    if (window_ofs == 0 && scroll_ofs == 0) return 0;

    u8 y1 = _y + 15;
    u8 y2 = _y + 31;

    u16 left_tile1 = ( (y1 >> 3) << 5) + (x >> 3);
    u16 left_tile2 = ( (y2 >> 3) << 5) + (x >> 3);

    return screen_block[left_tile1] <= 4 &&  screen_block[left_tile2] <= 4;
}

u8 can_move_right(u16 _x, u8 _y)
{
    //TODO: add a border check
    u8 x = _x + scroll_ofs + 14;
    u8 y1 = _y + 15;
    u8 y2 = _y + 31;

    u16 right_tile1 = ( (y1 >> 3) << 5) + (x >> 3);
    u16 right_tile2 = ( (y2 >> 3) << 5) + (x >> 3);

    return screen_block[right_tile1] <= 4 && screen_block[right_tile2] <= 4;
}

// may need to adjust for flipped sprite
u8 can_move_down(u16 _x, u8 _y)
{
    u8 xl = _x + scroll_ofs + 6;
    u8 xr = _x + scroll_ofs + 12;
    u8 y = _y + 32;

    u16 down_tile_left  = ( (y >> 3) << 5) + (xl >> 3);
    u16 down_tile_right = ( (y >> 3) << 5) + (xr >> 3);

    return screen_block[down_tile_left] <= 4 && screen_block[down_tile_right] <= 4;
}

u8 can_move_down_offs   (u16 _x, u8 _y, u8 offs)
{
    u8 xl = _x + scroll_ofs + 6;
    u8 xr = _x + scroll_ofs + 12;
    u8 y = _y + 31 + offs;

    u16 down_tile_left  = ( (y >> 3) << 5) + (xl >> 3);
    u16 down_tile_right = ( (y >> 3) << 5) + (xr >> 3);

    return screen_block[down_tile_left] <= 4 && screen_block[down_tile_right] <= 4;
}
