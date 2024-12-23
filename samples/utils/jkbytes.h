/*
 *
 */

#ifndef __JK_BYTES__H_
#define __JK_BYTES__H_

void HexStrToByte(const char *source, unsigned char *dest, int sourceLen);

void string_reerect(char *src, char *destBuf);

int hex_set_short(unsigned char *dst, unsigned char v);

int save_int_to_hex(int v, unsigned char *dst);

int save_short_to_hex(int v, unsigned char *dst);

#endif
