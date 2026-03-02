#include "Engine.h"
#include "Collisions.h"
#include "Utilities.h"
#include "Enemy_ai.h"
#include "font.h"
#include "Player.h"
#include "level_data.h"

#define GAME_STATE_START 0
#define GAME_STATE_NORMAL 1
#define GAME_STATE_OVER 3

#define SCROLL_STATE_IDLE 0
#define SCROLL_STATE_MAP_RIGHT 1
#define SCROLL_STATE_MAP_LEFT 2
#define SCROLL_STATE_PL_RIGHT 3
#define SCROLL_STATE_PL_LEFT 4

#include "agb.h"

static u8 game_state = GAME_STATE_NORMAL;
static u8 ext_counter = 0;

static u8 enemies_killed = 0;

static u16 keys_held = 0xFFFF;
static u16 keys_down = 0xFFFF;
static s8 scroll_mov_offset = 0;

static u8 current_lvl = 0;

static u8 scroll_state; /* 0 = nothing, 1 = map scroll right, 2 = map scroll left, 3 pl scroll right, 4 pl scroll left */

static void move_player_or_map_right()
{
    if (!no_collision_right(pl_get_x(), pl_get_y())) return;

    if (map_can_scroll_right() && pl_is_centered())
    {
        scroll_state = SCROLL_STATE_MAP_RIGHT;
    }
    else if (pl_can_scroll_right())
    {
        scroll_state = SCROLL_STATE_PL_RIGHT;
    }

    OAM[PLAYER_OAM_INDEX].attr1 &= ~(ATTR1_FLIP_X);
}

static void move_player_or_map_left()
{
    if (map_can_scroll_left() && pl_is_centered())
    {
        scroll_state = SCROLL_STATE_MAP_LEFT;
    }
    else if (pl_can_scroll_left())
    {
        scroll_state = SCROLL_STATE_PL_LEFT;
    }

    OAM[PLAYER_OAM_INDEX].attr1 |= (ATTR1_FLIP_X);
}

static inline void scroll_entire_before_vblank()
{
    scroll_state = SCROLL_STATE_IDLE;

    if (keys_held & KEY_RIGHT)
    {
        move_player_or_map_right();
    }
    else if (keys_held & KEY_LEFT)
    {
        move_player_or_map_left();
    }
}

void scroll_entire_vblank()
{
    scroll_mov_offset = 0;

    switch (scroll_state)
    {
        case SCROLL_STATE_MAP_RIGHT:
            if (enemies_killed >= 3)
            {
                u16 cur_x = OAM[ARROW_TILE_INDEX].attr1 & 0x1FF;
                OAM[ARROW_TILE_INDEX].attr1 = OBJ_X(cur_x - 2) | ATTR1_SIZE_8;
            }

            if (!map_scroll_right())
            {
                scroll_mov_offset = -1;
            }
        break;

        case SCROLL_STATE_PL_RIGHT:
            pl_scroll_right();
        break;

        case SCROLL_STATE_MAP_LEFT:
            if (enemies_killed >= 3)
            {
                u16 cur_x = OAM[ARROW_TILE_INDEX].attr1 & 0x1FF;
                OAM[ARROW_TILE_INDEX].attr1 = OBJ_X(cur_x + 2) | ATTR1_SIZE_8;
            }

            if (!map_scroll_left())
            {
                scroll_mov_offset = 1;
            }
        break;

        case SCROLL_STATE_PL_LEFT:
            pl_scroll_left();
        break;
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
        //if (!OAM[ARROW_OAM_INDEX].attr2)
        {
            //OAM[ARROW_OAM_INDEX].attr0 = OBJ_Y(100) | ATTR0_COLOR_16 | ATTR0_SQUARE;
            //OAM[ARROW_OAM_INDEX].attr1 = OBJ_X(100) | ATTR1_SIZE_8;
            //OAM[ARROW_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(ARROW_TILE_INDEX) | ATTR2_PRIORITY(0);
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

static void tick_state_normal_before_vblank()
{
    //if (check_player_extant(pl_get_x() + 8, pl_get_y() + 31) || check_extant_from_fire(pl_get_x() + 8, pl_get_y() + 31))
    if (0)
    {
        game_state = GAME_STATE_OVER;
        return;
    }

    scroll_entire_before_vblank();

    // Player stuff
    pl_advance_anim_before_vblank(keys_held);
    pl_tick_gravity(keys_down & KEY_B);
    pl_handle_player_bullets(keys_held); // checks if bullet needs to be added due to input

    // Enemies stuff
    u8 k = handle_enemies_before_vblank();
    enemies_killed += k;
    // lvl stuff
    check_level_progression();
}

static void tick_state_over_vblank()
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

static void tick_state_normal_vblank()
{
    // key state is set before vblank
    // Move player according to gravity calculated before vblank
    pl_set_y();

    // map scrolling NEEDS to happen during vblank
    scroll_entire_vblank();

    advance_fire_anim();

    pl_add_bullet_to_oam();
    pl_move_bullets(scroll_mov_offset);
    pl_advance_anim_vblank();

    handle_enemies_vblank(scroll_mov_offset);
}

static void tick_state_start_vblank()
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

void tick_before_vblank()
{
    scanKeys();
    keys_down = keysDown();
    keys_held = keysHeld();

    switch (game_state)
    {
        case GAME_STATE_NORMAL:
            tick_state_normal_before_vblank();
        break;
    }
}

void tick_vblank()
{
    switch (game_state)
    {
        case GAME_STATE_START:
            tick_state_start_vblank();
        break;

        case GAME_STATE_NORMAL:
            tick_state_normal_vblank();
        break;

        case GAME_STATE_OVER:
            tick_state_over_vblank();
        break;
    }
}
