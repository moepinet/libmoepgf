/*
 * This file is part of moep80211gf.
 *
 * Copyright (C) 2014   Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014   Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013   Alexander Kurtz <alexander@kurtz.be>
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

void
maddrc256_gfni128(uint8_t *region1, const uint8_t *region2,
                    uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m128i in1, in2, out, bc;

    if (constant == 0)
        return;

    if (constant == 1) {
        xorr_sse2(region1, region2, length);
        return;
    }

    bc = _mm_set1_epi8(constant);

    for (end=region1+length; region1<end; region1+=16, region2+=16) {
        in2 = _mm_load_si128((void *)region2);
        in1 = _mm_load_si128((void *)region1);
        out = _mm_gf2p8mul_epi8(in2, bc);
        out = _mm_xor_si128(out, in1);
        _mm_store_si128((void *)region1, out);
    }
}

void
maddrc256_gfni256(uint8_t *region1, const uint8_t *region2,
                    uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m256i in1, in2, out, bc;

    if (constant == 0)
        return;

    if (constant == 1) {
        xorr_avx2(region1, region2, length);
        return;
    }

    bc = _mm256_set1_epi8(constant);

    for (end=region1+length; region1<end; region1+=32, region2+=32) {
        in2 = _mm256_load_si256((void *)region2);
        in1 = _mm256_load_si256((void *)region1);
        out = _mm256_gf2p8mul_epi8(in2, bc);
        out = _mm256_xor_si256(out, in1);
        _mm256_store_si256((void *)region1, out);
    }
}

void
maddrc256_gfni512(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m512i in1, in2, out, bc;

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_avx512(region1, region2, length);
		return;
	}

    bc = _mm512_set1_epi8(constant);

    for (end=region1+length; region1<end; region1+=64, region2+=64) {
		in2 = _mm512_load_si512((void *)region2);
		in1 = _mm512_load_si512((void *)region1);
		out = _mm512_gf2p8mul_epi8(in2, bc);
		out = _mm512_xor_si512(out, in1);
		_mm512_store_si512((void *)region1, out);
	}
}


void
mulrc256_gfni128(uint8_t *region, uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m128i in, out, bc;

    if (constant == 0) {
        memset(region, 0, length);
        return;
    }

    if (constant == 1)
        return;

    bc = _mm_set1_epi8(constant);

    for (end=region+length; region<end; region+=16) {
        in = _mm_load_si128((void *)region);
        bc = _mm_set1_epi8(constant);
        out = _mm_gf2p8mul_epi8(in, bc);
        _mm_store_si128((void *)region, out);
    }
}

void
mulrc256_gfni256(uint8_t *region, uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m256i in, out, bc;

    if (constant == 0) {
        memset(region, 0, length);
        return;
    }

    if (constant == 1)
        return;

    bc = _mm256_set1_epi8(constant);

    for (end=region+length; region<end; region+=32) {
        in = _mm256_load_si256((void *)region);
        bc = _mm256_set1_epi8(constant);
        out = _mm256_gf2p8mul_epi8(in, bc);
        _mm256_store_si256((void *)region, out);
    }
}

void
mulrc256_gfni512(uint8_t *region, uint8_t constant, size_t length)
{
    uint8_t *end;
    register __m512i in, out, bc;

    if (constant == 0) {
        memset(region, 0, length);
        return;
    }

    if (constant == 1)
        return;

    bc = _mm512_set1_epi8(constant);

    for (end=region+length; region<end; region+=64) {
        in = _mm512_load_si512((void *)region);
        bc = _mm512_set1_epi8(constant);
        out = _mm512_gf2p8mul_epi8(in, bc);
        _mm512_store_si512((void *)region, out);
    }
}

