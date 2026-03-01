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

#include "agb.h"

static u8 game_state = GAME_STATE_NORMAL;
static u8 ext_counter = 0;

static u8 enemies_killed = 0;

static u16 keys_held = 0xFFFF;
static u16 keys_down = 0xFFFF;
static s8 scroll_state = 0;

static u8 current_lvl = 0;

static void draw_text_to_oam(const char *txt, u8 x, u8 y, u8 oam_index)
{
    u8 i = oam_index;
    u8 k = 0;

    while (*txt)
    {
        u16 index;
        if (*txt >= '0' && *txt <= '9')
        {
            index = FONT_OAM_INDEX + ((*txt - '0' + 26) << 2);
        }
        else
        {
            index = FONT_OAM_INDEX + ((*txt - 'A') << 2);
        }


        OAM[i].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_SQUARE;
        OAM[i].attr1 = OBJ_X(x + (k << 3) + k) | ATTR1_SIZE_16;

        OAM[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(index) | ATTR2_PRIORITY(0);

        i++;
        k++;
        txt++;
    }
}

static inline void scroll_entire()
{
    scroll_state = 0;

    if (keys_held & KEY_RIGHT)
    {
        OAM[PLAYER_OAM_INDEX].attr1 &= ~(ATTR1_FLIP_X);

        if (can_move_right(pl_get_x(), pl_get_y()) && !scroll_right()) scroll_state = -1;
    }
    else if (keys_held & KEY_LEFT)
    {
        OAM[PLAYER_OAM_INDEX].attr1 |= (ATTR1_FLIP_X);

        if (can_move_left(pl_get_x(), pl_get_y()) && !scroll_left()) scroll_state = 1;
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
    scroll_state = 0;
    ext_counter = 0;

    reset_lvl(SCREEN_BASE_BLOCK(31), tilemap_ptr[current_lvl], pal_ptr[current_lvl], tiles_ptr[current_lvl], tiles_len[current_lvl]);
    reset_window();
    reset_player();
    reset_enemy_ai(current_lvl);
    reset_collisions(current_lvl);

    unblack_screen();

    VBlankIntrWait();
}

static void check_level_progression()
{
    u16 absolute_x = (window_ofs << 3) + pl_get_x() + 200;
    //AGBPrintInt(absolute_x);

    if (enemies_killed >= 3 && absolute_x >= lvlTrigRegion[current_lvl][0] && absolute_x <= lvlTrigRegion[current_lvl][1])
    {
        current_lvl++;
        fade_out();
        reset_engine(current_lvl);
        pl_unhide();
        game_state = GAME_STATE_NORMAL;
        draw_window();
        fade_in();
    }
}

static void tick_state_normal_before_vblank()
{
    ext_counter = check_player_extant(pl_get_x(), pl_get_y());

    if (0 && ext_counter)
    {
        game_state = GAME_STATE_OVER;
        return;
    }

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

    draw_text_to_oam("SCORE 100", 80, 50, 40);
    draw_text_to_oam("PRESS FIRE TO RESTART", 25, 70, 50);

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
    scroll_entire();

    pl_add_bullet_to_oam();
    pl_move_bullets(scroll_state);
    pl_advance_anim_vblank();

    handle_enemies_vblank(scroll_state);
}

static void tick_state_start_vblank()
{
    VBlankIntrWait();

    clear_screen();
    //draw_text_to_oam("HAUNTED WALK", 90, 50, 40);
    draw_text_to_oam("PRESS FIRE TO START", 40, 70, 48);

    scanKeys();
    keys_down = keysDown();
    keys_held = keysHeld();

    if (keys_down & KEY_A)
    {
        black_screen();
        clear_oam();

        reset_player();
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
