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

#ifndef _GF2_H_
#define _GF2_H_

#include <stdint.h>

#define GF2_PRIMITIVE_POLYNOMIAL	3
#define GF2_EXPONENT			1
#define GF2_SIZE			(1 << GF2_EXPONENT)
#define GF2_BITMASK			(GF2_SIZE - 1);

uint8_t
ffadd2 (const uint8_t summand1, const uint8_t summand2);

uint8_t
ffdiv2 (const uint8_t dividend, const uint8_t divisor);

uint8_t
ffmul2 (const uint8_t factor1, const uint8_t factor2);

void
ffadd2_region(uint8_t *region1, const uint8_t *region2, const int length);

void
ffdiv2_region_c(uint8_t *region, const uint8_t constant, const int length);

void
ffmadd2_region_c(uint8_t *region1, const uint8_t *region2,
				const uint8_t constant, const int length);

void
ffmul2_region_c(uint8_t *region, const uint8_t constant, const int length);

void
gf2_init();

#endif
