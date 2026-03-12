#include <stdlib.h>

#include "Engine.h"
#include "Enemy_ai.h"
#include "Utilities.h"
#include "Map.h"
#include "agb.h"

#define ENEMY_DEATH_ANIM_FRAMES 60

// self explanatory
static s8 enemy_hp[MAX_ACTIVE_ENEMIES];
// animation counters
static u8 next_frame = 1;

static u8 current_lvl = 0;

// TODO: fix this stupid animation
static u8 death_anim_counter[MAX_ACTIVE_ENEMIES];

static const u16 death_anim_sprite[ENEMY_DEATH_ANIM_FRAMES] = {
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    201, 201, 201, 201, 201, 201, 201, 201, 201, 201,
    201, 201, 201, 201, 201, 201, 201, 201, 201, 201,
    210, 210, 210, 210, 210, 210, 210, 210, 210, 210,
    210, 210, 210, 210, 210, 210, 210, 210, 210, 210
};

void reset_enemy_ai(u8 l)
{
    current_lvl = l;

    for (u8 i = 0; i < MAX_ACTIVE_ENEMIES; ++i)
    {
        enemy_hp[i] = 0;
    }

    next_frame = 1;

    for (u8 i = 0; i < MAX_ACTIVE_ENEMIES; ++i)
    {
        death_anim_counter[i] = 0;
    }
}

static u8 clear_extanct_enemies()
{
    u8 killed_enemies = 0;
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        if (shadow_oam[i].attr0)
        {
            u16 cur_x = shadow_oam[i].attr1 & 0x1FF;
            u16 flip  = shadow_oam[i].attr1 & ATTR1_FLIP_X;

            if (enemy_hp[k] <= 0)
            {
                enemy_hp[k] -= 1;
            }
            // check offscreen or health under limit
            if ((!flip && (cur_x >= 240 && cur_x < 255) ) || (flip && cur_x <= 1) || enemy_hp[k] <= -59)
            {
                death_anim_counter[k] = 0;
                shadow_oam[i] = (OBJATTR){ 0, 0, 0 };

                if (enemy_hp[k] <= -59) killed_enemies++;
            }
        }

        k++;
    }

    if (shadow_oam[BAT_OAM_INDEX].attr0)
    {
        u16 bat_x = shadow_oam[BAT_OAM_INDEX].attr1 & 0x1FF;
        u16 flip  = shadow_oam[BAT_OAM_INDEX].attr1 & ATTR1_FLIP_X;

        AGBPrintInt(flip);
        AGBPrintInt(bat_x);

        if ((!flip && bat_x <= 1) || (flip && bat_x >= 240 && bat_x < 255))
        {
            shadow_oam[BAT_OAM_INDEX] = (OBJATTR){ 0, 0, 0 };
        }
    }

    return killed_enemies;
}

static inline void move_enemies(s8 scroll_state)
{
    // speed becomes 1 once every two frames to make enemies slower
    static u8 speed = 0;
    if (speed++)
    {
        speed = 0;
    }

    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        if (shadow_oam[i].attr0)
        {
            u8 cur_x = shadow_oam[i].attr1 & 0x1FF;
            u16 flip = shadow_oam[i].attr1 & ATTR1_FLIP_X;

            if (enemy_hp[k] > 0)
            {
                shadow_oam[i].attr1 = OBJ_X(cur_x + scroll_state*2 + ((flip)?-speed:+speed)) | ATTR1_SIZE_32 | flip;
            }
            else
            {
                shadow_oam[i].attr1 = OBJ_X(cur_x + scroll_state*2) | ATTR1_SIZE_32 | flip;
            }
        }

        k++;
    }

    if (shadow_oam[BAT_OAM_INDEX].attr0)
    {
        u8 cur_x = shadow_oam[BAT_OAM_INDEX].attr1 & 0x1FF;
        u16 flip = shadow_oam[BAT_OAM_INDEX].attr1 & ATTR1_FLIP_X;

        shadow_oam[BAT_OAM_INDEX].attr1 = OBJ_X(cur_x + scroll_state*2 + ((flip)?2:-2)) | ATTR1_SIZE_16 | flip;
    }
}

u8 check_extant_from_enemy(u16 pl_x, u8 pl_y)
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        u16 en_x = shadow_oam[i].attr1 & 0x1FF;
        u8  en_y = 112;

        // first check if the player is touching an enemy THAT IS ALLIVE
        if (enemy_hp[k] > 0 && pl_x >= en_x && pl_x <= en_x + 16 && pl_y >= en_y && pl_y <= en_y + 32) return 1;

        k++;
    }

    if (shadow_oam[BAT_OAM_INDEX].attr0)
    {
        u16 bat_x = shadow_oam[BAT_OAM_INDEX].attr1 & 0x1FF;
        u8 bat_y = 75;

        if (pl_x >= bat_x && pl_x <= bat_x + 16 && pl_y >= bat_y && pl_y <= bat_y + 14) return 1;
    }

    return 0;
}

static void add_enemy()
{
    static u8 enemy_delay = 0;
    static u16 bat_delay = 0;

    if (enemy_delay++ == ENEMY_DELAY)
    {
        u8 k = 0;
        for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
        {
            if (!shadow_oam[i].attr0)
            {
                enemy_hp[k] = 8;
                u16 flip = ATTR1_FLIP_X * (rand() > 1073741823);

                shadow_oam[i].attr0 = OBJ_Y(112) | ATTR0_COLOR_16 | ATTR0_TALL;
                shadow_oam[i].attr1 = OBJ_X(255) | ATTR1_SIZE_32 | flip;
                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(ENEMY_TILE_INDEX) | ATTR2_PRIORITY(1);

                break;
            }
            k++;
        }
        enemy_delay = 0;
    }

    if (bat_delay++ == 120*2)
    {
        u16 flip = ATTR1_FLIP_X * (rand() > 1073741823);

        shadow_oam[BAT_OAM_INDEX].attr0 = OBJ_Y(75) | ATTR0_COLOR_16 | ATTR0_SQUARE;
        shadow_oam[BAT_OAM_INDEX].attr1 = OBJ_X(255) | ATTR1_SIZE_16 | flip;
        shadow_oam[BAT_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(BAT_TILE_INDEX+4) | ATTR2_PRIORITY(1);

        bat_delay = 0;
    }
}

// TODO: FIX u8 / u16 mismatches for the x coordinate inside the code
static void check_enemy_damage()
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        u8 en_y = 112; // y coordinate is the same for all enemies
        u8 en_x = shadow_oam[i].attr1 & 0x1FF;

        for (u8 j = PLAYER_BULLETS_OAM_INDEX; j < PLAYER_BULLETS_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS; ++j)
        {
            u8 bullet_x = shadow_oam[j].attr1 & 0x1FF;
            u8 bullet_y = shadow_oam[j].attr0 & 0xFF;

            if (enemy_hp[k] > 0 && bullet_x >= en_x && bullet_x <= en_x + 16 && bullet_y >= en_y && bullet_y <= en_y + 32)
            {
                enemy_hp[k] -= 1;

                shadow_oam[j] = (OBJATTR) { 0, 0, 0 };
            }

        }

        k++;
    }
}

static void advance_anim(s8 scroll_state)
{
    static u8 frame_oam_index = 1;
    static u8 en_anim_delay = 0;
    static u8 bat_anim_delay = 0;

    if (en_anim_delay == 6)
    {
        next_frame = ENEMY_TILE_INDEX + frame_oam_index*9;

        if (frame_oam_index == 7) frame_oam_index = 1;
        else frame_oam_index++;

        en_anim_delay = 0;
    }
    else
    {
        en_anim_delay++;
    }

    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        if (shadow_oam[i].attr1)
        {
            if (enemy_hp[k] > 0)
            {
                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(next_frame) | ATTR2_PRIORITY(0);
            }
            else
            {
                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(death_anim_sprite[death_anim_counter[k]]) | ATTR2_PRIORITY(0);

                death_anim_counter[k]++; // will be reset to 0 when the enemy is extanct
            }
        }

        k++;
    }

    if (shadow_oam[BAT_OAM_INDEX].attr0)
    {
        if (bat_anim_delay++ <= 8)
        {
            shadow_oam[BAT_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(BAT_TILE_INDEX) | ATTR2_PRIORITY(0);
        }
        else if (bat_anim_delay++ <= 16)
        {
            shadow_oam[BAT_OAM_INDEX].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(BAT_TILE_INDEX+4) | ATTR2_PRIORITY(0);
        }
        else
        {
            bat_anim_delay = 0;
        }
    }
}

u8 handle_enemies(s8 scroll_state)
{
    u8 k = clear_extanct_enemies();

    add_enemy();
    advance_anim(scroll_state);

    check_enemy_damage();

    move_enemies(scroll_state);

    return k; // number of enemies killed by bullets
}


