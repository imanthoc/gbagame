#include "Utilities.h"
#include "gba.h"

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
	}
}
