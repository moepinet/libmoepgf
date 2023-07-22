/*
 * This file is part of moep80211gf.
 *
 * Copyright (C) 2014   Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014   Maximilian Riemensberger <riemensberger@tum.de>
 *
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>
 *
 */

#include <altivec.h>

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

__attribute__((optimize("unroll-loops")))
void
xorr_vsx_128(uint8_t *region1, const uint8_t *region2, size_t length)
{
	int i;
	vector unsigned char in0, out0;

//	#pragma clang loop unroll_count(16)
	for (i=0; i<length; i+=16) {
		in0  = vec_vsx_ld(i, region2);
		out0 = vec_vsx_ld(i, region1);
		out0 = vec_xor(in0, out0);
		vec_vsx_st(out0, i, region1);
	}

//	for (end=region1+length; region1<end; region1+=16, region2+=16) {
//		in  = vec_lvebx(0, region2);
//		out = vec_lvebx(0, region1);
//		out = vec_xor(in, out);
//		vec_st(out, 0, region1);
//	}

//	register uint64x2_t in, out;
//
//	for (end=region1+length; region1<end; region1+=16, region2+=16) {
//		in  = vld1q_u64((void *)region2);
//		out = vld1q_u64((void *)region1);
//		out = veorq_u64(in, out);
//		vst1q_u64((void *)region1, out);
//	}
}

