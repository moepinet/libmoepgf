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

#ifndef _MOEPGF16_H_
#define _MOEPGF16_H_

#include <stdint.h>

uint8_t inv16(uint8_t element);

void maddrc16_imul_scalar(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_imul_gpr32(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_imul_gpr64(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_flat_table(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_log_table(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);

void mulrc16_imul_scalar(uint8_t *region, uint8_t constant, size_t length);
void mulrc16_imul_gpr32(uint8_t *region, uint8_t constant, size_t length);
void mulrc16_imul_gpr64(uint8_t *region, uint8_t constant, size_t length);

#ifdef __x86_64__
void maddrc16_imul_sse2(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_imul_avx2(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_shuffle_ssse3(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_shuffle_avx2(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);

void mulrc16_imul_sse2(uint8_t *region, uint8_t constant, size_t length);
void mulrc16_shuffle_ssse3(uint8_t *region, uint8_t constant, size_t length);
void mulrc16_shuffle_avx2(uint8_t *region, uint8_t constant, size_t length);
#endif

#ifdef __arm__
void maddrc16_shuffle_neon(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_imul_neon_64(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void maddrc16_imul_neon_128(uint8_t *region1, const uint8_t *region2, uint8_t constant, size_t length);
void mulrc16_shuffle_neon(uint8_t *region, uint8_t constant, size_t length);
#endif

#endif
