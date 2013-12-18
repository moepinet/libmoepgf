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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf.h"
#include "gf4.h"

#if GF4_POLYNOMIAL == 7
#include "gf4tables7.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t inverses[GF4_SIZE] = GF4_INV_TABLE;
static const uint8_t pt[GF4_SIZE][GF16_EXPONENT] = GF4_POLYNOMIAL_DIV_TABLE;
static const uint8_t mul[GF4_SIZE][GF4_SIZE] = GF4_MUL_TABLE;

inline uint8_t
ffinv4(uint8_t element)
{
	return inverses[element];
}

inline uint8_t
ffadd4(uint8_t summand1, uint8_t summand2)
{
	return summand1 ^ summand2;
}

inline uint8_t
ffdiv4(uint8_t dividend, uint8_t divisor)
{
	ffmul4_region_c_gpr(&dividend, inverses[divisor], 1);
	return dividend;
}

inline uint8_t
ffmul4(uint8_t factor1, uint8_t factor2)
{
	ffmul4_region_c_gpr(&factor2, factor2, 1);
	return factor1;
}

inline void
ffadd4_region_gpr(uint8_t* region1, const uint8_t* region2, int length)
{
	ffxor_region_gpr(region1, region2, length);
}

inline void
ffdiv4_region_c_gpr(uint8_t* region, uint8_t constant, int length)
{
	ffmul4_region_c_gpr(region, inverses[constant], length);
}

void
ffmadd4_region_c_slow(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_gpr(region1, region2, length);
		return;
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x55) >> 0) * p[0];
		r[1] = ((*region2 & 0xaa) >> 1) * p[1];
		*region1 ^= r[0] ^ r[1];
	}
}

inline void
ffmadd4_region_c_gpr(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];
	uint64_t r64[4];

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region2 & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region2 & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region1) ^= r64[0] ^ r64[1];
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x55) >> 0) * p[0];
		r[1] = ((*region2 & 0xaa) >> 1) * p[1];
		*region1 ^= r[0] ^ r[1];
	}
}

void
ffmul4_region_c_slow(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	for (; length; region++, length--) {
		r[0] = ((*region & 0x55) >> 0) * p[0];
		r[1] = ((*region & 0xaa) >> 1) * p[1];
		*region = r[0] ^ r[1];
	}
}

void
ffmul4_region_c_gpr(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[2];
	uint64_t r64[2];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;
	
	for (; length & 0xfffffff8; region+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region) = r64[0] ^ r64[1];
	}

	for (; length; region++, length--) {
		r[0] = ((*region & 0x55) >> 0) * p[0];
		r[1] = ((*region & 0xaa) >> 1) * p[1];
		*region = r[0] ^ r[1];
	}
}

