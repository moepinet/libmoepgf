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
#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf.h"
#include "gf4.h"

#if GF16_POLYNOMIAL == 19
#include "gf4tables7.h"
#else
#error "Invalid prim polynomial or tables not available."
#endif

static const uint8_t inverses[GF4_SIZE] = GF4_INV_TABLE;
static const uint8_t pt[GF4_SIZE][GF16_EXPONENT] = GF4_POLYNOMIAL_DIV_TABLE;
static const uint8_t mul[GF4_SIZE][GF4_SIZE] = GF4_MUL_TABLE;

inline uint8_t
ffinv4(uint8_t element)
{
	return inverses[element];
}

inline uint8_t
ffadd4(uint8_t summand1, uint8_t summand2)
{
	return summand1 ^ summand2;
}

inline uint8_t
ffdiv4(uint8_t dividend, uint8_t divisor)
{
	ffmul4_region_c(&dividend, inverses[divisor], 1);
	return dividend;
}

inline uint8_t
ffmul4(uint8_t factor1, uint8_t factor2)
{
	ffmul4_region_c(&factor1, factor2, 1);
	return factor1;
}

inline void
ffadd4_region(uint8_t* region1, const uint8_t* region2, int length)
{
	ffxor_region(region1, region2, length);
}

inline void
ffdiv4_region_c(uint8_t* region, uint8_t constant, int length)
{
	ffmul4_region_c(region, inverses[constant], length);
}

void
ffmadd4_region_c_slow(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region(region1, region2, length);
		return;
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x55) >> 0) * p[0];
		r[1] = ((*region2 & 0xaa) >> 1) * p[1];
		*region1 ^= r[0] ^ r[1];
	}
}

void
ffmadd4_region_c(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];
	uint64_t r64[4];

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region(region1, region2, length);
		return;
	}

#if defined __ARM_NEON__
	register uint8x8_t reg1, reg2, ri[2], sp[2], mi[2];
	mi[0] = vdup_n_u8(0x55);
	mi[1] = vdup_n_u8(0xaa);
	sp[0] = vdup_n_u8(p[0]);
	sp[1] = vdup_n_u8(p[1]);

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		reg2 = vld1_u8((void *)region2);
		reg1 = vld1_u8((void *)region1);
		ri[0] = vand_u8(reg2, mi[0]);
		ri[1] = vand_u8(reg2, mi[1]);
		ri[1] = vshr_n_u8(ri[1], 1);
		ri[0] = vmul_u8(ri[0], sp[0]);
		ri[1] = vmul_u8(ri[1], sp[1]);
		ri[0] = veor_u8(ri[0], ri[1]);
		ri[0] = veor_u8(ri[0], reg1);
		vst1_u8((void *)region1, ri[0]);
	}
#elif defined __AVX2__
	register __m256i reg1, reg2, ri[2], sp[2], mi[2];
	mi[0] = _mm256_set1_epi8(0x55);
	mi[1] = _mm256_set1_epi8(0xaa);
	sp[0] = _mm256_set1_epi16(p[0]);
	sp[1] = _mm256_set1_epi16(p[1]);

	for (; length & 0xffffffe0; region1+=32, region2+=32, length-=32) {
		reg2 = _mm256_load_si256((void *)region2);
		reg1 = _mm256_load_si256((void *)region1);
		ri[0] = _mm256_and_si256(reg2, mi[0]);
		ri[1] = _mm256_and_si256(reg2, mi[1]);
		ri[1] = _mm256_srli_epi16(ri[1], 1);
		ri[0] = _mm256_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm256_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm256_xor_si256(ri[0], ri[1]);
		ri[0] = _mm256_xor_si256(ri[0], reg1);
		_mm256_store_si256((void *)region1, ri[0]);
	}
#elif defined __SSE2__
	register __m128i reg1, reg2, ri[2], sp[2], mi[2];
	mi[0] = _mm_set1_epi8(0x55);
	mi[1] = _mm_set1_epi8(0xaa);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);

	for (; length & 0xfffffff0; region1+=16, region2+=16, length-=16) {
		reg2 = _mm_load_si128((void *)region2);
		reg1 = _mm_load_si128((void *)region1);
		ri[0] = _mm_and_si128(reg2, mi[0]);
		ri[1] = _mm_and_si128(reg2, mi[1]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		ri[0] = _mm_xor_si128(ri[0], reg1);
		_mm_store_si128((void *)region1, ri[0]);
	}
#endif

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region2 & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region2 & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region1) ^= r64[0] ^ r64[1];
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x55) >> 0) * p[0];
		r[1] = ((*region2 & 0xaa) >> 1) * p[1];
		*region1 ^= r[0] ^ r[1];
	}
}

void
ffmul4_region_c_slow(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	for (; length; region++, length--) {
		r[0] = ((*region & 0x55) >> 0) * p[0];
		r[1] = ((*region & 0xaa) >> 1) * p[1];
		*region = r[0] ^ r[1];
	}
}

void
ffmul4_region_c(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[2];
	uint64_t r64[2];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

#if defined __ARM_NEON__
	register uint8x8_t reg, ri[2], sp[2], mi[2];
	mi[0] = vdup_n_u8(0x55);
	mi[1] = vdup_n_u8(0xaa);
	sp[0] = vdup_n_u8(p[0]);
	sp[1] = vdup_n_u8(p[1]);

	for (; length & 0xfffffff8; region+=8, length-=8) {
		reg = vld1_u8((void *)region);
		ri[0] = vand_u8(reg, mi[0]);
		ri[1] = vand_u8(reg, mi[1]);
		ri[1] = vshr_n_u8(ri[1], 1);
		ri[0] = vmul_u8(ri[0], sp[0]);
		ri[1] = vmul_u8(ri[1], sp[1]);
		ri[0] = veor_u8(ri[0], ri[1]);
		vst1_u8((void *)region, ri[0]);
	}
#elif defined __AVX2__
	register __m256i reg, ri[2], sp[2], mi[2];
	mi[0] = _mm256_set1_epi8(0x55);
	mi[1] = _mm256_set1_epi8(0xaa);
	sp[0] = _mm256_set1_epi16(p[0]);
	sp[1] = _mm256_set1_epi16(p[1]);

	for (; length & 0xffffffe0; region+=32, length-=32) {
		reg = _mm256_load_si256((void *)region);
		ri[0] = _mm256_and_si256(reg, mi[0]);
		ri[1] = _mm256_and_si256(reg, mi[1]);
		ri[1] = _mm256_srli_epi16(ri[1], 1);
		ri[0] = _mm256_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm256_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm256_xor_si256(ri[0], ri[1]);
		_mm256_store_si256((void *)region, ri[0]);
	}
#elif defined __SSE2__
	register __m128i reg, ri[2], sp[2], mi[2];
	mi[0] = _mm_set1_epi8(0x55);
	mi[1] = _mm_set1_epi8(0xaa);
	sp[0] = _mm_set1_epi16(p[0]);
	sp[1] = _mm_set1_epi16(p[1]);

	for (; length & 0xfffffff0; region+=16, length-=16) {
		reg = _mm_load_si128((void *)region);
		ri[0] = _mm_and_si128(reg, mi[0]);
		ri[1] = _mm_and_si128(reg, mi[1]);
		ri[1] = _mm_srli_epi16(ri[1], 1);
		ri[0] = _mm_mullo_epi16(ri[0], sp[0]);
		ri[1] = _mm_mullo_epi16(ri[1], sp[1]);
		ri[0] = _mm_xor_si128(ri[0], ri[1]);
		_mm_store_si128((void *)region, ri[0]);
	}
#endif

	for (; length & 0xfffffff8; region+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region) = r64[0] ^ r64[1];
	}

	for (; length; region++, length--) {
		r[0] = ((*region & 0x55) >> 0) * p[0];
		r[1] = ((*region & 0xaa) >> 1) * p[1];
		*region = r[0] ^ r[1];
	}
}
