/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013 	Alexander Kurtz <alexander@kurtz.be>
 * 
 * moep80211gf is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License * along
 * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <emmintrin.h>

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

inline void
ffadd4_region_sse2(uint8_t* region1, const uint8_t* region2, int length)
{
	ffxor_region_sse2(region1, region2, length);
}

inline void
ffdiv4_region_c_sse2(uint8_t* region, uint8_t constant, int length)
{
	ffmul4_region_c_sse2(region, inverses[constant], length);
}

void
ffmadd4_region_c_sse2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	register __m128i reg1, reg2, ri[2], sp[2], mi[2];
	const uint8_t *p = pt[constant];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_sse2(region1, region2, length);
		return;
	}

	mi[0] = _mm_set1_epi8(0x55);
	mi[1] = _mm_set1_epi8(0xaa);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
		reg2 = _mm_load_si128((void *)region2);
		reg1 = _mm_load_si128((void *)region1);
		ri[0] = _mm_and_si128(reg2, mi[0]);
		ri[1] = _mm_and_si128(reg2, mi[1]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[0] = _mm_xor_si128(ri[0], reg1);
		_mm_store_si128((void *)region1, ri[0]);
	}
	
	ffmadd4_region_c_gpr(region1, region2, constant, length);
}

void
ffmul4_region_c_sse2(uint8_t *region, uint8_t constant, int length)
{
	register __m128i reg, ri[2], sp[2], mi[2];
	const uint8_t *p = pt[constant];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	mi[0] = _mm_set1_epi8(0x55);
	mi[1] = _mm_set1_epi8(0xaa);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);

	for (; length & 0xfffffff0; region+=16, length-=16) {
		reg = _mm_load_si128((void *)region);
		ri[0] = _mm_and_si128(reg, mi[0]);
		ri[1] = _mm_and_si128(reg, mi[1]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		_mm_store_si128((void *)region, ri[0]);
	}
	
	ffmul4_region_c_gpr(region, constant, length);
}

