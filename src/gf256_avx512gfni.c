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
maddrc256_gfni_avx512(uint8_t *region1, const uint8_t *region2,
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
mulrc256_gfni_avx512(uint8_t *region, uint8_t constant, size_t length)
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

