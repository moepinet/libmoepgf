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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf.h"
#include "gf4.h"
#include "xor.h"

#if GF4_POLYNOMIAL == 7
#include "gf4tables7.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t inverses[GF4_SIZE] = GF4_INV_TABLE;
static const uint8_t pt[GF4_SIZE][GF16_EXPONENT] = GF4_POLYNOMIAL_DIV_TABLE;
static const uint8_t multab[GF4_SIZE][256] = GF4_LOOKUP_TABLE;

inline uint8_t
inv4(uint8_t element)
{
	return inverses[element];
}

inline void
maddrc4_imul_scalar(uint8_t* region1, const uint8_t* region2,
					uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint8_t r[4];

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_scalar(region1, region2, length);
		return;
	}

	for (; length; region1++, region2++, length--) {
		r[0] = ((*region2 & 0x55) >> 0) * p[0];
		r[1] = ((*region2 & 0xaa) >> 1) * p[1];
		*region1 ^= r[0] ^ r[1];
	}
}

inline void
maddrc4_imul_gpr32(uint8_t *region1, const uint8_t *region2, uint8_t constant, 
								int length)
{
	const uint8_t *p = pt[constant];
	uint32_t r64[4];

	if (constant == 0)
	       return;

        if (constant == 1) {
	       xorr_gpr32(region1, region2, length);
	       return;
        }

	for (; length > 0; region1+=4, region2+=4, length-=4) {
		r64[0] = ((*(uint32_t *)region2 & 0x55555555)>>0)*p[0];
		r64[1] = ((*(uint32_t *)region2 & 0xaaaaaaaa)>>1)*p[1];
		*((uint32_t *)region1) ^= r64[0] ^ r64[1];
	}
}

inline void
maddrc4_imul_gpr64(uint8_t *region1, const uint8_t *region2, uint8_t constant, 
								int length)
{
	const uint8_t *p = pt[constant];
	uint64_t r64[4];

	if (constant == 0)
	       return;

        if (constant == 1) {
	       xorr_gpr64(region1, region2, length);
	       return;
        }

	for (; length > 0; region1+=8, region2+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region2 & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region2 & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region1) ^= r64[0] ^ r64[1];
	}
}

inline void
maddrc4_flat_table(uint8_t *region1, const uint8_t *region2, uint8_t constant,
								int length)
{
	if (constant == 0)
	       return;

        if (constant == 1) {
		xorr_scalar(region1, region2, length);
	       return;
        }

	for (; length; region1++, region2++, length--) {
		*region1 ^= multab[constant][*region2];
	}
}

void
mulrc4_imul_scalar(uint8_t *region, uint8_t constant, int length)
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
mulrc4_imul_gpr32(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint64_t r64[2];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;
	
	for (; length > 0; region+=4, length-=4) {
		r64[0] = ((*(uint32_t *)region & 0x55555555)>>0)*p[0];
		r64[1] = ((*(uint32_t *)region & 0xaaaaaaaa)>>1)*p[1];
		*((uint32_t *)region) = r64[0] ^ r64[1];
	}
}

void
mulrc4_imul_gpr64(uint8_t *region, uint8_t constant, int length)
{
	const uint8_t *p = pt[constant];
	uint64_t r64[2];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;
	
	for (; length > 0; region+=8, length-=8) {
		r64[0] = ((*(uint64_t *)region & 0x5555555555555555)>>0)*p[0];
		r64[1] = ((*(uint64_t *)region & 0xaaaaaaaaaaaaaaaa)>>1)*p[1];
		*((uint64_t *)region) = r64[0] ^ r64[1];
	}
}

