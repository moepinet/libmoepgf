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
#define HWCAPS_SIMD_NONE	(1 << 0)
#define HWCAPS_SIMD_SSE2	(1 << 1)
#define HWCAPS_SIMD_SSE41	(1 << 2)
#define HWCAPS_SIMD_AVX2	(1 << 3)
#define HWCAPS_SIMD_NEON	(1 << 4)

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

/*
 * Used to identify differen GFs.
 */
enum GF_TYPE {
	GF2	= 0,
	GF4	= 1,
	GF16	= 2,
	GF256	= 3
};

/*
 * Structure representing a GF, including functions to user-accessible
 * functions.
 */
struct galois_field {
	char		name[16];
	uint32_t	simd;
	enum GF_TYPE	type;

	int	polynomial;
	int	exponent;
	int	size;
	int	mask;
	
	void	(* fmulrctest)(uint8_t *, uint8_t, int);
	void	(* fmaddrctest)(uint8_t *, const uint8_t *, uint8_t, int);

	uint8_t (* finv)(uint8_t);
	void	(* faddr)(uint8_t *, const uint8_t *, int);
	void	(* fmulrc)(uint8_t *, uint8_t, int);
	void	(* fmaddrc)(uint8_t *, const uint8_t *, uint8_t, int);
};

/*
 * Checks for SIMD extensions offered by hardware. Return value is a bitmask
 * indicating available HWCAPS.
 */
uint32_t
check_available_simd_extensions();

/*
 * Initializes the GF pointed to by gf according to the requested type. SIMD
 * extensions may be explicitly requested by the parameter fset. If fset is set
 * to zero, the fastest implementation available for the current architecture is
 * automatically determined. The function returns 0 on succes and -1 on any
 * error, e.g. the requested SIMD extensions are not available.
 */
int
get_galois_field(struct galois_field *gf, enum GF_TYPE type, uint32_t fset);

void
ffxor_region_gpr(uint8_t *region1, const uint8_t *region2, int length);

/*
 * These functions should not be called directly. Use the function pointers
 * provided by the galois_field structure instead.
 */
#ifdef __x86_64__
void
ffxor_region_sse2(uint8_t *region1, const uint8_t *region2, int length);

void
ffxor_region_avx2(uint8_t *region1, const uint8_t *region2, int length);
#endif

#ifdef __arm__
void
ffxor_region_neon(uint8_t *region1, const uint8_t *region2, int length);
#endif

#endif

