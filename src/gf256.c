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
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif
#ifdef __AVX2__
#include <immintrin.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf256.h"
#include "gf.h"

#if GF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prim polynomial or tables not available."
#endif

static const uint8_t inverses[GF256_SIZE] = GF256_INV_TABLE;
static const uint8_t pt[GF256_SIZE][GF256_EXPONENT] = GF256_POLYNOMIAL_DIV_TABLE;
static const uint8_t tl[GF256_SIZE][16] = GF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[GF256_SIZE][16] = GF256_SHUFFLE_HIGH_TABLE;

inline uint8_t
ffinv256(uint8_t element)
{
	return inverses[element];
}

inline uint8_t
ffadd256(uint8_t summand1, uint8_t summand2)
{
	return summand1 ^ summand2;
}

inline uint8_t
ffdiv256(uint8_t dividend, uint8_t divisor)
{
	ffmul256_region_c(&dividend, inverses[divisor], 1);
	return dividend;
}

inline uint8_t
ffmul256(uint8_t factor1, uint8_t factor2)
{
	ffmul256_region_c(&factor1, factor2, 1);
	return factor1;
}

inline void
ffadd256_region(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region(region1, region2, length);
}

inline void
ffdiv256_region_c(uint8_t *region, uint8_t constant, int length)
{
	ffmul256_region_c(region, inverses[constant], length);
}

void
ffmadd256_region_c_slow(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region(region1, region2, length);
		return ;
	}

	for (; length; region1++, region2++, length--) {
		r[0] = (*region2 &   1) ? p[0] : 0;
		r[1] = (*region2 &   2) ? p[1] : 0;
		r[2] = (*region2 &   4) ? p[2] : 0;
		r[3] = (*region2 &   8) ? p[3] : 0;
		r[4] = (*region2 &  16) ? p[4] : 0;
		r[5] = (*region2 &  32) ? p[5] : 0;
		r[6] = (*region2 &  64) ? p[6] : 0;
		r[7] = (*region2 & 128) ? p[7] : 0;
		*region1 ^= r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmadd256_region_c(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];
	uint64_t r64[8];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region(region1, region2, length);
		return;
	}

#if defined __AVX2__
	register __m256i t1, t2, m1, m2, in1, in2, out, l, h;
	t1 = __builtin_ia32_vbroadcastsi256((void *)tl[constant]);
	t2 = __builtin_ia32_vbroadcastsi256((void *)th[constant]);
	m1 = _mm256_set1_epi8(0x0f);
	m2 = _mm256_set1_epi8(0xf0);

	for (; length & 0xffffffe0; region1+=32, region2+=32, length-=32) {
		in2 = _mm256_load_si256((void *)region2);
		in1 = _mm256_load_si256((void *)region1);
		l = _mm256_and_si256(in2, m1);
		l = _mm256_shuffle_epi8(t1, l);
		h = _mm256_and_si256(in2, m2);
		h = _mm256_srli_epi64(h, 4);
		h = _mm256_shuffle_epi8(t2, h);
		out = _mm256_xor_si256(h,l);
		out = _mm256_xor_si256(out, in1);
		_mm256_store_si256((void *)region1, out);
	}
#elif defined __SSE4_1__
	register __m128i t1, t2, m1, m2, in1, in2, out, l, h;
	t1 = _mm_loadu_si128((void *)tl[constant]);
	t2 = _mm_loadu_si128((void *)th[constant]);
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
#elif defined __SSE2__
	register __m128i ri[8], mi[8], sp[8], reg1, reg2;

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

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
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
#endif

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region2 & 0x0101010101010101)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region2 & 0x0202020202020202)>>1)*p[1];
		r64[2] = ((*(uint64_t *)region2 & 0x0404040404040404)>>2)*p[2];
		r64[3] = ((*(uint64_t *)region2 & 0x0808080808080808)>>3)*p[3];
		r64[4] = ((*(uint64_t *)region2 & 0x1010101010101010)>>4)*p[4];
		r64[5] = ((*(uint64_t *)region2 & 0x2020202020202020)>>5)*p[5];
		r64[6] = ((*(uint64_t *)region2 & 0x4040404040404040)>>6)*p[6];
		r64[7] = ((*(uint64_t *)region2 & 0x8080808080808080)>>7)*p[7];
		*(uint64_t *)region1 ^= r64[0]^r64[1]^r64[2]^r64[3]
					^r64[4]^r64[5]^r64[6]^r64[7];
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x01) >> 0) * p[0];
		r[1] = ((*region2 & 0x02) >> 1) * p[1];
		r[2] = ((*region2 & 0x04) >> 2) * p[2];
		r[3] = ((*region2 & 0x08) >> 3) * p[3];
		r[4] = ((*region2 & 0x10) >> 4) * p[4];
		r[5] = ((*region2 & 0x20) >> 5) * p[5];
		r[6] = ((*region2 & 0x40) >> 6) * p[6];
		r[7] = ((*region2 & 0x80) >> 7) * p[7];
		*region1 ^= r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void ffmul256_region_c_slow(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	for (; length; region++, length--) {
		r[0] = (*region &   1) ? p[0] : 0;
		r[1] = (*region &   2) ? p[1] : 0;
		r[2] = (*region &   4) ? p[2] : 0;
		r[3] = (*region &   8) ? p[3] : 0;
		r[4] = (*region &  16) ? p[4] : 0;
		r[5] = (*region &  32) ? p[5] : 0;
		r[6] = (*region &  64) ? p[6] : 0;
		r[7] = (*region & 128) ? p[7] : 0;
		*region = r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}

void
ffmul256_region_c(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[8];
	uint64_t r64[8];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if(constant == 1)
		return;

#if defined __AVX2__
	register __m256i t1, t2, m1, m2, in, out, l, h;
	t1 = __builtin_ia32_vbroadcastsi256((void *)tl[constant]);
	t2 = __builtin_ia32_vbroadcastsi256((void *)th[constant]);
	m1 = _mm256_set1_epi8(0x0f);
	m2 = _mm256_set1_epi8(0xf0);

	for (; length & 0xffffffe0; region+=32, length-=32) {
		in = _mm256_load_si256((void *)region);
		l = _mm256_and_si256(in, m1);
		l = _mm256_shuffle_epi8(t1, l);
		h = _mm256_and_si256(in, m2);
		h = _mm256_srli_epi64(h, 4);
		h = _mm256_shuffle_epi8(t2, h);
		out = _mm256_xor_si256(h,l);
		_mm256_store_si256((void *)region, out);
	}
#elif defined __SSE4_1__
	register __m128i t1, t2, m1, m2, in, out, l, h;
	t1 = _mm_loadu_si128((void *)tl[constant]);
	t2 = _mm_loadu_si128((void *)th[constant]);
	m1 = _mm_set1_epi8(0x0f);
	m2 = _mm_set1_epi8(0xf0);

	for (; length & 0xfffffff0; region+=16, length-=16) {
		in = _mm_load_si128((void *)region);
		l = _mm_and_si128(in, m1);
		l = _mm_shuffle_epi8(t1, l);
		h = _mm_and_si128(in, m2);
		h = _mm_srli_epi64(h, 4);
		h = _mm_shuffle_epi8(t2, h);
		out = _mm_xor_si128(h,l);
		_mm_store_si128((void *)region, out);
	}
#elif defined __SSE2__
	register __m128i ri[8], mi[8], sp[8], reg;
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

	for (; length & 0xfffffff0; region+=16, length-=16) {
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
#endif

	for (; length & 0xfffffff8; region+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region & 0x0101010101010101)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region & 0x0202020202020202)>>1)*p[1];
		r64[2] = ((*(uint64_t *)region & 0x0404040404040404)>>2)*p[2];
		r64[3] = ((*(uint64_t *)region & 0x0808080808080808)>>3)*p[3];
		r64[4] = ((*(uint64_t *)region & 0x1010101010101010)>>4)*p[4];
		r64[5] = ((*(uint64_t *)region & 0x2020202020202020)>>5)*p[5];
		r64[6] = ((*(uint64_t *)region & 0x4040404040404040)>>6)*p[6];
		r64[7] = ((*(uint64_t *)region & 0x8080808080808080)>>7)*p[7];
		*(uint64_t *)region =
			r64[0]^r64[1]^r64[2]^r64[3]^r64[4]^r64[5]^r64[6]^r64[7];
	}

	for (; length; region++, length--) {
		r[0] = ((*region & 0x01) >> 0) * p[0];
		r[1] = ((*region & 0x02) >> 1) * p[1];
		r[2] = ((*region & 0x04) >> 2) * p[2];
		r[3] = ((*region & 0x08) >> 3) * p[3];
		r[4] = ((*region & 0x10) >> 4) * p[4];
		r[5] = ((*region & 0x20) >> 5) * p[5];
		r[6] = ((*region & 0x40) >> 6) * p[6];
		r[7] = ((*region & 0x80) >> 7) * p[7];
		*region = r[0]^r[1]^r[2]^r[3]^r[4]^r[5]^r[6]^r[7];
	}
}
