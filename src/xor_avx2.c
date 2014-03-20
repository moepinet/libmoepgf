/*
 * Copyright 2014	Stephan M. Guenther <moepi@moepi.net>
 * 			Maximilian Riemensberger <riemensberger@tum.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See COPYING for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <immintrin.h>

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"

void
xorr_avx2(uint8_t *region1, const uint8_t *region2, size_t length)
{
	uint8_t *end;
	register __m256i in, out;

	for (end=region1+length; region1<end; region1+=32, region2+=32) {
		in  = _mm256_load_si256((void *)region2);
		out = _mm256_load_si256((void *)region1);
		out = _mm256_xor_si256(in, out);
		_mm256_store_si256((void *)region1, out);
	}
}

