/*
 * This is a library providing arithmetic functions on GF(2^1) and GF(2^8).
 * Copyright (C) 2013  Alexander Kurtz <alexander@kurtz.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>

enum GF_SIZE {
	GF2	= 0,
	GF16	= 1,
	GF256	= 2
};

void
ffdisplay(char* name, void *data, size_t length);

uint64_t
ffpow(const uint64_t base, const uint64_t previous, const int exponent,
						const uint64_t polynomial);

void
ffxor_region(uint8_t *region1, const uint8_t *region2, int length);

#endif
