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

#include "xor.h"

void
xorr_scalar(uint8_t *region1, const uint8_t *region2, size_t length)
{
	for(; length; region1++, region2++, length--)
		*region1 ^= *region2;
}

void
xorr_gpr32(uint8_t *region1, const uint8_t *region2, size_t length)
{
	uint8_t *end = region1 + length;

	for(; region1 < end; region1+=4, region2+=4)
		*(uint32_t *)region1 ^= *(uint32_t *)region2;
}

void
xorr_gpr64(uint8_t *region1, const uint8_t *region2, size_t length)
{
	uint8_t *end = region1 + length;

	for(; region1 < end; region1+=8, region2+=8)
		*(uint64_t *)region1 ^= *(uint64_t *)region2;
}
