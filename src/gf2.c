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

#include <stdint.h>
#include <string.h>

#include "gf2.h"
#include "gf.h"

inline uint8_t
inv2(uint8_t element)
{
	return element;
}

inline void
maddrc2_scalar(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_gpr32(region1, region2, length);
}

inline void
maddrc2_gpr32(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_gpr32(region1, region2, length);
}

inline void
maddrc2_gpr64(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_gpr64(region1, region2, length);
}

inline void
mulrc2(uint8_t *region, uint8_t constant, int length)
{
	if (constant == 0)
		memset(region, 0, length);
}

void
gf2_init()
{
	return;
}

#ifdef __x86_64__
inline void
maddrc2_sse2(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_sse2(region1, region2, length);
}

inline void
maddrc2_avx2(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_avx2(region1, region2, length);
}
#endif

#ifdef __arm__
inline void
maddrc2_neon(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		xorr_neon(region1, region2, length);
}
#endif

