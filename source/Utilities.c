#include "Utilities.h"
#include "Engine.h"
#include "gba.h"
#include "Map.h"

inline void memcpy_hw(void *dst, const void *src, u16 n)
{
	u16 *__dst = (u16*)dst;
	const u16 *__src = (const u16*)src;

	n >>= 1;

	while(n--)
	{
		*__dst++ = *__src++;
	}
}

inline void clear_oam()
{
	for (u8 i = 0; i < 128; ++i)
	{
		OAM[i] = (OBJATTR) { 0, 0, 0 };
		shadow_oam[i] = (OBJATTR) { 0, 0, 0 };
	}
}

inline void clear_bg()
{
	u16 *scb = screen_block;

    for (u16 i = 0; i < 1024; ++i)
    {
        *scb++ = 0;
    }
}

inline void clear_screen()
{
	clear_oam();
	clear_bg();
}
