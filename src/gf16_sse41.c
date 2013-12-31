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

#include <smmintrin.h>

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

void
ffmadd16_region_c_sse41(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	register __m128i in1, in2, out, t1, t2, m1, m2, l, h;

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_sse2(region1, region2, length);
		return;
	}
	
	t1 = _mm_loadu_si128((void *)tl[constant]);
	t2 = _mm_slli_epi64(t1, 4);
	m1 = _mm_set1_epi8(0x0f);
	m2 = _mm_set1_epi8(0xf0);

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
		in2 = _mm_load_si128((void *)region2);
		in1 = _mm_load_si128((void *)region1);
		l = _mm_and_si128(in2, m1);
		l = _mm_shuffle_epi8(t1, l);
		h = _mm_and_si128(in2, m2);
		h = _mm_srli_epi64(h, 4);
		h = _mm_shuffle_epi8(t2, h);
		out = _mm_xor_si128(h,l);
		out = _mm_xor_si128(out, in1);
		_mm_store_si128((void *)region1, out);
	}
	
	ffmadd16_region_c_gpr(region1, region2, constant, length);
}

void
ffmul16_region_c_sse41(uint8_t *region, uint8_t constant, int length)
{
	register __m128i in, out, t1, t2, m1, m2, l, h;

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	t1 = _mm_loadu_si128((void *)tl[constant]);
	t2 = _mm_slli_epi64(t1, 4);
	m1 = _mm_set1_epi8(0x0f);
	m2 = _mm_set1_epi8(0xf0);

	for (; length & 0xfffffff0; region+=16, length-=16) {
		in = _mm_load_si128((void *)region);
		l = _mm_and_si128(in, m1);
		l = _mm_shuffle_epi8(t1, l);
		h = _mm_and_si128(in, m2);
		h = _mm_srli_epi64(h, 4);
		h = _mm_shuffle_epi8(t2, h);
		out = _mm_xor_si128(h, l);
		_mm_store_si128((void *)region, out);
	}
	
	ffmul16_region_c_gpr(region, constant, length);
}

