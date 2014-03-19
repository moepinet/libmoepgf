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

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "moepgf.h"
#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"
#include "xor.h"

const char *gf_names[] =
{
	[GF_SELFTEST]		= "selftest",
	[GF_XOR_SCALAR]		= "xor_scalar",
	[GF_XOR_GPR32]		= "xor_gpr32",
	[GF_XOR_GPR64]		= "xor_gpr64",
	[GF_XOR_SSE2]		= "xor_sse2",
	[GF_XOR_AVX2]		= "xor_avx2",
	[GF_XOR_NEON_128]	= "xor_neon_128",
	[GF_LOG_TABLE]		= "log_table",
	[GF_FLAT_TABLE]		= "flat_table",
	[GF_IMUL_SCALAR]	= "imul_scalar",
	[GF_IMUL_GPR32]		= "imul_gpr32",
	[GF_IMUL_GPR64]		= "imul_gpr64",
	[GF_IMUL_SSE2]		= "imul_sse2",
	[GF_IMUL_AVX2]		= "imul_avx2",
	[GF_IMUL_NEON_64]	= "imul_neon_64",
	[GF_IMUL_NEON_128]	= "imul_neon_128",
	[GF_SHUFFLE_SSSE3]	= "shuffle_ssse3",
	[GF_SHUFFLE_AVX2]	= "shuffle_avx2",
	[GF_SHUFFLE_NEON_64]	= "shuffle_neon_64"
};

const struct {
	mulrc_t		mulrc;
	maddrc_t	maddrc;
} best_algorithms[GF_COUNT][HWCAPS_COUNT] = {
	[GF2][HWCAPS_SIMD_NONE]  = {
		.mulrc	= mulrc2,
		.maddrc	= maddrc2_gpr64
	},
#ifdef __x86_64__
	[GF2][HWCAPS_SIMD_SSE2]  = {
		.mulrc	= mulrc2,
		.maddrc	= maddrc2_sse2
	},
	[GF2][HWCAPS_SIMD_SSSE3] = {
		.mulrc	= mulrc2,
		.maddrc	= maddrc2_sse2
	},
	[GF2][HWCAPS_SIMD_AVX2]  = {
		.mulrc	= mulrc2,
		.maddrc	= maddrc2_avx2
	},
#endif
#ifdef __arm__
	[GF2][HWCAPS_SIMD_NEON]  = {
		.mulrc	= mulrc2,
		.maddrc	= maddrc2_neon
	},
#endif

	[GF4][HWCAPS_SIMD_NONE]  = {
		.mulrc	= mulrc4_imul_gpr64,
		.maddrc	= maddrc4_imul_gpr64
	},
#ifdef __x86_64__
	[GF4][HWCAPS_SIMD_SSE2]  = {
		.mulrc	= mulrc4_imul_sse2,
		.maddrc	= maddrc4_imul_sse2
	},
	[GF4][HWCAPS_SIMD_SSSE3] = {
		.mulrc	= mulrc4_shuffle_ssse3,
		.maddrc	= maddrc4_shuffle_ssse3
	},
	[GF4][HWCAPS_SIMD_AVX2]  = {
		.mulrc	= mulrc4_shuffle_avx2,
		.maddrc	= maddrc4_shuffle_avx2
	},
#endif
#ifdef __arm__
	[GF4][HWCAPS_SIMD_NEON]  = {
		.mulrc	= mulrc4_imul_neon_64,
		.maddrc	= maddrc4_imul_neon_128
	},
#endif

	[GF16][HWCAPS_SIMD_NONE]  = {
		.mulrc	= mulrc16_imul_gpr64,
		.maddrc	= maddrc16_imul_gpr64
	},
#ifdef __x86_64__
	[GF16][HWCAPS_SIMD_SSE2]  = {
		.mulrc	= mulrc16_imul_sse2,
		.maddrc	= maddrc16_imul_sse2
	},
	[GF16][HWCAPS_SIMD_SSSE3] = {
		.mulrc	= mulrc16_shuffle_ssse3,
		.maddrc	= maddrc16_shuffle_ssse3
	},
	[GF16][HWCAPS_SIMD_AVX2]  = {
		.mulrc	= mulrc16_shuffle_avx2,
		.maddrc	= maddrc16_shuffle_avx2
	},
#endif
#ifdef __arm__
	[GF16][HWCAPS_SIMD_NEON]  = {
		.mulrc	= mulrc16_shuffle_neon_64,
		.maddrc	= maddrc16_shuffle_neon_64
	},
#endif

	[GF256][HWCAPS_SIMD_NONE]  = {
		.mulrc	= mulrc256_imul_gpr64,
		.maddrc	= maddrc256_imul_gpr64
	},
#ifdef __x86_64__
	[GF256][HWCAPS_SIMD_SSE2]  = {
		.mulrc	= mulrc256_imul_sse2,
		.maddrc	= maddrc256_imul_sse2
	},
	[GF256][HWCAPS_SIMD_SSSE3] = {
		.mulrc	= mulrc256_shuffle_ssse3,
		.maddrc	= maddrc256_shuffle_ssse3
	},
	[GF256][HWCAPS_SIMD_AVX2]  = {
		.mulrc	= mulrc256_shuffle_avx2,
		.maddrc	= maddrc256_shuffle_avx2
	},
#endif
#ifdef __arm__
	[GF256][HWCAPS_SIMD_NEON]  = {
		.mulrc	= mulrc256_shuffle_neon_64,
		.maddrc	= maddrc256_shuffle_neon_64
	},
#endif
};


const char *
gf_a2name(enum GF_ALGORITHM a)
{
	if (a > GF_ALGORITHM_COUNT-2)
		return NULL;

	return gf_names[a];
}

#ifdef __x86_64__
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
#endif

uint32_t
check_available_simd_extensions()
{
	uint32_t ret = 0;
	ret |= (1 << HWCAPS_SIMD_NONE);

#ifdef __x86_64__
	unsigned int eax, ebx, ecx, edx;
	
	eax = 1;
	ebx = ecx = edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (edx & (1 << 23))
		ret |= (1 << HWCAPS_SIMD_MMX);
	if (edx & (1 << 25))
		ret |= (1 << HWCAPS_SIMD_SSE);
	if (edx & (1 << 26))
		ret |= (1 << HWCAPS_SIMD_SSE2);
	if (ecx & (1 << 9))
		ret |= (1 << HWCAPS_SIMD_SSSE3);
	if (ecx & (1 << 19))
		ret |= (1 << HWCAPS_SIMD_SSE41);
	if (ecx & (1 << 20))
		ret |= (1 << HWCAPS_SIMD_SSE42);
	if (ecx & (1 << 28))
		ret |= (1 << HWCAPS_SIMD_AVX);

	eax = 7;
	ebx = ecx = edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if (ebx & (1 << 5))
		ret |= (1 << HWCAPS_SIMD_AVX2);
#endif

#ifdef __arm__
	//FIXME ARM does not have this kind of cpuid. For now, we assume that the
	//platform we are running on supports neon.
	ret |= (1 << HWCAPS_SIMD_NEON);
#endif

	return ret;
}
	
int
gf_get(struct galois_field *gf, enum GF_TYPE type, enum GF_ALGORITHM atype)
{
	int ret = 0;
	int hwcaps;
	
	memset(gf, 0, sizeof(*gf));

	hwcaps = check_available_simd_extensions();
		
	switch (type) {
		gf->hwcaps = (1 << HWCAPS_SIMD_NONE);
	case GF2:
		strcpy(gf->name, "GF2");
		gf->type		= GF2;
		gf->ppoly		= GF2_POLYNOMIAL;
		gf->exponent		= GF2_EXPONENT;
		gf->size		= GF2_SIZE;
		gf->mask		= GF2_MASK;
		gf->inv			= inv2;
		break;

	case GF4:
		strcpy(gf->name, "GF4");
		gf->type		= GF4;
		gf->ppoly		= GF4_POLYNOMIAL;
		gf->exponent		= GF4_EXPONENT;
		gf->size		= GF4_SIZE;
		gf->mask		= GF4_MASK;
		gf->inv			= inv4;
		break;
	
	case GF16:
		strcpy(gf->name, "GF16");
		gf->type		= GF16;
		gf->ppoly		= GF16_POLYNOMIAL;
		gf->exponent		= GF16_EXPONENT;
		gf->size		= GF16_SIZE;
		gf->mask		= GF16_MASK;
		gf->inv			= inv16;
		break;
	
	case GF256:
		strcpy(gf->name, "GF256");
		gf->type		= GF256;
		gf->ppoly		= GF256_POLYNOMIAL;
		gf->exponent		= GF256_EXPONENT;
		gf->size		= GF256_SIZE;
		gf->mask		= GF256_MASK;
		gf->inv			= inv256;
		break;
	default:
		return -1;
	}

	switch (atype) {
	case GF_SELFTEST:
		switch (type) {
		case GF2:
			gf->mulrc = mulrc2;
			gf->maddrc = maddrc2_scalar; 
			break;
		case GF4:
			gf->mulrc = mulrc4_imul_scalar;
			gf->maddrc = maddrc4_imul_scalar; 
			break;
		case GF16:
			gf->mulrc = mulrc16_imul_scalar;
			gf->maddrc = maddrc16_imul_scalar; 
			break;
		case GF256:
			gf->mulrc = mulrc256_pdiv;
			gf->maddrc = maddrc256_pdiv; 
			break;
		default:
			return -1;
		}
		break;

	case GF_ALGORITHM_BEST:
#ifdef __x86_64__
		if (hwcaps & (1 << HWCAPS_SIMD_AVX2)) {
			gf->hwcaps = (1 << HWCAPS_SIMD_AVX2);
			gf->mulrc  = best_algorithms[type][HWCAPS_SIMD_AVX2].mulrc;
			gf->maddrc = best_algorithms[type][HWCAPS_SIMD_AVX2].maddrc;
		}
		else if (hwcaps & (1 << HWCAPS_SIMD_SSSE3)) {
			gf->hwcaps = (1 << HWCAPS_SIMD_SSSE3);
			gf->mulrc  = best_algorithms[type][HWCAPS_SIMD_SSSE3].mulrc;
			gf->maddrc = best_algorithms[type][HWCAPS_SIMD_SSSE3].maddrc;
		}
		else if (hwcaps & (1 << HWCAPS_SIMD_SSE2)) {
			gf->hwcaps = (1 << HWCAPS_SIMD_SSE2);
			gf->mulrc  = best_algorithms[type][HWCAPS_SIMD_SSE2].mulrc;
			gf->maddrc = best_algorithms[type][HWCAPS_SIMD_SSE2].maddrc;
		}
#endif
#ifdef __arm__
		if (hwcaps & (1 << HWCAPS_SIMD_NEON)) {
			gf->hwcaps = (1 << HWCAPS_SIMD_NEON);
			gf->mulrc  = best_algorithms[type][HWCAPS_SIMD_NEON].mulrc;
			gf->maddrc = best_algorithms[type][HWCAPS_SIMD_NEON].maddrc;
		}
#endif
		else if (hwcaps & (1 << HWCAPS_SIMD_NONE)) {
			gf->mulrc  = best_algorithms[type][HWCAPS_SIMD_NONE].mulrc;
			gf->maddrc = best_algorithms[type][HWCAPS_SIMD_NONE].maddrc;
		}
		else {
			return -1;
		}
		break;

	default:
		return -1;
	}

	return ret;
}

static int
add_algorithm(struct list_head *list, enum GF_TYPE gt, enum GF_ALGORITHM at, 
		enum HWCAPS hwcaps, maddrc_t maddrc, mulrc_t mulrc)
{
	struct algorithm *alg;

	alg = malloc(sizeof(*alg));

	memset(alg, 0, sizeof(*alg));

	alg->maddrc = maddrc;
	alg->mulrc = mulrc;
	alg->hwcaps = hwcaps;
	alg->type = at;
	alg->field = gt;
	list_add_tail(&alg->list, list);

	return 0;
}

int
gf_get_algorithms(struct list_head *list, enum GF_TYPE field)
{
	struct algorithm *alg, tmp;

	if (!list)
		return -1;

	list_for_each_entry_extra_safe(alg, &tmp, list, list) {
		list_del(&alg->list);
		free(alg);
	} list_end_extra_safe(list); 

	switch (field) {
	case GF2:
		add_algorithm(list, field, GF_XOR_GPR32, HWCAPS_SIMD_NONE,
				maddrc2_gpr32, NULL);
		add_algorithm(list, field, GF_XOR_GPR64, HWCAPS_SIMD_NONE,
				maddrc2_gpr64, NULL);
#ifdef __x86_64__
		add_algorithm(list, field, GF_XOR_SSE2, HWCAPS_SIMD_SSE2,
				maddrc2_sse2, NULL);
		add_algorithm(list, field, GF_XOR_AVX2, HWCAPS_SIMD_AVX2,
				maddrc2_avx2, NULL);
#endif
#ifdef __arm__
		add_algorithm(list, field, GF_XOR_NEON_128, HWCAPS_SIMD_NEON,
				maddrc2_neon, NULL);
#endif
		break;
	case GF4:
		add_algorithm(list, field, GF_FLAT_TABLE, HWCAPS_SIMD_NONE,
				maddrc4_flat_table, NULL);
		add_algorithm(list, field, GF_IMUL_GPR32, HWCAPS_SIMD_NONE,
				maddrc4_imul_gpr32, NULL);
		add_algorithm(list, field, GF_IMUL_GPR64, HWCAPS_SIMD_NONE,
				maddrc4_imul_gpr64, NULL);
#ifdef __x86_64__
		add_algorithm(list, field, GF_IMUL_SSE2, HWCAPS_SIMD_SSE2,
				maddrc4_imul_sse2, NULL);
		add_algorithm(list, field, GF_IMUL_AVX2, HWCAPS_SIMD_AVX2,
				maddrc4_imul_avx2, NULL);
		add_algorithm(list, field, GF_SHUFFLE_SSSE3, HWCAPS_SIMD_SSSE3,
				maddrc4_shuffle_ssse3, NULL);
		add_algorithm(list, field, GF_SHUFFLE_AVX2, HWCAPS_SIMD_AVX2,
				maddrc4_shuffle_avx2, NULL);
#endif
#ifdef __arm__
		add_algorithm(list, field, GF_IMUL_NEON_64, HWCAPS_SIMD_NEON,
				maddrc4_imul_neon_64, NULL);
		add_algorithm(list, field, GF_IMUL_NEON_128, HWCAPS_SIMD_NEON,
				maddrc4_imul_neon_128, NULL);
		add_algorithm(list, field, GF_SHUFFLE_NEON_64, HWCAPS_SIMD_NEON,
				maddrc4_shuffle_neon, NULL);
#endif
		break;
	case GF16:
		add_algorithm(list, field, GF_FLAT_TABLE, HWCAPS_SIMD_NONE,
				maddrc16_flat_table, NULL);
		add_algorithm(list, field, GF_IMUL_GPR32, HWCAPS_SIMD_NONE,
				maddrc16_imul_gpr32, NULL);
		add_algorithm(list, field, GF_IMUL_GPR64, HWCAPS_SIMD_NONE,
				maddrc16_imul_gpr64, NULL);
#ifdef __x86_64__
		add_algorithm(list, field, GF_IMUL_SSE2, HWCAPS_SIMD_SSE2,
				maddrc16_imul_sse2, NULL);
		add_algorithm(list, field, GF_IMUL_AVX2, HWCAPS_SIMD_AVX2,
				maddrc16_imul_avx2, NULL);
		add_algorithm(list, field, GF_SHUFFLE_SSSE3, HWCAPS_SIMD_SSSE3,
				maddrc16_shuffle_ssse3, NULL);
		add_algorithm(list, field, GF_SHUFFLE_AVX2, HWCAPS_SIMD_AVX2,
				maddrc16_shuffle_avx2, NULL);
#endif
#ifdef __arm__
		add_algorithm(list, field, GF_IMUL_NEON_64, HWCAPS_SIMD_NEON,
				maddrc16_imul_neon_64, NULL);
		add_algorithm(list, field, GF_IMUL_NEON_128, HWCAPS_SIMD_NEON,
				maddrc16_imul_neon_128, NULL);
		add_algorithm(list, field, GF_SHUFFLE_NEON_64, HWCAPS_SIMD_NEON,
				maddrc16_shuffle_neon, NULL);
#endif
		break;
	case GF256:
		add_algorithm(list, field, GF_FLAT_TABLE, HWCAPS_SIMD_NONE,
				maddrc256_flat_table, NULL);
		add_algorithm(list, field, GF_IMUL_GPR32, HWCAPS_SIMD_NONE,
				maddrc256_imul_gpr32, NULL);
		add_algorithm(list, field, GF_IMUL_GPR64, HWCAPS_SIMD_NONE,
				maddrc256_imul_gpr64, NULL);
#ifdef __x86_64__
		add_algorithm(list, field, GF_IMUL_SSE2, HWCAPS_SIMD_SSE2,
				maddrc256_imul_sse2, NULL);
		add_algorithm(list, field, GF_IMUL_AVX2, HWCAPS_SIMD_AVX2,
				maddrc256_imul_avx2, NULL);
		add_algorithm(list, field, GF_SHUFFLE_SSSE3, HWCAPS_SIMD_SSSE3,
				maddrc256_shuffle_ssse3, NULL);
		add_algorithm(list, field, GF_SHUFFLE_AVX2, HWCAPS_SIMD_AVX2,
				maddrc256_shuffle_avx2, NULL);
#endif
#ifdef __arm__
		add_algorithm(list, field, GF_IMUL_NEON_64, HWCAPS_SIMD_NEON,
				maddrc256_imul_neon_64, NULL);
		add_algorithm(list, field, GF_IMUL_NEON_128, HWCAPS_SIMD_NEON,
				maddrc256_imul_neon_128, NULL);
		add_algorithm(list, field, GF_SHUFFLE_NEON_64, HWCAPS_SIMD_NEON,
				maddrc256_shuffle_neon, NULL);
#endif
		break;

	default:
		return -1;
	}

	return 0;
}

