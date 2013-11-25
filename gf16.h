/*
 * This is a library providing arithmetic functions on GF(2^1) and GF(2^8).
 * Copyright (C) 2013  Alexander Kurtz <alexander@kurtz.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _GF16_H_
#define _GF16_H_

#include "gf.h"

#define GF16_PRIMITIVE_POLYNOMIAL	(19 & 0x0f)
#define GF16_EXPONENT			4
#define GF16_SIZE			(1 << GF16_EXPONENT)
#define GF16_BITMASK			(GF16_SIZE - 1);

uint8_t
ffadd16(const uint8_t summand1, const uint8_t summand2);

uint8_t
ffdiv16(uint8_t dividend, const uint8_t divisor);

uint8_t
ffmul16(uint8_t factor1, const uint8_t factor2);

void 
ffadd16_region(uint8_t *region1, const uint8_t *region2, const int length);

void
ffdiv16_region_c(uint8_t *region, const uint8_t constant, const int length);

void
ffmadd16_region_c_slow(uint8_t *region1, const uint8_t *region2,
					const uint8_t constant, int length);

void
ffmadd16_region_c(uint8_t *region1, const uint8_t *region2,
					const uint8_t constant, int length);

void
ffmul16_region_c_slow(uint8_t *region, const uint8_t constant, int length);

void
ffmul16_region_c(uint8_t *region, const uint8_t constant, int length);


void
gf16_init();

#endif
