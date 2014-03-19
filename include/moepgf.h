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

#ifndef __MOEPGF_H_
#define __MOEPGF_H_

#include <stdint.h>
#include "list.h"

/*
 * Definitions for hardware SIMD capabilities:
 * MOEPGF_HWCAPS_SIMD_NONE: No SIMD extensions, usage of general purpose registers
 * (GPRs) only. Makes use of 64bit instructions if supported by hardware.
 *
 * MOEPGF_HWCAPS_SIMD_SSE2: Uses SIMD instructions up to and including SSE2 extensions
 * (x86/x86-64 only). 
 *
 * MOEPGF_HWCAPS_SIMD_SSE41: Uses SIMD instructions up to and including SSE4.1 
 * extensions (x86/x86-64 only). 
 *
 * MOEPGF_HWCAPS_SIMD_AVX2: Uses SIMD instructions up to and including AVX2 extensions 
 * (x86/x86-64 only). 
 *
 * MOEPGF_HWCAPS_SIMD_NEON: Uses SIMD instructions up to and including AVX2 extensions 
 * (ARM only). 
 */
enum MOEPGF_HWCAPS
{
	MOEPGF_HWCAPS_SIMD_NONE		= 0,
	MOEPGF_HWCAPS_SIMD_MMX		= 1,
	MOEPGF_HWCAPS_SIMD_SSE		= 2,
	MOEPGF_HWCAPS_SIMD_SSE2		= 3,
	MOEPGF_HWCAPS_SIMD_SSSE3	= 4,
	MOEPGF_HWCAPS_SIMD_SSE41	= 5,
	MOEPGF_HWCAPS_SIMD_SSE42	= 6,
	MOEPGF_HWCAPS_SIMD_AVX		= 7,
	MOEPGF_HWCAPS_SIMD_AVX2		= 8,
	MOEPGF_HWCAPS_SIMD_NEON		= 9,
	MOEPGF_HWCAPS_COUNT		= 10
};

/*
 * Defines GF parameters. Do not change.
 */
#define MOEPGF2_POLYNOMIAL		3
#define MOEPGF2_EXPONENT		1
#define MOEPGF2_SIZE			(1 << MOEPGF2_EXPONENT)
#define MOEPGF2_MASK			(MOEPGF2_SIZE - 1)

#define MOEPGF4_POLYNOMIAL		7
#define MOEPGF4_EXPONENT		2
#define MOEPGF4_SIZE			(1 << MOEPGF4_EXPONENT)
#define MOEPGF4_MASK			(MOEPGF4_SIZE - 1)

#define MOEPGF16_POLYNOMIAL		19
#define MOEPGF16_EXPONENT		4
#define MOEPGF16_SIZE			(1 << MOEPGF16_EXPONENT)
#define MOEPGF16_MASK			(MOEPGF16_SIZE - 1)

#define MOEPGF256_POLYNOMIAL		285
#define MOEPGF256_EXPONENT		8
#define MOEPGF256_SIZE			(1 << MOEPGF256_EXPONENT)
#define MOEPGF256_MASK			(MOEPGF256_SIZE - 1)

typedef void (*maddrc_t)(uint8_t *, const uint8_t *, uint8_t, int);
typedef void (*mulrc_t)(uint8_t *, uint8_t, int);
typedef uint8_t (*inv_t)(uint8_t);

/*
 * Used to identify differen GFs.
 */
enum MOEPGF_TYPE {
	MOEPGF2		= 0,
	MOEPGF4		= 1,
	MOEPGF16	= 2,
	MOEPGF256	= 3,
	MOEPGF_COUNT
};

/*
 * Algorithms available ordered by type and required feature set.
 */
enum MOEPGF_ALGORITHM
{
	MOEPGF_SELFTEST		= 0,
	MOEPGF_XOR_SCALAR,
	MOEPGF_XOR_GPR32,
	MOEPGF_XOR_GPR64,
	MOEPGF_XOR_SSE2,
	MOEPGF_XOR_AVX2,
	MOEPGF_XOR_NEON_128,
	MOEPGF_LOG_TABLE,
	MOEPGF_FLAT_TABLE,
	MOEPGF_IMUL_SCALAR,
	MOEPGF_IMUL_GPR32,
	MOEPGF_IMUL_GPR64,
	MOEPGF_IMUL_SSE2,
	MOEPGF_IMUL_AVX2,
	MOEPGF_IMUL_NEON_64,
	MOEPGF_IMUL_NEON_128,
	MOEPGF_SHUFFLE_SSSE3,
	MOEPGF_SHUFFLE_AVX2,
	MOEPGF_SHUFFLE_NEON_64,
	MOEPGF_ALGORITHM_BEST,
	MOEPGF_ALGORITHM_COUNT
};

/*
 * Structure representing a moepgf algorithm as list element, including function
 * poitners and informations about the algorithm, required hwcaps and field.
 */
struct moepgf_algorithm {
	struct list_head	list;
	maddrc_t		maddrc;
	mulrc_t			mulrc;
	enum MOEPGF_HWCAPS	hwcaps;
	enum MOEPGF_ALGORITHM	type;
	enum MOEPGF_TYPE	field;
};

/*
 * Structure representing a GF, including functions to user-accessible
 * functions.
 */
struct moepgf {
	enum MOEPGF_TYPE		type;
	enum MOEPGF_HWCAPS		hwcaps;
	char 			name[256];
	uint32_t		exponent;
	uint32_t		mask;
	uint32_t		ppoly;
	int			size;
	maddrc_t		maddrc;
	mulrc_t			mulrc;
	inv_t			inv;
};

/*
 * Translates an enum MOEPGF_ALGORITHM into a string. Not thread-safe.
 */
const char * moepgf_a2name(enum MOEPGF_ALGORITHM a);

/*
 * Checks for SIMD extensions offered by hardware. Return value is a bitmask
 * indicating available MOEPGF_HWCAPS.
 */
uint32_t moepgf_check_available_simd_extensions();

/*
 * Initializes the GF pointed to by gf according to the requested type. SIMD
 * extensions may be explicitly requested by the parameter fset. If fset is set
 * to zero, the fastest implementation available for the current architecture is
 * automatically determined. The function returns 0 on succes and -1 on any
 * error, e.g. the requested SIMD extensions are not available.
 */
int moepgf_init(struct moepgf *gf, enum MOEPGF_TYPE type, enum MOEPGF_ALGORITHM atype);

/*
 * Returns a list of all algorithms for the given field. Use the functions
 * privded in list.h to iterate over the algorithms. Useful for benchmarks only.
 */
int moepgf_get_algorithms(struct list_head *list, enum MOEPGF_TYPE type);

#endif // __MOEPGF_H_

