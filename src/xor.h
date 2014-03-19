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

#ifndef _XOR_H_
#define _XOR_H_

#include <stdint.h>
#include <sys/types.h>

/*
 * These functions should not be called directly. Use the function pointers
 * provided by the galois_field structure instead.
 */
void xorr_scalar(uint8_t *region1, const uint8_t *region2, size_t length);
void xorr_gpr32(uint8_t *region1, const uint8_t *region2, size_t length);
void xorr_gpr64(uint8_t *region1, const uint8_t *region2, size_t length);

#ifdef __x86_64__
void xorr_sse2(uint8_t *region1, const uint8_t *region2, size_t length);
void xorr_avx2(uint8_t *region1, const uint8_t *region2, size_t length);
#endif

#ifdef __arm__
void xorr_neon_64(uint8_t *region1, const uint8_t *region2, size_t length);
void xorr_neon_128(uint8_t *region1, const uint8_t *region2, size_t length);
#endif

#endif // _XOR_H_
