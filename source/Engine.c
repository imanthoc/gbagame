#include "Engine.h"
#include "Collisions.h"
#include "Utilities.h"
#include "Enemy_ai.h"
#include "font.h"
#include "Player.h"
#include "level_data.h"

OBJATTR shadow_oam[128];

#define GAME_STATE_START 0
#define GAME_STATE_NORMAL 1
#define GAME_STATE_OVER 3

#define SCROLL_STATE_IDLE 0
#define SCROLL_STATE_MAP_RIGHT -1
#define SCROLL_STATE_MAP_LEFT 1
#define SCROLL_STATE_PL_RIGHT -2
#define SCROLL_STATE_PL_LEFT 2

#define ARROW_ANIM_FRAMES 27

#include "agb.h"

static u8 game_state = GAME_STATE_NORMAL;
static u8 ext_counter = 0;

static u8 enemies_killed = 0;

static u16 keys_held = 0xFFFF;
static u16 keys_down = 0xFFFF;
static s8 scroll_mov_offset = 0;

static u8 current_lvl = 0;

static s8 scroll_state; /* 0 = nothing, 1 = map scroll right, 2 = map scroll left, 3 pl scroll right, 4 pl scroll left */

static u8 arrow_anim_counter;
static s8 arrow_anim_y_offset[ARROW_ANIM_FRAMES] = {
    0, 0, 0, 1, 1, 1, 1, 2, 2,
    1, 1, 1, 0, 0, 0, -1, -1, -1,
    -2, -2, -1, -1, -1, -1, 0, 0, 0
};

static void move_player_or_map_right()
{
    if (!no_collision_right(pl_get_x(), pl_get_y())) return;

    if (map_can_scroll_right() && pl_is_centered())
    {
        if (!map_scroll_right())
        {
            u16 cur_x = shadow_oam[ARROW_OAM_INDEX].attr1 & 0x1FF;
            shadow_oam[ARROW_OAM_INDEX].attr1 = OBJ_X(cur_x - 2) | ATTR1_SIZE_8;

            scroll_mov_offset = -1;
            scroll_state = SCROLL_STATE_MAP_RIGHT;
        }
        shadow_oam[PLAYER_OAM_INDEX].attr1 &= ~(ATTR1_FLIP_X);
    }
    else if (pl_can_scroll_right())
    {
        pl_scroll_right();
        shadow_oam[PLAYER_OAM_INDEX].attr1 &= ~(ATTR1_FLIP_X);
        scroll_state = SCROLL_STATE_PL_RIGHT;
    }
}

static void move_player_or_map_left()
{
    if (map_can_scroll_left() && pl_is_centered())
    {
        if (!map_scroll_left())
        {
            // maybe generalize on ALL the sprites that need to be scrolled
            u16 cur_x = shadow_oam[ARROW_OAM_INDEX].attr1 & 0x1FF;
            shadow_oam[ARROW_OAM_INDEX].attr1 = OBJ_X(cur_x + 2) | ATTR1_SIZE_8;

            scroll_mov_offset = 1;
            scroll_state = SCROLL_STATE_MAP_LEFT;
        }
        shadow_oam[PLAYER_OAM_INDEX].attr1 |= (ATTR1_FLIP_X);
    }
    else if (pl_can_scroll_left())
    {
        pl_scroll_left();
        shadow_oam[PLAYER_OAM_INDEX].attr1 |= (ATTR1_FLIP_X);
        scroll_state = SCROLL_STATE_PL_LEFT;
    }
}

static inline void scroll_entire()
{
    scroll_state = SCROLL_STATE_IDLE;
    scroll_mov_offset = 0;

    if (keys_held & KEY_RIGHT)
    {
        move_player_or_map_right();
    }
    else if (keys_held & KEY_LEFT)
    {
        move_player_or_map_left();
    }
}

void fade_in()
{
    VBlankIntrWait();
    REG_BLDALPHA = 16;
    REG_BLDCNT = 0x003F | (3 << 6); // set background and blend mode
    REG_BLDY = 16;

    for (u8 i = 16*4; i > 0; i--)
    {
        REG_BLDY = i >> 2;
        VBlankIntrWait();
    }
}

void fade_out()
{
    VBlankIntrWait();
    REG_BLDALPHA = 16;
    REG_BLDCNT = 0x003F | (3 << 6); // set background and blend mode
    REG_BLDY = 0;

    for (u8 i = 0; i <= 16*4; i++)
    {
        REG_BLDY = i >> 2;
        VBlankIntrWait();
    }
}

void black_screen()
{
    VBlankIntrWait();
    REG_BLDALPHA = 16;
    REG_BLDCNT = 0x003F | (3 << 6); // set background and blend mode
    REG_BLDY = 16;
}

void unblack_screen()
{
    VBlankIntrWait();
    REG_BLDALPHA = 16;
    REG_BLDCNT = 0x003F | (3 << 6); // set background and blend mode
    REG_BLDY = 0;
}

void reset_engine(u8 c)
{
    VBlankIntrWait();

    REG_BLDALPHA = 16;

    black_screen();

    clear_screen();

    current_lvl = c;
    game_state = GAME_STATE_START;

    keys_held = 0xFFFF;
    keys_down = 0xFFFF;
    scroll_mov_offset = 0;
    ext_counter = 0;
    arrow_anim_counter = 0;

    reset_lvl(current_lvl, SCREEN_BASE_BLOCK(31));
    reset_window();
    reset_player(current_lvl);
    reset_enemy_ai(current_lvl);
    reset_collisions(current_lvl);

    unblack_screen();

    VBlankIntrWait();
}

static void check_level_progression()
{
    u16 absolute_x = (window_ofs << 3) + pl_get_x() + 200;

    if (enemies_killed >= 3)
    {
        // add arrow if it hasnt already been added
        if (!shadow_oam[ARROW_OAM_INDEX].attr0)
        {
            shadow_oam[ARROW_OAM_INDEX].attr0 = OBJ_Y(100) | ATTR0_COLOR_16 | ATTR0_SQUARE;
            shadow_oam[ARROW_OAM_INDEX].attr1 = OBJ_X(lvlTrigRegion[current_lvl][0] + 320) | ATTR1_SIZE_8;
            shadow_oam[ARROW_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(ARROW_TILE_INDEX) | ATTR2_PRIORITY(0);
        }
        else
        {
            // else animate it up and down
            u8 cur_y = shadow_oam[ARROW_OAM_INDEX].attr0 & 0xFF;

            cur_y += arrow_anim_y_offset[arrow_anim_counter];

            shadow_oam[ARROW_OAM_INDEX].attr0 = OBJ_Y(cur_y) | ATTR0_COLOR_16 | ATTR0_SQUARE;

            if (arrow_anim_counter == ARROW_ANIM_FRAMES-1) arrow_anim_counter = 0;
            else arrow_anim_counter++;
        }

        if (0 && absolute_x >= lvlTrigRegion[current_lvl][0] && absolute_x <= lvlTrigRegion[current_lvl][1])
        {
            current_lvl++;

            fade_out();

            reset_engine(current_lvl);
            place_fire_tiles();
            pl_unhide();
            game_state = GAME_STATE_NORMAL;
            draw_window();

            fade_in();
        }
    }
}

static void tick_state_normal()
{
    //if (check_player_extant(pl_get_x() + 8, pl_get_y() + 31) || check_extant_from_fire(pl_get_x() + 8, pl_get_y() + 31))
    if (0)
    {
        game_state = GAME_STATE_OVER;
        return;
    }

    scroll_entire();

    // Player stuff
    pl_tick_gravity(keys_down & KEY_B);
    pl_advance_anim(keys_held);
    pl_handle_player_bullets(keys_held); // checks if bullet needs to be added due to input
    pl_move_bullets(scroll_mov_offset, scroll_state);

    // Enemies stuff
    enemies_killed += handle_enemies(scroll_mov_offset);

    // lvl stuff
    check_level_progression();
    advance_fire_anim();
}

static void tick_state_over()
{
    VBlankIntrWait();

    fade_out();
    clear_screen();

    do
    {
        scanKeys();
        keys_down = keysDown();
        VBlankIntrWait();

    } while (!(keys_down & KEY_A));

    reset_engine(0);

    VBlankIntrWait();
}

static void tick_state_start()
{
    VBlankIntrWait();

    clear_screen();

    scanKeys();
    keys_down = keysDown();
    keys_held = keysHeld();

    if (keys_down & KEY_A)
    {
        black_screen();
        clear_oam();

        reset_player(current_lvl);
        place_fire_tiles();
        draw_window();

        game_state = GAME_STATE_NORMAL;
        pl_unhide();
        fade_in();
    }
}

void tick()
{
    scanKeys();
    keys_down = keysDown();
    keys_held = keysHeld();

    switch (game_state)
    {
        case GAME_STATE_START:
            tick_state_start();
        break;

        case GAME_STATE_NORMAL:
            tick_state_normal();
        break;

        case GAME_STATE_OVER:
            tick_state_over();
        break;
    }
}

void copy_shadow_oam()
{
    dmaCopy(shadow_oam, OAM, 128 * sizeof(OBJATTR));
}
