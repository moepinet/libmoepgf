/*
 * This file is part of moep80211gf.
 *
 * Copyright (C) 2014,2019   Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2016        Nicolas Appel <n.appel@tum.de>
 * Copyright (C) 2014        Maximilian Riemensberger <riemensberger@tum.de>
 *
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>
 *
 */

#include <immintrin.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <moepgf/moepgf.h>

#include "gf256.h"
#include "xor.h"

#if MOEPGF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t tl[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_HIGH_TABLE;

void
maddrc256_shuffle_avx512(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, size_t length)
{
	uint8_t *end;
	register __m512i t1, t2, m1, m2, in1, in2, out, l, h;
	register __m128i bc;

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_avx512(region1, region2, length);
		return;
	}

	bc = _mm_load_si128((void *)tl[constant]);
	t1 = _mm512_broadcast_i32x4 (bc);
	bc = _mm_load_si128((void *)th[constant]);
	t2 = _mm512_broadcast_i32x4 (bc);
	m1 = _mm512_set1_epi8(0x0f);
	m2 = _mm512_set1_epi8(0xf0);

	for (end=region1+length; region1<end; region1+=64, region2+=64) {
		in2 = _mm512_load_si512((void *)region2);
		in1 = _mm512_load_si512((void *)region1);
		l = _mm512_and_si512(in2, m1);
		l = _mm512_shuffle_epi8(t1, l);
		h = _mm512_and_si512(in2, m2);
		h = _mm512_srli_epi64(h, 4);
		h = _mm512_shuffle_epi8(t2, h);
		out = _mm512_xor_si512(h, l);
		out = _mm512_xor_si512(out, in1);
		_mm512_store_si512((void *)region1, out);
	}
}


void
mulrc256_shuffle_avx512(uint8_t *region, uint8_t constant, size_t length)
{
	uint8_t *end;
	register __m512i t1, t2, m1, m2, in, out, l, h;
	register __m128i bc;

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	bc = _mm_load_si128((void *)tl[constant]);
	t1 = _mm512_broadcast_i32x4(bc);
	bc = _mm_load_si128((void *)th[constant]);
	t2 = _mm512_broadcast_i32x4(bc);
	m1 = _mm512_set1_epi8(0x0f);
	m2 = _mm512_set1_epi8(0xf0);

	for (end=region+length; region<end; region+=64) {
		in = _mm512_load_si512((void *)region);
		l = _mm512_and_si512(in, m1);
		l = _mm512_shuffle_epi8(t1, l);
		h = _mm512_and_si512(in, m2);
		h = _mm512_srli_epi64(h, 4);
		h = _mm512_shuffle_epi8(t2, h);
		out = _mm512_xor_si512(h, l);
		_mm512_store_si512((void *)region, out);
	}
}

