/* Copyright (C) 2024 by Salvatore Sanfilippo -- All rights reserved.
 * This code is licensed under the MIT license. See LICENSE file for info. */

#ifndef SMAZ2_H
#define SMAZ2_H

unsigned long smaz2_compress(unsigned char *dst, unsigned long dstlen, unsigned char *s, unsigned long len);
unsigned long smaz2_decompress(unsigned char *dst, unsigned long dstlen, unsigned char *c, unsigned long len);

#endif
