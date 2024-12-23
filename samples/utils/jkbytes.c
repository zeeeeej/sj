/*
 *
 *
 */

#include "jkbytes.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

void HexStrToByte(const char *source, unsigned char *dest, int sourceLen)
{
    short         i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return;
}

/* 字符串拆开，重新组装
 *  * eg: ab\xe8\x80\x83\xe5\x8b\xa4转换为
 *   *        ab鑰冨嫟(即ab考勤)
 *    */
void string_reerect(char *src, char *destBuf)
{
    int i = 0;
    int j = 0;

    while (src[j] != '\0')
    {
        if (strncmp(&src[j], "\\x", 2) != 0)
        {
            memcpy(&destBuf[i], &src[j], 1);
            i = i + 1;
            j = j + 1;
        }
        else
        {
            unsigned char *tmp = (unsigned char *)destBuf;
            HexStrToByte(&src[j + 2], &tmp[i], 2);
            i = i + 1;
            j = j + 4;
        }
    }
    destBuf[i] = '\0';

    return;
}

int hex_set_short(unsigned char *dst, unsigned char v)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        dst[1 - i] = ((v & 0x0f) > 9) ? (v & 0x0f) + 'A' : (v & 0x0f) + '0';
        v >>= 4;
    }
    return 0;
}

int save_int_to_hex(int v, unsigned char *dst)
{
    dst[0] = v & 0xff;
    dst[1] = (v >> 8) & 0xff;
    dst[2] = (v >> 16) & 0xff;
    dst[3] = (v >> 24) & 0xff;
    return 0;
}

int save_short_to_hex(int v, unsigned char *dst)
{
    dst[0] = v & 0xff;
    dst[1] = (v >> 8) & 0xff;
    return 0;
}
