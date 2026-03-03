#include <stdlib.h>

#include "Engine.h"
#include "Enemy_ai.h"
#include "Utilities.h"
#include "Map.h"
#include "agb.h"


// delay counter for the next enemy to be spawned
static u8 enemy_delay = 0;

// self explanatory
static s8 enemy_hp[4];
// animation counters
static u8 en_anim_delay = 0;
static u8 frame_oam_index = 1;
static u8 next_frame = 1;

static u8 current_lvl = 0;

// TODO: fix this stupid animation
static u8 death_anim_counter[4];

static const u8 death_anim_sprite[60] = {
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89,
    81, 81, 81, 81, 81, 89, 89, 89, 89, 89
};

void reset_enemy_ai(u8 l)
{
    current_lvl = l;

    enemy_delay = 0;

    enemy_hp[0] = 0;
    enemy_hp[1] = 0;
    enemy_hp[2] = 0;
    enemy_hp[3] = 0;

    en_anim_delay = 0;
    frame_oam_index = 1;
    next_frame = 1;

    death_anim_counter[0] = 0;
    death_anim_counter[1] = 0;
    death_anim_counter[2] = 0;
    death_anim_counter[3] = 0;
}

static u8 clear_extanct_enemies()
{
    u8 killed_enemies = 0;
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
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
            if ((!flip && (cur_x >= 240 && cur_x < 255) ) || (flip && !cur_x) || enemy_hp[k] <= -55)
            {
                death_anim_counter[k] = 0;
                shadow_oam[i] = (OBJATTR){ 0, 0, 0 };

                if (enemy_hp[k] <= -55) killed_enemies++;
            }
        }

        k++;
    }

    return killed_enemies;
}

static inline void move_enemies(s8 scroll_state)
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
    {
        if (shadow_oam[i].attr0)
        {
            u8 cur_x = shadow_oam[i].attr1 & 0x1FF;

            if (enemy_hp[k] > 0)
            {
                if (shadow_oam[i].attr1 & ATTR1_FLIP_X)
                {
                    shadow_oam[i].attr1 = OBJ_X(cur_x + scroll_state*2 - 1) | ATTR1_SIZE_32 | ATTR1_FLIP_X;
                }
                else
                {
                    shadow_oam[i].attr1 = OBJ_X(cur_x + scroll_state*2 + 1) | ATTR1_SIZE_32;
                }
            }
            else
            {
                shadow_oam[i].attr1 = OBJ_X(cur_x + scroll_state) | ATTR1_SIZE_32;
            }
        }

        k++;
    }
}

u8 check_player_extant(u16 pl_x, u8 pl_y)
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
    {
        u16 en_x = shadow_oam[i].attr1 & 0x1FF;
        u8  en_y = 112;

        // first check if the player is touching an enemy THAT IS ALLIVE
        if (enemy_hp[k] > 0 && pl_x >= en_x && pl_x <= en_x + 16 && pl_y >= en_y && pl_y <= en_y + 32) return 1;

        k++;
    }

    return 0;
}

static void add_enemy()
{
    if (enemy_delay == ENEMY_DELAY)
    {
        u8 k = 0;
        for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
        {
            if (!shadow_oam[i].attr0)
            {
                enemy_hp[k] = 100;

                shadow_oam[i].attr0 = OBJ_Y(112) | ATTR0_COLOR_16 | ATTR0_TALL;

                if (rand() > 1073741823) shadow_oam[i].attr1 = OBJ_X(255) | ATTR1_SIZE_32 | ATTR1_FLIP_X;
                else  shadow_oam[i].attr1 = OBJ_X(255) | ATTR1_SIZE_32;

                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(1) | ATTR2_PRIORITY(1);

                break;
            }
            k++;
        }
        enemy_delay = 0;
    }
    else
    {
        enemy_delay++;
    }
}

// TODO: FIX u8 / u16 mismatches for the x coordinate inside the code
static void check_enemy_damage()
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
    {
        u8 en_y = 112; // y coordinate is the same for all enemies
        u8 en_x = shadow_oam[i].attr1 & 0x1FF;

        for (u8 j = PLAYER_BULLETS_OAM_INDEX; j < PLAYER_BULLETS_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS; ++j)
        {
            u8 bullet_x = shadow_oam[j].attr1 & 0x1FF;
            u8 bullet_y = shadow_oam[j].attr0 & 0xFF;

            if (enemy_hp[k] > 0 && bullet_x >= en_x && bullet_x <= en_x + 16 && bullet_y >= en_y && bullet_y <= en_y + 32)
            {
                enemy_hp[k] -= 20;

                shadow_oam[j] = (OBJATTR) { 0, 0, 0 };
            }

        }

        k++;
    }
}

static void advance_anim(s8 scroll_state)
{
    if (en_anim_delay == 3)
    {
        next_frame = 1 + frame_oam_index*9;

        if (frame_oam_index == 8) frame_oam_index = 1;
        else frame_oam_index++;

        en_anim_delay = 0;
    }
    else
    {
        en_anim_delay++;
    }

    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + 4; i++)
    {
        if (shadow_oam[i].attr1)
        {
            if (enemy_hp[k] > 0)
            {
                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(next_frame) | ATTR2_PRIORITY(0);
            }
            else
            {
                u16 x = shadow_oam[i].attr1 & 0x1FF;

                shadow_oam[i].attr1 = OBJ_X(x + scroll_state) | ATTR1_SIZE_32;
                shadow_oam[i].attr2 = ATTR2_PALETTE(0) | OBJ_CHAR(death_anim_sprite[death_anim_counter[k]]) | ATTR2_PRIORITY(0);

                death_anim_counter[k]++; // will be reset to 0 when the enemy is extanct
            }
        }

        k++;
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


