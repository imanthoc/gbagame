#include "Map.h"

#include "Utilities.h"

const u16 *tilemap;
const u16 *pal;
const unsigned int *tiles;

u16 *screen_block;
u16 scroll_ofs;
u8 window_ofs;

void reset_lvl(void *scb, const u16 *tm, const u16 *pl, const unsigned int *tl, u16 tiles_len)
{
    screen_block = scb;
    scroll_ofs = 0;
    window_ofs = 0;

    tilemap = tm;
    pal = pl;
    tiles = tl;


    REG_BG0HOFS = 0;

    memcpy_hw(BG_PALETTE, pal, 512);
	memcpy_hw(CHAR_BASE_BLOCK(0), tiles, tiles_len);

}

void reset_window()
{
    scroll_ofs = 0;
    window_ofs = 0;

    REG_BG0HOFS = 0;
}

/*
 * Window Size is Always 32x32 blocks (256x256 pixels)
 */
void draw_window()
{
    u16 *scb = screen_block;
    const u16 *tm = tilemap + (window_ofs << 1);
    u8 row_counter = 1;

    for (u16 i = 0; i < 640; ++i)
    {
        *scb++ = tm[i];

        if (row_counter++ == 32)
        {
            tm += MAP_WT >> 1;
            row_counter = 1;
        }
    }
}



u8 scroll_right()
{
    if (scroll_ofs == 16)
    {
        window_ofs++;
        scroll_ofs = 0;
        REG_BG0HOFS = 0;

        draw_window();

        return 1;
    }
    else
    {
        scroll_ofs += 2;
        REG_BG0HOFS = scroll_ofs;

        return 0;
    }
}

u8 scroll_left()
{

    if (scroll_ofs == 0)
    {
        window_ofs--;
        scroll_ofs = 16;
        REG_BG0HOFS = 16;

        draw_window();

        return 1;
    }
    else
    {
        scroll_ofs -= 2;
        REG_BG0HOFS = scroll_ofs;

        return 0;
    }
}
