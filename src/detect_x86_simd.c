/*
 * Copyright 2014	Stephan M. Guenther <moepi@moepi.net>
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

#include <stddef.h>
#include <moepgf/moepgf.h>

#include "detect_x86_simd.h"

static void
cpuid(unsigned int *eax, unsigned int *ebx, unsigned int *ecx,
							unsigned int *edx)
{
	asm volatile("cpuid"
		: "=a" (*eax),
		  "=b" (*ebx),
		  "=c" (*ecx),
		  "=d" (*edx)
		: "0" (*eax), "2" (*ecx));
}

uint32_t
detect_x86_simd()
{
	uint32_t hwcaps = 0;
	unsigned int eax = 1;
	unsigned int ebx, ecx, edx = 0;
	
	cpuid(&eax, &ebx, &ecx, &edx);

	if (edx & (1 << 23))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_MMX);
	if (edx & (1 << 25))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_SSE);
	if (edx & (1 << 26))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_SSE2);
	if (ecx & (1 << 9))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_SSSE3);
	if (ecx & (1 << 19))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_SSE41);
	if (ecx & (1 << 20))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_SSE42);
	if (ecx & (1 << 28))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_AVX);

	eax = 7;
	ebx = ecx = edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);

	if (ebx & (1 << 5))
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_AVX2);

	return hwcaps;
}
