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

#include <stdint.h>
#include <string.h>

#include "gf2.h"
#include "gf.h"

inline uint8_t
ffinv2(uint8_t element)
{
	return element;
}

inline uint8_t
ffadd2(uint8_t summand1, uint8_t summand2)
{
	return summand1 ^ summand2;
}

inline uint8_t
ffdiv2(uint8_t dividend, uint8_t divisor)
{
	return dividend;
}

inline uint8_t
ffmul2(uint8_t factor1, uint8_t factor2)
{
	return factor1 & factor2;
}

inline void
ffadd2_region(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region(region1, region2, length);
}

inline void
ffdiv2_region_c(uint8_t *region, uint8_t constant, int length)
{
	return;
}

inline void
ffmadd2_region_c(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		ffxor_region(region1, region2, length);
}

inline void
ffmul2_region_c(uint8_t *region, uint8_t constant, int length)
{
	if (constant == 0)
		memset(region, 0, length);
}

void
gf2_init()
{
	return;
}
