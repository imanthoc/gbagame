#include <stdlib.h>

#include "Player.h"
#include "Engine.h"
#include "Enemy_ai.h"
#include "Utilities.h"
#include "Map.h"
#include "agb.h"

#define ENEMY_DEATH_ANIM_FRAMES 60
#define BOSS_ANIM_FRAMES 48
// self explanatory
static s8 enemy_hp[MAX_ACTIVE_ENEMIES];
// animation counters
static u8 next_frame = 1;

static u8 current_lvl = 0;

static u16 boss_hp = 500;
static u16 boss_anim_x_counter = 0;
static u8 boss_anim_y_counter = 0;
static u8 boss_direction = 255;

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

static s8 boss_anim_y_offset[BOSS_ANIM_FRAMES] = {
    2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -2, -2, -2,
    -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1
};

u8 is_boss_dead() { return boss_hp == 0; }

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

    boss_hp = 500;
    boss_anim_x_counter = 0;
    boss_anim_y_counter = 0;
    boss_direction = 255;

    if (current_lvl == 2) // BOSS LEVEL
    {
        shadow_oam[BOSS_OAM_INDEX].attr0 = OBJ_Y(60) | ATTR0_COLOR_16 | ATTR0_SQUARE;
        shadow_oam[BOSS_OAM_INDEX].attr1 = OBJ_X(255) | ATTR1_SIZE_64;
        shadow_oam[BOSS_OAM_INDEX].attr2 = ATTR2_PALETTE(1) | OBJ_CHAR(BOSS_TILE_INDEX) | ATTR2_PRIORITY(0);
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

    return 0;
}

u8 check_extant_from_boss(u16 pl_x, u8 pl_y)
{
    if (current_lvl != 2) return 0;

    u16 boss_x = (shadow_oam[BOSS_OAM_INDEX].attr1 & 0x1FF) + 10;
    u8  boss_y = (shadow_oam[BOSS_OAM_INDEX].attr0 & 0xFF) + 10;

    // first check if the player is touching an enemy THAT IS ALLIVE
    return (boss_hp > 0 && pl_x >= boss_x && pl_x <= boss_x + 32 && pl_y >= boss_y && pl_y <= boss_y + 32);
}

static void add_enemy()
{
    static u8 enemy_delay = 0;

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
}

// TODO: FIX u8 / u16 mismatches for the x coordinate inside the code
static void check_enemy_damage()
{
    u8 k = 0;
    for (u8 i = ENEMY_OAM_INDEX; i < ENEMY_OAM_INDEX + MAX_ACTIVE_ENEMIES; i++)
    {
        u8 en_y = 112; // y coordinate is the same for all enemies
        u16 en_x = shadow_oam[i].attr1 & 0x1FF;

        u8 boss_y = shadow_oam[BOSS_OAM_INDEX].attr0 & 0xFF;
        u16 boss_x = shadow_oam[BOSS_OAM_INDEX].attr1 & 0x1FF;


        for (u8 j = PLAYER_BULLETS_OAM_INDEX; j < PLAYER_BULLETS_OAM_INDEX + PLAYER_MAX_ACTIVE_BULLETS; ++j)
        {
            u8 bullet_x = shadow_oam[j].attr1 & 0x1FF;
            u8 bullet_y = shadow_oam[j].attr0 & 0xFF;

            if (enemy_hp[k] > 0 && bullet_x >= en_x && bullet_x <= en_x + 16 && bullet_y >= en_y && bullet_y <= en_y + 32)
            {
                enemy_hp[k] -= 1;

                shadow_oam[j] = (OBJATTR) { 0, 0, 0 };
            }

            if (current_lvl == 2 && boss_hp > 0 && bullet_x >= boss_x && bullet_x <= boss_x + 16 && bullet_y >= boss_y && bullet_y <= boss_y + 32)
            {
                boss_hp -= 1;

                shadow_oam[j] = (OBJATTR) { 0, 0, 0 };
            }
        }

        k++;
    }
}

static void move_boss_to_direction()
{
    u16 cur_x = shadow_oam[BOSS_OAM_INDEX].attr1 & 0x1FF;
    u8 cur_y  = shadow_oam[BOSS_OAM_INDEX].attr0 & 0xFF;

    switch (boss_direction)
    {
        case 0:
            if (cur_x > 0) cur_x--;
            else boss_direction = 1;
        break;

        case 1:
            if (cur_x < 200) cur_x++;
            else boss_direction = 0;
        break;

        case 2:
            if (cur_y > 5) cur_y--;
            else boss_direction = 3;
        break;

        case 3:
            if (cur_y < 100) cur_y++;
            else boss_direction = 2;
        break;
    }
    u16 pl_x = pl_get_x();
    u16 flip = (pl_x > cur_x) * ATTR1_FLIP_X;
    shadow_oam[BOSS_OAM_INDEX].attr1 = OBJ_X(cur_x) | ATTR1_SIZE_64 | flip;
    shadow_oam[BOSS_OAM_INDEX].attr0 = OBJ_Y(cur_y) | ATTR0_COLOR_16 | ATTR0_SQUARE;
}

static void move_boss()
{
    if (current_lvl != 2) return;

    // animate it up and down
    u8 cur_y = shadow_oam[BOSS_OAM_INDEX].attr0 & 0xFF;

    cur_y += boss_anim_y_offset[boss_anim_y_counter];

    shadow_oam[BOSS_OAM_INDEX].attr0 = OBJ_Y(cur_y) | ATTR0_COLOR_16 | ATTR0_SQUARE;

    if (boss_anim_y_counter == BOSS_ANIM_FRAMES-1) boss_anim_y_counter = 0;
    else boss_anim_y_counter++;

    if (boss_anim_x_counter <= 60*3)
    {
        boss_anim_x_counter++;
    }
    else if (boss_anim_x_counter <= 60*5)
    {
        u16 cur_x = shadow_oam[BOSS_OAM_INDEX].attr1 & 0x1FF;
        shadow_oam[BOSS_OAM_INDEX].attr1 = OBJ_X(cur_x - 1) | ATTR1_SIZE_64;

        boss_anim_x_counter++;

    }
    else if (boss_anim_x_counter <= 60*6)
    {
        boss_anim_x_counter++;

        boss_direction = rand() & 3;
    }
    else if (boss_anim_x_counter <= 60*8)
    {
        move_boss_to_direction();

        boss_anim_x_counter++;
    }
    else
    {
        boss_anim_x_counter = 60*6;
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
}

u8 handle_enemies(s8 scroll_state)
{
    u8 k = clear_extanct_enemies();

    add_enemy();
    advance_anim(scroll_state);

    check_enemy_damage();

    move_enemies(scroll_state);
    move_boss();

    return k; // number of enemies killed by bullets
}


