#define AGB_PRINT_BUFFER  ((volatile unsigned short  *) (0x09FD0000))
#define AGB_PRINT_CONTEXT ((AGBPrintContext volatile *) (0x09FE20F8))
#define AGB_PRINT_PROTECT (*(volatile unsigned short *) (0x09FE2FFE))

#include <gba.h>

typedef struct
{
    unsigned short request;
    unsigned short bank;
    unsigned short get;
    unsigned short put;
} AGBPrintContext;

void AGBPrintFlush(void)
{
    asm volatile("swi 0xFA");
}

void AGBPrintInit(void)
{
    AGB_PRINT_PROTECT = (unsigned short) 0x20;
    AGB_PRINT_CONTEXT->request = (unsigned short) 0x00;
    AGB_PRINT_CONTEXT->get = (unsigned short) 0x00;
    AGB_PRINT_CONTEXT->put = (unsigned short) 0x00;
    AGB_PRINT_CONTEXT->bank = (unsigned short) 0xFD;
    AGB_PRINT_PROTECT = (unsigned short) 0x00;
}

void AGBPutChar(const char c)
{
    unsigned short data = AGB_PRINT_BUFFER[AGB_PRINT_CONTEXT->put >> 1];

    AGB_PRINT_PROTECT = (unsigned short) 0x20;
    data = (AGB_PRINT_CONTEXT->put & 1) ? (c << 8) | (data & 0xFF) : (data & 0xFF00) | c;
    AGB_PRINT_BUFFER[AGB_PRINT_CONTEXT->put >> 1] = data;
    AGB_PRINT_CONTEXT->put++;
    AGB_PRINT_PROTECT = (unsigned short) 0x00;
}

void AGBPrintString(const char *s)
{
    while (*s)
    {
        AGBPutChar(*s);
        s++;
    }
    AGBPrintFlush();
}

void AGBPrintInt(u16 n)
{
    if (!n)
    {
        AGBPrintString("0");
        return;
    }
    char str[20] = "0";
    u16 i = 0, temp = n;

    // Count digits and fill the array in reverse
    while (temp > 0) {
        str[i++] = (temp % 10) + '0';
        temp /= 10;
    }

    // Reverse the string to get the correct order
    for (u16 j = 0; j < (i>>1); j++) {
        char t = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = t;
    }

    str[i] = '\0'; // Null terminate

    AGBPrintString(str);
}
