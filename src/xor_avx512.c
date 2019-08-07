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

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "xor.h"

void
xorr_avx512(uint8_t *region1, const uint8_t *region2, size_t length)
{
	uint8_t *end;
	register __m512i in, out;

	for (end=region1+length; region1<end; region1+=64, region2+=64) {
		in  = _mm512_load_si512((void *)region2);
		out = _mm512_load_si512((void *)region1);
		out = _mm512_xor_si512(in, out);
		_mm512_store_si512((void *)region1, out);
	}
}
