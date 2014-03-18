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

#include <arm_neon.h>

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gf.h"
#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"

void
xorr_neon_64(uint8_t *region1, const uint8_t *region2, int length)
{
	register uint64x1_t in, out;

	for (; length > 0; region1+=8, region2+=8, length-=8) {
		in  = vld1_u64((void *)region2);
		out = vld1_u64((void *)region1);
		out = veor_u64(in, out);
		vst1_u64((void *)region1, out);
	}
}

void
xorr_neon_128(uint8_t *region1, const uint8_t *region2, int length)
{
	register uint64x2_t in, out;

	for (; length > 0; region1+=16, region2+=16, length-=16) {
		in  = vld1q_u64((void *)region2);
		out = vld1q_u64((void *)region1);
		out = veorq_u64(in, out);
		vst1q_u64((void *)region1, out);
	}
}

