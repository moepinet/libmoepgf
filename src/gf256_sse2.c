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

#include <emmintrin.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "moepgf.h"
#include "gf256.h"
#include "xor.h"

#if GF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t pt[GF256_SIZE][GF256_EXPONENT] = GF256_POLYNOMIAL_DIV_TABLE;

void
maddrc256_imul_sse2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	register __m128i ri[8], mi[8], sp[8], reg1, reg2;
	const uint8_t *p = pt[constant];
	
	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_sse2(region1, region2, length);
		return;
	}
	
	mi[0] = _mm_set1_epi8(0x01);
	mi[1] = _mm_set1_epi8(0x02);
	mi[2] = _mm_set1_epi8(0x04);
	mi[3] = _mm_set1_epi8(0x08);
	mi[4] = _mm_set1_epi8(0x10);
	mi[5] = _mm_set1_epi8(0x20);
	mi[6] = _mm_set1_epi8(0x40);
	mi[7] = _mm_set1_epi8(0x80);

	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);
	sp[2] = _mm_set1_epi16(p[2]);
	sp[3] = _mm_set1_epi16(p[3]);
	sp[4] = _mm_set1_epi16(p[4]);
	sp[5] = _mm_set1_epi16(p[5]);
	sp[6] = _mm_set1_epi16(p[6]);
	sp[7] = _mm_set1_epi16(p[7]);

	for (; length > 0; region1+=16, region2+=16, length-=16) {
		reg1 = _mm_load_si128((void *)region1);
		reg2 = _mm_load_si128((void *)region2);

		ri[0] = _mm_and_si128(reg2, mi[0]);
		ri[1] = _mm_and_si128(reg2, mi[1]);
		ri[2] = _mm_and_si128(reg2, mi[2]);
		ri[3] = _mm_and_si128(reg2, mi[3]);
		ri[4] = _mm_and_si128(reg2, mi[4]);
		ri[5] = _mm_and_si128(reg2, mi[5]);
		ri[6] = _mm_and_si128(reg2, mi[6]);
		ri[7] = _mm_and_si128(reg2, mi[7]);

		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[2] = _mm_srli_epi16(ri[2], 2);
		ri[3] = _mm_srli_epi16(ri[3], 3);
		ri[4] = _mm_srli_epi16(ri[4], 4);
		ri[5] = _mm_srli_epi16(ri[5], 5);
		ri[6] = _mm_srli_epi16(ri[6], 6);
		ri[7] = _mm_srli_epi16(ri[7], 7);

		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[2] = _mm_mullo_epi16(ri[2], sp[2]);
		ri[3] = _mm_mullo_epi16(ri[3], sp[3]);
		ri[4] = _mm_mullo_epi16(ri[4], sp[4]);
		ri[5] = _mm_mullo_epi16(ri[5], sp[5]);
		ri[6] = _mm_mullo_epi16(ri[6], sp[6]);
		ri[7] = _mm_mullo_epi16(ri[7], sp[7]);

		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[2] = _mm_xor_si128(ri[2], ri[3]);
		ri[4] = _mm_xor_si128(ri[4], ri[5]);
		ri[6] = _mm_xor_si128(ri[6], ri[7]);
		ri[0] = _mm_xor_si128(ri[0], ri[2]);
		ri[4] = _mm_xor_si128(ri[4], ri[6]);
		ri[0] = _mm_xor_si128(ri[0], ri[4]);
		ri[0] = _mm_xor_si128(ri[0], reg1);

		_mm_store_si128((void *)region1, ri[0]);
	}
}

void
mulrc256_imul_sse2(uint8_t *region, uint8_t constant, int length)
{
	register __m128i ri[8], mi[8], sp[8], reg;
	const uint8_t *p = pt[constant];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	mi[0] = _mm_set1_epi8(0x01);
	mi[1] = _mm_set1_epi8(0x02);
	mi[2] = _mm_set1_epi8(0x04);
	mi[3] = _mm_set1_epi8(0x08);
	mi[4] = _mm_set1_epi8(0x10);
	mi[5] = _mm_set1_epi8(0x20);
	mi[6] = _mm_set1_epi8(0x40);
	mi[7] = _mm_set1_epi8(0x80);

	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);
	sp[2] = _mm_set1_epi16(p[2]);
	sp[3] = _mm_set1_epi16(p[3]);
	sp[4] = _mm_set1_epi16(p[4]);
	sp[5] = _mm_set1_epi16(p[5]);
	sp[6] = _mm_set1_epi16(p[6]);
	sp[7] = _mm_set1_epi16(p[7]);

	for (; length > 0; region+=16, length-=16) {
		reg = _mm_load_si128((void *)region);

		ri[0] = _mm_and_si128(reg, mi[0]);
		ri[1] = _mm_and_si128(reg, mi[1]);
		ri[2] = _mm_and_si128(reg, mi[2]);
		ri[3] = _mm_and_si128(reg, mi[3]);
		ri[4] = _mm_and_si128(reg, mi[4]);
		ri[5] = _mm_and_si128(reg, mi[5]);
		ri[6] = _mm_and_si128(reg, mi[6]);
		ri[7] = _mm_and_si128(reg, mi[7]);

		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[2] = _mm_srli_epi16(ri[2], 2);
		ri[3] = _mm_srli_epi16(ri[3], 3);
		ri[4] = _mm_srli_epi16(ri[4], 4);
		ri[5] = _mm_srli_epi16(ri[5], 5);
		ri[6] = _mm_srli_epi16(ri[6], 6);
		ri[7] = _mm_srli_epi16(ri[7], 7);

		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[2] = _mm_mullo_epi16(ri[2], sp[2]);
		ri[3] = _mm_mullo_epi16(ri[3], sp[3]);
		ri[4] = _mm_mullo_epi16(ri[4], sp[4]);
		ri[5] = _mm_mullo_epi16(ri[5], sp[5]);
		ri[6] = _mm_mullo_epi16(ri[6], sp[6]);
		ri[7] = _mm_mullo_epi16(ri[7], sp[7]);

		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[2] = _mm_xor_si128(ri[2], ri[3]);
		ri[4] = _mm_xor_si128(ri[4], ri[5]);
		ri[6] = _mm_xor_si128(ri[6], ri[7]);
		ri[0] = _mm_xor_si128(ri[0], ri[2]);
		ri[4] = _mm_xor_si128(ri[4], ri[6]);
		ri[0] = _mm_xor_si128(ri[0], ri[4]);

		_mm_store_si128((void *)region, ri[0]);
	}
}

