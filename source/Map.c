#include "Map.h"
#include "Utilities.h"

const u16 *tilemap;
u16 *screen_block;
u16 scroll_ofs;
u8 window_ofs;

void init_lvl(void *scb, const u16 *tl)
{
    screen_block = scb;
    scroll_ofs = 0;
    window_ofs = 0;
    tilemap = tl;
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

void reset_window()
{
    scroll_ofs = 0;
    window_ofs = 0;

    REG_BG0HOFS = 0;
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
