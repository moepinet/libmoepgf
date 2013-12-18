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

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gf.h"
#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"

static void
cpuid(unsigned int *eax, unsigned int *ebx, unsigned int *ecx,
							unsigned int *edx)
{
	asm volatile("cpuid"
		: "=a" (*eax),
		  "=b" (*ebx),
		  "=c" (*ecx),
		  "=d" (*edx)
		: "0" (*eax), "2" (*ecx));
}

uint32_t
check_available_simd_extensions()
{
	unsigned int eax, ebx, ecx, edx;
	uint32_t ret = 0;

	ret |= HWCAPS_SIMD_NONE;
	
	eax = 1;
	ebx = ecx = edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (edx & (1 << 26))
		ret |= HWCAPS_SIMD_SSE2;
	if (ecx & (1 << 19))
		ret |= HWCAPS_SIMD_SSE41;

	eax = 7;
	ebx = ecx = edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (ebx & (1 << 5))
		ret |= HWCAPS_SIMD_AVX2;

	return ret;
}

int
get_galois_field(struct galois_field *gf, enum GF_TYPE type, uint32_t fset)
{
	int ret = 0;

	if (fset == 0)
		fset = check_available_simd_extensions();
		
	switch (type) {
		gf->simd = HWCAPS_SIMD_NONE;
	case GF2:
		strcpy(gf->name, "GF2");
		gf->type		= GF2;
		gf->polynomial		= GF2_POLYNOMIAL;
		gf->exponent		= GF2_EXPONENT;
		gf->size		= GF2_SIZE;
		gf->mask		= GF2_MASK;
		gf->fmulrctest		= ffmul2_region_c;
		gf->fmaddrctest		= ffmadd2_region_c_gpr;
		gf->finv		= ffinv2;
		gf->faddr		= ffxor_region_gpr;
		gf->fmulrc		= ffmul2_region_c;
		gf->fmaddrc		= ffmadd2_region_c_gpr;
		break;

	case GF4:
		strcpy(gf->name, "GF4");
		gf->type		= GF4;
		gf->polynomial		= GF4_POLYNOMIAL;
		gf->exponent		= GF4_EXPONENT;
		gf->size		= GF4_SIZE;
		gf->mask		= GF4_MASK;
		gf->fmulrctest		= ffmul4_region_c_slow;
		gf->fmaddrctest		= ffmadd4_region_c_slow;
		gf->finv		= ffinv4;
		gf->faddr		= ffxor_region_gpr;
		gf->fmulrc		= ffmul4_region_c_gpr;
		gf->fmaddrc		= ffmadd4_region_c_gpr;
		break;
	
	case GF16:
		strcpy(gf->name, "GF16");
		gf->type		= GF16;
		gf->polynomial		= GF16_POLYNOMIAL;
		gf->exponent		= GF16_EXPONENT;
		gf->size		= GF16_SIZE;
		gf->mask		= GF16_MASK;
		gf->fmulrctest		= ffmul16_region_c_slow;
		gf->fmaddrctest		= ffmadd16_region_c_slow;
		gf->finv		= ffinv16;
		gf->faddr		= ffxor_region_gpr;
		gf->fmulrc		= ffmul16_region_c_gpr;
		gf->fmaddrc		= ffmadd16_region_c_gpr;
		break;
	
	case GF256:
		strcpy(gf->name, "GF256");
		gf->type		= GF256;
		gf->polynomial		= GF256_POLYNOMIAL;
		gf->exponent		= GF256_EXPONENT;
		gf->size		= GF256_SIZE;
		gf->mask		= GF256_MASK;
		gf->fmulrctest		= ffmul256_region_c_slow;
		gf->fmaddrctest		= ffmadd256_region_c_slow;
		gf->finv		= ffinv256;
		gf->faddr		= ffxor_region_gpr;
		gf->fmulrc		= ffmul256_region_c_gpr;
		gf->fmaddrc		= ffmadd256_region_c_gpr;
		break;
	}

#ifdef __x86_64__
	if (fset & HWCAPS_SIMD_AVX2) {
		gf->simd = HWCAPS_SIMD_AVX2;
		switch (type) {
		case GF2:
			gf->faddr		= ffxor_region_avx2;
			gf->fmulrc		= ffmul2_region_c;
			gf->fmaddrc		= ffmadd2_region_c_avx2;
			break;

		case GF4:
			gf->faddr		= ffxor_region_avx2;
			gf->fmulrc		= ffmul4_region_c_avx2;
			gf->fmaddrc		= ffmadd4_region_c_avx2;
			break;
		
		case GF16:
			gf->faddr		= ffxor_region_avx2;
			gf->fmulrc		= ffmul16_region_c_avx2;
			gf->fmaddrc		= ffmadd16_region_c_avx2;
			break;
		
		case GF256:
			gf->faddr		= ffxor_region_avx2;
			gf->fmulrc		= ffmul256_region_c_avx2;
			gf->fmaddrc		= ffmadd256_region_c_avx2;
			break;
		}
	}
	else if (fset & HWCAPS_SIMD_SSE41) {
		gf->simd = HWCAPS_SIMD_SSE41;
		switch (type) {
		case GF2:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul2_region_c;
			gf->fmaddrc		= ffmadd2_region_c_sse2;
			break;

		case GF4:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul4_region_c_sse2;
			gf->fmaddrc		= ffmadd4_region_c_sse2;
			break;
		
		case GF16:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul16_region_c_sse41;
			gf->fmaddrc		= ffmadd16_region_c_sse41;
			break;
		
		case GF256:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul256_region_c_sse41;
			gf->fmaddrc		= ffmadd256_region_c_sse41;
			break;
		}

	}
	else if (fset & HWCAPS_SIMD_SSE2) {
		gf->simd = HWCAPS_SIMD_SSE2;
		switch (type) {
		case GF2:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul2_region_c;
			gf->fmaddrc		= ffmadd2_region_c_sse2;
			break;

		case GF4:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul4_region_c_sse2;
			gf->fmaddrc		= ffmadd4_region_c_sse2;
			break;
		
		case GF16:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul16_region_c_sse2;
			gf->fmaddrc		= ffmadd16_region_c_sse2;
			break;
		
		case GF256:
			gf->faddr		= ffxor_region_sse2;
			gf->fmulrc		= ffmul256_region_c_sse2;
			gf->fmaddrc		= ffmadd256_region_c_sse2;
			break;
		}
	}
#endif

#ifdef __arm__
	if (fset & HWCAPS_SIMD_NEON) {
		gf->simd = HWCAPS_SIMD_NEON;
		switch (type) {
		case GF2:
			gf->faddr		= ffxor_region_neon;
			gf->fmulrc		= ffmul2_region_c;
			gf->fmaddrc		= ffmadd2_region_c_neon;
			break;

		case GF4:
			gf->faddr		= ffxor_region_neon;
			gf->fmulrc		= ffmul4_region_c_neon;
			gf->fmaddrc		= ffmadd4_region_c_neon;
			break;
		
		case GF16:
			gf->faddr		= ffxor_region_neon;
			gf->fmulrc		= ffmul16_region_c_neon;
			gf->fmaddrc		= ffmadd16_region_c_neon;
			break;
		
		case GF256:
			gf->faddr		= ffxor_region_neon;
			gf->fmulrc		= ffmul256_region_c_neon;
			gf->fmaddrc		= ffmadd256_region_c_neon;
			break;
		}
	}
#endif
	return ret;
}

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
ffxor_region_gpr(uint8_t *region1, const uint8_t *region2, int length)
{
	for(; length & 0xfffffff8; region1+=8, region2+=8, length-=8)
		*(uint64_t *)region1 ^= *(uint64_t *)region2;

	for(; length; region1++, region2++, length--)
		*region1 ^= *region2;
}

