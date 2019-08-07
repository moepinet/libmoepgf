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

#include "gf16.h"
#include "xor.h"

#if MOEPGF16_POLYNOMIAL == 19
#include "gf16tables19.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t pt[MOEPGF16_SIZE][MOEPGF16_EXPONENT] = MOEPGF16_POLYNOMIAL_DIV_TABLE;

void
maddrc16_imul_avx512(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, size_t length)
{
	uint8_t *end;
	register __m512i reg1, reg2, ri[4], sp[4], mi[4];
	const uint8_t *p = pt[constant];

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_avx512(region1, region2, length);
		return;
	}

	mi[0] = _mm512_set1_epi8(0x11);
	mi[1] = _mm512_set1_epi8(0x22);
	mi[2] = _mm512_set1_epi8(0x44);
	mi[3] = _mm512_set1_epi8(0x88);
	sp[0] = _mm512_set1_epi32(p[0]);
	sp[1] = _mm512_set1_epi32(p[1]);
	sp[2] = _mm512_set1_epi32(p[2]);
	sp[3] = _mm512_set1_epi32(p[3]);

	for (end=region1+length; region1<end; region1+=64, region2+=64) {
		reg2 = _mm512_load_si512((void *)region2);
		reg1 = _mm512_load_si512((void *)region1);
		ri[0] = _mm512_and_si512(reg2, mi[0]);
		ri[1] = _mm512_and_si512(reg2, mi[1]);
		ri[2] = _mm512_and_si512(reg2, mi[2]);
		ri[3] = _mm512_and_si512(reg2, mi[3]);
		ri[1] = _mm512_srli_epi32(ri[1], 1);
		ri[2] = _mm512_srli_epi32(ri[2], 2);
		ri[3] = _mm512_srli_epi32(ri[3], 3);
		ri[0] = _mm512_mullo_epi32(ri[0], sp[0]);
		ri[1] = _mm512_mullo_epi32(ri[1], sp[1]);
		ri[2] = _mm512_mullo_epi32(ri[2], sp[2]);
		ri[3] = _mm512_mullo_epi32(ri[3], sp[3]);
		ri[0] = _mm512_xor_si512(ri[0], ri[1]);
		ri[2] = _mm512_xor_si512(ri[2], ri[3]);
		ri[0] = _mm512_xor_si512(ri[0], ri[2]);
		ri[0] = _mm512_xor_si512(ri[0], reg1);
		_mm512_store_si512((void *)region1, ri[0]);
	}
}

void
mulrc16_imul_avx512(uint8_t *region, uint8_t constant, size_t length)
{
	uint8_t *end;
	register __m512i reg, ri[4], sp[4], mi[4];
	const uint8_t *p = pt[constant];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	mi[0] = _mm512_set1_epi8(0x11);
	mi[1] = _mm512_set1_epi8(0x22);
	mi[2] = _mm512_set1_epi8(0x44);
	mi[3] = _mm512_set1_epi8(0x88);
	sp[0] = _mm512_set1_epi32(p[0]);
	sp[1] = _mm512_set1_epi32(p[1]);
	sp[2] = _mm512_set1_epi32(p[2]);
	sp[3] = _mm512_set1_epi32(p[3]);

	for (end=region+length; region<end; region+=64) {
		reg = _mm512_load_si512((void *)region);
		ri[0] = _mm512_and_si512(reg, mi[0]);
		ri[1] = _mm512_and_si512(reg, mi[1]);
		ri[2] = _mm512_and_si512(reg, mi[2]);
		ri[3] = _mm512_and_si512(reg, mi[3]);
		ri[1] = _mm512_srli_epi32(ri[1], 1);
		ri[2] = _mm512_srli_epi32(ri[2], 2);
		ri[3] = _mm512_srli_epi32(ri[3], 3);
		ri[0] = _mm512_mullo_epi32(ri[0], sp[0]);
		ri[1] = _mm512_mullo_epi32(ri[1], sp[1]);
		ri[2] = _mm512_mullo_epi32(ri[2], sp[2]);
		ri[3] = _mm512_mullo_epi32(ri[3], sp[3]);
		ri[0] = _mm512_xor_si512(ri[0], ri[1]);
		ri[2] = _mm512_xor_si512(ri[2], ri[3]);
		ri[0] = _mm512_xor_si512(ri[0], ri[2]);
		_mm512_store_si512((void *)region, ri[0]);
	}
}

