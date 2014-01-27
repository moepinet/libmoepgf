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

#include "gf256.h"
#include "gf.h"

#if GF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t tl[GF256_SIZE][16] = GF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[GF256_SIZE][16] = GF256_SHUFFLE_HIGH_TABLE;

void
maddrc256_shuffle_neon(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	register uint8x8x2_t t1, t2;
	register uint8x8_t m1, m2, in1, in2, out, l, h;

	if (constant == 0)
		return;

	if (constant == 1) {
		xorr_neon(region1, region2, length);
		return;
	}

	t1 = vld2_u8((void *)tl[constant]);
	t2 = vld2_u8((void *)th[constant]);
	m1 = vdup_n_u8(0x0f);
	m2 = vdup_n_u8(0xf0);

	for (; length & 0xfffffff8; region1+=8, region2+=8, length-=8) {
		in2 = vld1_u8((void *)region2);
		in1 = vld1_u8((void *)region1);
		l = vand_u8(in2, m1);
		l = vtbl2_u8(t1, l);
		h = vand_u8(in2, m2);
		h = vshr_n_u8(h, 4);
		h = vtbl2_u8(t2, h);
		out = veor_u8(h, l);
		out = veor_u8(out, in1);
		vst1_u8(region1, out);
	}
	
	maddrc256_imul_gpr32(region1, region2, constant, length);
}

void
mulrc256_shuffle_neon(uint8_t *region, uint8_t constant, int length)
{
	register uint8x8x2_t t1, t2;
	register uint8x8_t m1, m2, in, out, l, h;

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	t1 = vld2_u8((void *)tl[constant]);
	t2 = vld2_u8((void *)th[constant]);
	m1 = vdup_n_u8(0x0f);
	m2 = vdup_n_u8(0xf0);

	for (; length & 0xfffffff8; region+=8, length-=8) {
		in = vld1_u8((void *)region);
		l = vand_u8(in, m1);
		l = vtbl2_u8(t1, l);
		h = vand_u8(in, m2);
		h = vshr_n_u8(h, 4);
		h = vtbl2_u8(t2, h);
		out = veor_u8(h, l);
		vst1_u8((void *)region, out);
	}
	
	mulrc256_imul_gpr32(region, constant, length);
}

