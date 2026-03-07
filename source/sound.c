#include <gba_dma.h>
#include <gba_sound.h>
#include <gba_timers.h>
#include <math.h>
#include <stdint.h>
#include "sound.h"
#include "theme.h"
#include "shot.h"
#include "agb.h"

void init_sound()
{
    SNDSTAT = SNDSTAT_ENABLE;
    DSOUNDCTRL =    DSOUNDCTRL_AR |
                    DSOUNDCTRL_AL |
                    DSOUNDCTRL_A50 |
                    DSOUNDCTRL_ATIMER(0) |
                    DSOUNDCTRL_ARESET |

                    DSOUNDCTRL_BR |
                    DSOUNDCTRL_BL |
                    DSOUNDCTRL_B100 |
                    DSOUNDCTRL_BTIMER(1) |
                    DSOUNDCTRL_BRESET;

    DSOUNDCTRL &= ~DSOUNDCTRL_ARESET;
    DSOUNDCTRL &= ~DSOUNDCTRL_BRESET;
}


void theme_play()
{
    // stop DMA
    REG_DMA1CNT = 0;

    // stop timer
    REG_TM0CNT = 0;

    // set reset value and start timer
    REG_TM0CNT_L = 64488;
    REG_TM0CNT_H = (1 << 7);

    REG_DMA1SAD = (u32)output_raw;
    REG_DMA1DAD = (u32)&REG_FIFO_A;
    REG_DMA1CNT =   DMA_DST_FIXED |
                    DMA_SRC_INC |
                    DMA_REPEAT |
                    DMA16 |
                    DMA_SPECIAL |
                    DMA_ENABLE;
}

void shot_play()
{
    // stop DMA
    REG_DMA2CNT = 0;

    // stop timer
    REG_TM1CNT = 0;

    // set reset value and start timer
    REG_TM1CNT_L = 64488;
    REG_TM1CNT_H = (1 << 7);

    REG_DMA2SAD = (u32)st_raw;
    REG_DMA2DAD = (u32)&REG_FIFO_B;
    REG_DMA2CNT =   DMA_DST_FIXED |
                    DMA_SRC_INC |
                    DMA_REPEAT |
                    DMA16 |
                    DMA_SPECIAL |
                    DMA_ENABLE;
}

void shot_stop()
{
    REG_DMA2CNT = 0;
    REG_TM1CNT = 0;
}

void sound_irq()
{
    static u16 theme_counter = 0;
    static u8 bullet_counter = 0;

    if (theme_counter++ == THEME_FRAMES-15)
    {
        theme_play();
        theme_counter = 0;
    }

    if (bullet_counter++ == 10)
    {
        shot_stop();
        bullet_counter = 0;
    }
}
