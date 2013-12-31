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

#ifndef _GF4_H_
#define _GF4_H_

#include <stdint.h>

uint8_t
ffinv4(uint8_t element);

void
ffadd4_region(uint8_t *region1, const uint8_t *region2, int length);

void
ffmadd4_region_c_slow(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);
void
ffmadd4_region_c_gpr(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);

void
ffmul4_region_c_slow(uint8_t *region, uint8_t constant, int length);

void
ffmul4_region_c_gpr(uint8_t *region, uint8_t constant, int length);

#ifdef __x86_64__
void
ffadd4_region_sse2(uint8_t *region1, const uint8_t *region2, int length);

void
ffadd4_region_avx2(uint8_t *region1, const uint8_t *region2, int length);

void
ffmul4_region_c_sse2(uint8_t *region, uint8_t constant, int length);

void
ffmul4_region_c_avx2(uint8_t *region, uint8_t constant, int length);

void
ffmadd4_region_c_sse2(uint8_t *region1, const uint8_t *region2,

					uint8_t constant, int length);
void
ffmadd4_region_c_avx2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);
#endif

#ifdef __arm__
void
ffmul4_region_c_neon(uint8_t *region, uint8_t constant, int length);

void
ffmadd4_region_c_neon(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length);
#endif

#endif
