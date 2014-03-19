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

#ifndef _MOEPGF2_H_
#define _MOEPGF2_H_

#include <stdint.h>

uint8_t inv2(uint8_t element);

void maddrc2_scalar(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);
void maddrc2_gpr32(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);
void maddrc2_gpr64(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);

void mulrc2(uint8_t *region, uint8_t constant, int length);

#ifdef __x86_64__
void maddrc2_sse2(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);
void maddrc2_avx2(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);
#endif

#ifdef __arm__
void maddrc2_neon(uint8_t *region1, const uint8_t *region2, uint8_t constant, int length);
#endif

#endif
