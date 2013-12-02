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

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#include <string.h>
#include <features.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gf.h"
#include "gf2.h"
#include "gf16.h"
#include "gf256.h"

const struct galois_field __galois_fields[3] = {
	{
	.polynomial	= GF2_POLYNOMIAL,
	.exponent	= GF2_EXPONENT,
	.size		= GF2_SIZE,
	.mask		= GF2_MASK,

	.finv		= ffinv2,
	.fadd		= ffadd2,
	.fdiv		= ffdiv2,
	.fmul		= ffmul2,
	.faddr		= ffadd2_region,
	.fdivrc		= ffdiv2_region_c,
	.fmulrc		= ffmul2_region_c,
	.fmaddrc	= ffmadd2_region_c,
	},

	{
	.polynomial	= GF16_POLYNOMIAL,
	.exponent	= GF16_EXPONENT,
	.size		= GF16_SIZE,
	.mask		= GF16_MASK,

	.finv		= ffinv16,
	.fadd		= ffadd16,
	.fdiv		= ffdiv16,
	.fmul		= ffmul16,
	.faddr		= ffadd16_region,
	.fdivrc		= ffdiv16_region_c,
	.fmulrc		= ffmul16_region_c,
	.fmaddrc	= ffmadd16_region_c,
	},

	{
	.polynomial	= GF256_POLYNOMIAL,
	.exponent	= GF256_EXPONENT,
	.size		= GF256_SIZE,
	.mask		= GF256_MASK,

	.finv		= ffinv256,
	.fadd		= ffadd256,
	.fdiv		= ffdiv256,
	.fmul		= ffmul256,
	.faddr		= ffadd256_region,
	.fdivrc		= ffdiv256_region_c,
	.fmulrc		= ffmul256_region_c,
	.fmaddrc	= ffmadd256_region_c,
	}
};

void
ffdisplay(char *name, void *data, int length)
{
	fprintf(stderr, "%s:", name);
	while (length) {
		fprintf(stderr, " %d%d%d%d%d%d%d%d",
			*(uint8_t *)data &   1 ? 1 : 0,
			*(uint8_t *)data &   2 ? 1 : 0,
			*(uint8_t *)data &   4 ? 1 : 0,
			*(uint8_t *)data &   8 ? 1 : 0,
			*(uint8_t *)data &  16 ? 1 : 0,
			*(uint8_t *)data &  32 ? 1 : 0,
			*(uint8_t *)data &  64 ? 1 : 0,
			*(uint8_t *)data & 128 ? 1 : 0
		);
		length--;
		data++;
	}
	fprintf(stderr, "\n");
}

uint64_t
ffpow(const uint64_t base, const uint64_t previous, const int exponent,
						const uint64_t polynomial)
{
	uint64_t result = previous * 2;
	if (result >= (1 << exponent))
		result = result ^ polynomial;
	return result;
}

void
ffxor_region(uint8_t *region1, const uint8_t *region2, int length)
{
#if __GNUC_PREREQ(4,7)
#ifdef __SSE2__
	register __m128i in, out;

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
		in  = _mm_load_si128((void *)region2);
		out = _mm_load_si128((void *)region1);
		_mm_store_si128((void *)region1, out);
	}
#endif
#endif

	for(; length & 0xfffffff8; region1+=8, region2+=8, length-=8)
		*(uint64_t *)region1 ^= *(uint64_t *)region2;

	for(; length; region1++, region2++, length--)
		*region1 ^= *region2;
}

