#include "Map.h"
#include "level_data.h"
#include "Utilities.h"
#include "Engine.h"

const u16 *tilemap;
const u16 *pal;
const unsigned int *tiles;

u16 *screen_block;
u16 scroll_ofs;
u8 window_ofs;

static u8 current_lvl;
static u8 fire_anim_counter[FIRETILE_CNT];

u8 lvl_widths[LVL_CNT] = {
    84, 84
};

void reset_lvl(u8 c, void *scb)
{
    current_lvl = c;
    screen_block = scb;
    scroll_ofs = 0;
    window_ofs = 0;

    tilemap = tilemap_ptr[current_lvl];
    pal     = pal_ptr[current_lvl];
    tiles   = tiles_ptr[current_lvl];

    REG_BG0HOFS = 0;

    for (u8 i = 0; i < FIRETILE_CNT; ++i)
    {
        fire_anim_counter[i] = 0;
    }

    memcpy_hw(BG_PALETTE, pal, 512);
    memcpy_hw(CHAR_BASE_BLOCK(0), tiles, tiles_len[current_lvl]);
}

void place_fire_tiles()
{
    for (u8 i = 0; i < FIRETILE_CNT; ++i)
    {
        u8 oam_index = FIRE_OAM_INDEX + i;
        u16 y = fire_tile_positions[current_lvl][i][1] << 3;
        u16 x = fire_tile_positions[current_lvl][i][0] << 3;

        shadow_oam[oam_index].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_SQUARE;
        shadow_oam[oam_index].attr1 = OBJ_X(x) | ATTR1_SIZE_16;
        shadow_oam[oam_index].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(FIRE_TILE_INDEX) | ATTR2_PRIORITY(1);
    }
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
            tm += lvl_widths[current_lvl] >> 1;
            row_counter = 1;
        }
    }
}

u8 check_extant_from_fire(u16 pl_x, u8 pl_y)
{
    for (u8 i = 0; i < FIRETILE_CNT; ++i)
    {
        u8 oam_index = FIRE_OAM_INDEX + i;
        u16 f_y = shadow_oam[oam_index + i].attr0 & 0xFF;
        u16 f_x = shadow_oam[oam_index + i].attr1 & 0x1FF;

        if (pl_x >= f_x && pl_x <= f_x + 16 && pl_y >= f_y && pl_y <= f_y + 16) return 1;
    }

    return 0;
}

void advance_fire_anim()
{
    for (u8 i = 0; i < FIRETILE_CNT; ++i)
    {
        u8 oam_index = FIRE_OAM_INDEX + i;
        if (fire_anim_counter[i] == 4)
        {
            u8 cur_tile = shadow_oam[oam_index].attr2 & 0x03FF;
            u8 next_tile;

            if (cur_tile == FIRE_TILE_INDEX + 15)
            {
                next_tile = FIRE_TILE_INDEX;
            }
            else
            {
                next_tile = cur_tile + 5;
            }

            shadow_oam[oam_index].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(next_tile) | ATTR2_PRIORITY(1);

            fire_anim_counter[i] = 0;
        }
        else
        {
            fire_anim_counter[i]++;
        }
    }
}

u8 map_scroll_right()
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

        for (u8 i = 0; i < FIRETILE_CNT; ++i)
        {
            u8 oam_index = FIRE_OAM_INDEX + i;
            u16 cur_x = shadow_oam[oam_index].attr1 & 0x1FF;
            shadow_oam[oam_index].attr1 = OBJ_X(cur_x-2) | ATTR1_SIZE_16;
        }

        return 0;
    }
}

u8 map_scroll_left()
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

        for (u8 i = 0; i < FIRETILE_CNT; ++i)
        {
            u8 oam_index = FIRE_OAM_INDEX + i;
            u16 cur_x = shadow_oam[oam_index].attr1 & 0x1FF;
            shadow_oam[oam_index].attr1 = OBJ_X(cur_x+2) | ATTR1_SIZE_16;
        }

        return 0;
    }
}

u8 map_can_scroll_right()
{
    return window_ofs < 20;
}
u8 map_can_scroll_left()
{
    return window_ofs > 0;
}
