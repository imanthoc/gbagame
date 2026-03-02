#include "Player.h"
#include "Collisions.h"
#include "Utilities.h"
#include "agb.h"

static u8 anim_delay = 0;
static u8 frame_oam_index = 1;
static u8 next_frame = 1;
static u8 y;
    // x is relative to window, not absolute
static u16 x;
static u8 counter;
static u8 added_bullet_index;
static u8 bullet_timer = 0;
static u8 current_lvl = 0;

static const u8 death_anim_sprite[60] = {
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89
};

inline u8 pl_get_y() { return y; }
inline u16 pl_get_x() { return x; }

u8 pl_is_centered()
{
    return x == 110;
}

void reset_player(u8 c)
{
    current_lvl = c;
    x = 110;
    y = 112;

    counter = 0;
    added_bullet_index = PLAYER_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS;

    anim_delay = 0;
    frame_oam_index = 1;
    next_frame = 1;

    bullet_timer = 0;

    OAM[PLAYER_OAM_INDEX].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_TALL | ATTR0_DISABLED;
    OAM[PLAYER_OAM_INDEX].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
    OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(1) | ATTR2_PRIORITY(0);
}

void pl_set_y_value(u8 _y) { y = _y; }

void pl_set_y()
{
    OAM[PLAYER_OAM_INDEX].attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_TALL;
}

void pl_hide()
{
    OAM[PLAYER_OAM_INDEX].attr0 |= ATTR0_DISABLED;
}

u8 pl_can_scroll_left()
{
    return x > 0;
}

u8 pl_can_scroll_right()
{
    return x < 224;
}

void pl_unhide()
{
    OAM[PLAYER_OAM_INDEX].attr0 &= ~ATTR0_DISABLED;
}

void pl_scroll_left()
{
    x -= 2;
    OAM[PLAYER_OAM_INDEX].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
}

void pl_scroll_right()
{
    x += 2;
    OAM[PLAYER_OAM_INDEX].attr1 = OBJ_X(x) | ATTR1_SIZE_32;
}

void pl_advance_anim_before_vblank(u16 keys_held)
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

void pl_advance_anim_vblank()
{
    OAM[PLAYER_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(next_frame) | ATTR2_PRIORITY(0);
}

// Gravity calculations are very heavy and are done before vblank
void pl_tick_gravity(u8 trigger_jump)
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

inline void pl_add_bullet_to_oam()
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

static void pl_add_bullet_to_array()
{
    //Each character's bullet array is one slot after it's sprite's oam index
    u8 oam_bullet_index = PLAYER_OAM_INDEX + 1;
    u8 oam_bullet_limit = oam_bullet_index + PLAYER_MAX_ACTIVE_BULLETS;

    while(OAM[oam_bullet_index].attr0 != 0 && oam_bullet_index < oam_bullet_limit) oam_bullet_index++;

    added_bullet_index = oam_bullet_index;
}

inline void pl_handle_player_bullets(u16 keys_held)
{
    if (keys_held & KEY_A)
    {
        if (bullet_timer++ == BULLET_DELAY)
        {
            pl_add_bullet_to_array();
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

void pl_move_bullets(s8 scroll_state)
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


