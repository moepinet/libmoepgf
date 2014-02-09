/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013 	Alexander Kurtz <alexander@kurtz.be>
 * 
 * moep80211gf is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License * along
 * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>
#include "list.h"

/*
 * Definitions for hardware SIMD capabilities:
 * HWCAPS_SIMD_NONE: No SIMD extensions, usage of general purpose registers
 * (GPRs) only. Makes use of 64bit instructions if supported by hardware.
 * HWCAPS_SIMD_SSE2: Uses SIMD instructions up to and including SSE2 extensions
 * (x86/x86-64 only). 
 * HWCAPS_SIMD_SSE41: Uses SIMD instructions up to and including SSE4.1 
 * extensions (x86/x86-64 only). 
 * HWCAPS_SIMD_AVX2: Uses SIMD instructions up to and including AVX2 extensions 
 * (x86/x86-64 only). 
 * HWCAPS_SIMD_NEON: Uses SIMD instructions up to and including AVX2 extensions 
 * (ARM only). 
 */
enum HWCAPS
{
	HWCAPS_SIMD_NONE	= 0,
	HWCAPS_SIMD_MMX		= 1,
	HWCAPS_SIMD_SSE		= 2,
	HWCAPS_SIMD_SSE2	= 3,
	HWCAPS_SIMD_SSSE3	= 4,
	HWCAPS_SIMD_SSE41	= 5,
	HWCAPS_SIMD_SSE42	= 6,
	HWCAPS_SIMD_AVX		= 7,
	HWCAPS_SIMD_AVX2	= 8,
	HWCAPS_SIMD_NEON	= 9,
	HWCAPS_COUNT		= 10
};

/*
 * Defines GF parameters. Do not change.
 */
#define GF2_POLYNOMIAL		3
#define GF2_EXPONENT		1
#define GF2_SIZE		(1 << GF2_EXPONENT)
#define GF2_MASK		(GF2_SIZE - 1)

#define GF4_POLYNOMIAL		7
#define GF4_EXPONENT		2
#define GF4_SIZE		(1 << GF4_EXPONENT)
#define GF4_MASK		(GF4_SIZE - 1)

#define GF16_POLYNOMIAL		19
#define GF16_EXPONENT		4
#define GF16_SIZE		(1 << GF16_EXPONENT)
#define GF16_MASK		(GF16_SIZE - 1)

#define GF256_POLYNOMIAL	285
#define GF256_EXPONENT		8
#define GF256_SIZE		(1 << GF256_EXPONENT)
#define GF256_MASK		(GF256_SIZE - 1)

typedef void (*maddrc_t)(uint8_t *, const uint8_t *, uint8_t, int);
typedef void (*mulrc_t)(uint8_t *, uint8_t, int);
typedef uint8_t (*inv_t)(uint8_t);

/*
 * Used to identify differen GFs.
 */
enum GF_TYPE {
	GF2	= 0,
	GF4	= 1,
	GF16	= 2,
	GF256	= 3,
	GF_COUNT
};

enum GF_ALGORITHM
{
	GF_SELFTEST		= 0,
	GF_XOR_SCALAR,
	GF_XOR_GPR32,
	GF_XOR_GPR64,
	GF_XOR_SSE2,
	GF_XOR_AVX2,
	GF_XOR_NEON_128,
	GF_LOG_TABLE,
	GF_FLAT_TABLE,
	GF_IMUL_SCALAR,
	GF_IMUL_GPR32,
	GF_IMUL_GPR64,
	GF_IMUL_SSE2,
	GF_IMUL_AVX2,
	GF_IMUL_NEON_64,
	GF_IMUL_NEON_128,
	GF_SHUFFLE_SSSE3,
	GF_SHUFFLE_AVX2,
	GF_SHUFFLE_NEON_64,
	GF_ALGORITHM_BEST,
	GF_ALGORITHM_COUNT
};

struct algorithm {
	struct list_head	list;
	maddrc_t		maddrc;
	mulrc_t			mulrc;
	enum HWCAPS		hwcaps;
	enum GF_ALGORITHM	type;
	enum GF_TYPE		field;
};

/*
 * Structure representing a GF, including functions to user-accessible
 * functions.
 */
struct galois_field {
	enum GF_TYPE		type;
	enum HWCAPS		hwcaps;
	char 			name[256];
	uint32_t		exponent;
	uint32_t		mask;
	uint32_t		ppoly;
	int			size;
	maddrc_t		maddrc;
	mulrc_t			mulrc;
	inv_t			inv;
};

const char * gf_a2name(enum GF_ALGORITHM a);

/*
 * Checks for SIMD extensions offered by hardware. Return value is a bitmask
 * indicating available HWCAPS.
 */
uint32_t check_available_simd_extensions();

/*
 * Initializes the GF pointed to by gf according to the requested type. SIMD
 * extensions may be explicitly requested by the parameter fset. If fset is set
 * to zero, the fastest implementation available for the current architecture is
 * automatically determined. The function returns 0 on succes and -1 on any
 * error, e.g. the requested SIMD extensions are not available.
 */
int gf_get(struct galois_field *gf, enum GF_TYPE type, enum GF_ALGORITHM atype);
int gf_get_algorithms(struct list_head *list, enum GF_TYPE type);

void xorr_scalar(uint8_t *region1, const uint8_t *region2, int length);
void xorr_gpr32(uint8_t *region1, const uint8_t *region2, int length);
void xorr_gpr64(uint8_t *region1, const uint8_t *region2, int length);

/*
 * These functions should not be called directly. Use the function pointers
 * provided by the galois_field structure instead.
 */
#ifdef __x86_64__
void xorr_sse2(uint8_t *region1, const uint8_t *region2, int length);
void xorr_avx2(uint8_t *region1, const uint8_t *region2, int length);
#endif

#ifdef __arm__
void xorr_neon_64(uint8_t *region1, const uint8_t *region2, int length);
void xorr_neon_128(uint8_t *region1, const uint8_t *region2, int length);
#endif

#endif

