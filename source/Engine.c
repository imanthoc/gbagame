#include "Engine.h"
#include "Collisions.h"
#include "Utilities.h"
#include "Enemy_ai.h"
#include "font.h"

#define GAME_STATE_START 0
#define GAME_STATE_NORMAL 1
#define GAME_STATE_RESTART 2
#define GAME_STATE_OVER 3

#include "agb.h"

static u8 game_state = GAME_STATE_NORMAL;
static u8 tries = 3;
static u8 bullet_timer = 0;

static u16 keys_held = 0xFFFF;
static u16 keys_down = 0xFFFF;
static s8 scroll_state = 0;
static u8 y;
    // x is relative to window, not absolute
static u16 x;
static u8 counter;
static u8 added_bullet_index;

static u8 anim_delay = 0;
static u8 frame_oam_index = 1;
static u8 next_frame = 1;

static u8 ext_counter = 0;
static const u8 death_anim_sprite[60] = {
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89
};



// Gravity calculations are very heavy and are done before vblank
static void tick_gravity(u8 trigger_jump)
{
    u8 cm = can_move_down(x,  y);

    if (!counter && trigger_jump) counter = 1;

    if (!counter && cm)
    {
        counter = 16;
        ++y;
    }

    if (counter > 0 && counter <= 15)
    {
        u8 speed = (15 - counter++) >> 1;
        y -= speed;
    }
    else if (cm)
    {
        u8 speed = (counter++ - 15) >> 1;

        if (speed > 10) speed = 10;
        while (!can_move_down_offs(x, y, speed)) speed--;

        if (!speed) speed = 1;
        y += speed;
    }
    else
    {
        counter = 0;
    }
}

static void move_bullets()
{
    u8 oam_bullet_index = PLAYER_OAM_INDEX + 1;
    u8 oam_bullet_limit = oam_bullet_index + PLAYER_MAX_ACTIVE_BULLETS;

    for (u8 i = oam_bullet_index; i < oam_bullet_limit; ++i)
    {
        if (OAM[i].attr0)
        {
            u8 cur_x = OAM[i].attr1 & (0x1FF);
            u16 flip = OAM[i].attr1 & ATTR1_FLIP_X;

            OAM[i].attr1 = OBJ_X(cur_x + scroll_state + (flip? -BULLET_SPEED : BULLET_SPEED)) | ATTR1_SIZE_8 | flip;


            if (cur_x >= 200 || cur_x <= 10)
            {
                OAM[i].attr0 = 0;
                OAM[i].attr1 = 0;
                OAM[i].attr2 = 0;
            }
        }
    }
}

static void add_bullet_to_array()
{
    //Each character's bullet array is one slot after it's sprite's oam index
    u8 oam_bullet_index = PLAYER_OAM_INDEX + 1;
    u8 oam_bullet_limit = oam_bullet_index + PLAYER_MAX_ACTIVE_BULLETS;

    while(OAM[oam_bullet_index].attr0 != 0 && oam_bullet_index < oam_bullet_limit) oam_bullet_index++;

    added_bullet_index = oam_bullet_index;
}

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

static inline void add_bullet_to_oam()
{
    if (added_bullet_index < PLAYER_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS)
    {
        if (OAM[PLAYER_OAM_INDEX].attr1 & (ATTR1_FLIP_X))
        {
            OAM[added_bullet_index].attr0 = OBJ_Y(y + 10) | ATTR0_COLOR_16 | ATTR0_SQUARE;
            OAM[added_bullet_index].attr1 = OBJ_X(x - 8) | ATTR1_SIZE_8 | ATTR1_FLIP_X;
        }
        else
        {
            OAM[added_bullet_index].attr0 = OBJ_Y(y + 10) | ATTR0_COLOR_16 | ATTR0_SQUARE;
            OAM[added_bullet_index].attr1 = OBJ_X(x + 16) | ATTR1_SIZE_8;
        }

        OAM[added_bullet_index].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(BULLET_OAM_INDEX) | ATTR2_PRIORITY(0);
    }
}

static inline void handle_player_bullets()
{
    if (keys_held & KEY_A)
    {
        if (bullet_timer++ == BULLET_DELAY)
        {
            add_bullet_to_array();
            bullet_timer = 0;
        }
        else
        {
            added_bullet_index = PLAYER_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS;
        }
    }
    else
    {
        added_bullet_index = PLAYER_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS;
    }
}



static inline void scroll_entire()
{
    if (keys_held & KEY_RIGHT)
    {
        OAM[PLAYER_OAM_INDEX].attr1 &= ~(ATTR1_FLIP_X);

        if (can_move_right(x, y))
        {
            u8 sw = scroll_right();
            if (!sw) scroll_state = -1;
            else scroll_state = 0;
        }
        else
        {
            scroll_state = 0;
        }
    }
    else if (keys_held & KEY_LEFT)
    {
        OAM[PLAYER_OAM_INDEX].attr1 |= (ATTR1_FLIP_X);

        if (can_move_left(x, y))
        {
            u8 sw = scroll_left();
            if (!sw) scroll_state = 1;
            else scroll_state = 0;
        }
        else
        {
            scroll_state = 0;
        }
    }
    else
    {
        scroll_state = 0;
    }
}

static void advance_anim_before_vblank()
{
    if (keys_held & KEY_LEFT || keys_held & KEY_RIGHT)
    {
        if (anim_delay == 3)
        {
            next_frame = 1 + frame_oam_index*9;

            frame_oam_index++;
            anim_delay = 0;

            if (frame_oam_index == 9) frame_oam_index = 1;
        }
        else
        {
            anim_delay++;
        }
    }
    else
    {
        next_frame = 1;
    }
}

static void advance_anim_vblank()
{
    OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(next_frame) | ATTR2_PRIORITY(0);
}

void reset_engine()
{
    clear_oam();

    x = 90;
    y = 80;

    OAM[PLAYER_OAM_INDEX].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_TALL;
    OAM[PLAYER_OAM_INDEX].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
    OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(1) | ATTR2_PRIORITY(0);
    REG_BLDY = 0;

    game_state = GAME_STATE_START;

    bullet_timer = 0;

    keys_held = 0xFFFF;
    keys_down = 0xFFFF;
    scroll_state = 0;
    counter = 0;
    added_bullet_index = PLAYER_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS;

    anim_delay = 0;
    frame_oam_index = 1;
    next_frame = 1;

    ext_counter = 0;

    reset_window();
    reset_enemy_ai();
}

static void tick_state_normal_before_vblank()
{
    AGBPrintString("asdasdasd");
    // this info comes from enemy_ai.c
    // as long as ext_counter == 0 the game is playing
    // when ext_counter == 1 death animation starts
    ext_counter = check_player_extant(x, y);
    if (ext_counter)
    {
        tries--;
        game_state = GAME_STATE_RESTART;
    }

    advance_anim_before_vblank();
    tick_gravity(keys_down & KEY_B);
    handle_player_bullets();
    handle_enemies_before_vblank();
}



static void tick_state_over_vblank()
{
    clear_oam();

    draw_text_to_oam("SCORE 100", 80, 50, 40);
    draw_text_to_oam("PRESS FIRE TO RESTART", 25, 70, 50);

    do
    {
        scanKeys();
        keys_down = keysDown();
        VBlankIntrWait();

    } while (!(keys_down & KEY_A));

    tries = 3;
    reset_engine();
    REG_BLDY = 16;

    VBlankIntrWait();
}

static void tick_state_restart_vblank()
{
    if (ext_counter >= 59)
    {
        ext_counter = 59;
        REG_BLDCNT = 1 | (3 << 6); // set background and blend mode

        for (u8 i = 0; i <= 16*4; i++)
        {
            REG_BLDY = i >> 2;
            VBlankIntrWait();
        }

        reset_window();
        draw_window();

        for (u8 i = 0; i < 16*3; ++i)
        {
            VBlankIntrWait();
        }

        if (!tries)
        {
            game_state = GAME_STATE_OVER;
        }
        else
        {
            reset_engine();
            game_state = GAME_STATE_NORMAL;
        }
    }
    else
    {
        OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(death_anim_sprite[ext_counter++]) | ATTR2_PRIORITY(0);
    }
}

static void tick_state_normal_vblank()
{
    // key state is set before vblank
    // Move player according to gravity calculated before vblank
    OAM[PLAYER_OAM_INDEX].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_TALL;

    // map scrolling NEEDS to happen during vblank
    scroll_entire();

    add_bullet_to_oam();
    move_bullets();

    advance_anim_vblank();

    handle_enemies_vblank(scroll_state);
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

static void init_player()
{
    x = 90;
    y = 80;

    OAM[PLAYER_OAM_INDEX].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_TALL;
    OAM[PLAYER_OAM_INDEX].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
    OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(1) | ATTR2_PRIORITY(0);
}

static void tick_state_start_vblank()
{
    clear_oam();
    draw_text_to_oam("HAUNTED WALK", 90, 50, 40);
    draw_text_to_oam("PRESS FIRE TO START", 40, 70, 48);

    scanKeys();
    keys_down = keysDown();
    keys_held = keysHeld();

    REG_BLDCNT = 1 | (3 << 6); // set background and blend mode
    REG_BLDY = 16;

    if (keys_down & KEY_A)
    {
        clear_oam();
        init_player();
        draw_window();

        game_state = GAME_STATE_NORMAL;

        for (u8 i = 16*4; i > 0; i--)
        {
            REG_BLDY = i >> 2;
            VBlankIntrWait();
        }
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

        case GAME_STATE_RESTART: // this isn't very important, it is all done during vblank
            tick_state_restart_vblank();
        break;

        case GAME_STATE_OVER:
            tick_state_over_vblank();
        break;
    }
}
