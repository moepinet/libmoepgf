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

#include <arm_neon.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf.h"
#include "gf4.h"

#if GF4_POLYNOMIAL == 7
#include "gf4tables7.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t pt[GF4_SIZE][GF16_EXPONENT] = GF4_POLYNOMIAL_DIV_TABLE;
static const uint8_t mul[GF4_SIZE][GF4_SIZE] = GF4_MUL_TABLE;

void
maddrc4_imul_neon(uint8_t *region1, const uint8_t *region2, uint8_t constant,
								int length)
{
	const uint8_t *p = pt[constant];

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_neon(region1, region2, length);
		return;
	}

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
	
	maddrc4_imul_gpr32(region1, region2, constant, length);
}

void
mulrc4_imul_neon(uint8_t *region, uint8_t constant, int length)
{
	register uint8x8_t reg, ri[2], sp[2], mi[2];
	const uint8_t *p = pt[constant];

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;
	
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
	
	mulrc4_imul_gpr32(region, constant, length);
}

