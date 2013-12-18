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

#include <emmintrin.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf.h"
#include "gf16.h"

#if GF16_POLYNOMIAL == 19
#include "gf16tables19.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t inverses[GF16_SIZE] = GF16_INV_TABLE;
static const uint8_t pt[GF16_SIZE][GF16_EXPONENT] = GF16_POLYNOMIAL_DIV_TABLE;
static const uint8_t tl[GF16_SIZE][GF16_SIZE] = GF16_SHUFFLE_LOW_TABLE;
static const uint8_t th[GF16_SIZE][GF16_SIZE] = GF16_SHUFFLE_HIGH_TABLE;

inline void
ffadd16_region_sse2(uint8_t* region1, const uint8_t* region2, int length)
{
	ffxor_region_sse2(region1, region2, length);
}

void
ffmadd16_region_c_sse2(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	register __m128i reg1, reg2, ri[4], sp[4], mi[4];
	const uint8_t *p = pt[constant];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_sse2(region1, region2, length);
		return;
	}
	
	mi[0] = _mm_set1_epi8(0x11);
	mi[1] = _mm_set1_epi8(0x22);
	mi[2] = _mm_set1_epi8(0x44);
	mi[3] = _mm_set1_epi8(0x88);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);
	sp[2] = _mm_set1_epi16(p[2]);
	sp[3] = _mm_set1_epi16(p[3]);

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
		reg2 = _mm_load_si128((void *)region2);
		reg1 = _mm_load_si128((void *)region1);
		ri[0] = _mm_and_si128(reg2, mi[0]);
		ri[1] = _mm_and_si128(reg2, mi[1]);
		ri[2] = _mm_and_si128(reg2, mi[2]);
		ri[3] = _mm_and_si128(reg2, mi[3]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[2] = _mm_srli_epi16(ri[2], 2);
		ri[3] = _mm_srli_epi16(ri[3], 3);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[2] = _mm_mullo_epi16(ri[2], sp[2]);
		ri[3] = _mm_mullo_epi16(ri[3], sp[3]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[2] = _mm_xor_si128(ri[2], ri[3]);
		ri[0] = _mm_xor_si128(ri[0], ri[2]);
		ri[0] = _mm_xor_si128(ri[0], reg1);
		_mm_store_si128((void *)region1, ri[0]);
	}

	ffmadd16_region_c_gpr(region1, region2, constant, length);
}

void
ffmul16_region_c_sse2(uint8_t *region, uint8_t constant, int length)
{
	register __m128i reg, ri[4], sp[4], mi[4];
	const uint8_t *p = pt[constant];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	mi[0] = _mm_set1_epi8(0x11);
	mi[1] = _mm_set1_epi8(0x22);
	mi[2] = _mm_set1_epi8(0x44);
	mi[3] = _mm_set1_epi8(0x88);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);
	sp[2] = _mm_set1_epi16(p[2]);
	sp[3] = _mm_set1_epi16(p[3]);

	for (; length & 0xfffffff0; region+=16, length-=16) {
		reg = _mm_load_si128((void *)region);
		ri[0] = _mm_and_si128(reg, mi[0]);
		ri[1] = _mm_and_si128(reg, mi[1]);
		ri[2] = _mm_and_si128(reg, mi[2]);
		ri[3] = _mm_and_si128(reg, mi[3]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[2] = _mm_srli_epi16(ri[2], 2);
		ri[3] = _mm_srli_epi16(ri[3], 3);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[2] = _mm_mullo_epi16(ri[2], sp[2]);
		ri[3] = _mm_mullo_epi16(ri[3], sp[3]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[2] = _mm_xor_si128(ri[2], ri[3]);
		ri[0] = _mm_xor_si128(ri[0], ri[2]);
		_mm_store_si128((void *)region, ri[0]);
	}
	
	ffmul16_region_c_gpr(region, constant, length);
}

