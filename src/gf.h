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
#undef __SSE2__
#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>

#define GF_MEMORY_ALIGNMENT	8
#ifdef __SSE2__
#undef GF_MEMORY_ALIGNMENT
#define GF_MEMORY_ALIGNMENT	16
#endif
#ifdef __SSE4_1__
#undef GF_MEMORY_ALIGNMENT
#define GF_MEMORY_ALIGNMENT	16
#endif
#ifdef __AVX2__
#undef GF_MEMORY_ALIGNMENT
#define GF_MEMORY_ALIGNMENT	32
#endif

#define GF2_POLYNOMIAL		3
#define GF2_EXPONENT		1
#define GF2_SIZE		(1 << GF2_EXPONENT)
#define GF2_MASK		(GF2_SIZE - 1)

#define GF4_POLYNOMIAL		7
#define GF4_EXPONENT		2
#define GF4_SIZE		(1 << GF4_EXPONENT)
#define GF4_MASK		(GF4_SIZE - 1)

#define GF16_POLYNOMIAL		19
#define GF16_EXPONENT		4
#define GF16_SIZE		(1 << GF16_EXPONENT)
#define GF16_MASK		(GF16_SIZE - 1)

#define GF256_POLYNOMIAL	285
#define GF256_EXPONENT		8
#define GF256_SIZE		(1 << GF256_EXPONENT)
#define GF256_MASK		(GF256_SIZE - 1)

enum GF_TYPE {
	GF2	= 0,
	GF4	= 1,
	GF16	= 2,
	GF256	= 3
};

struct galois_field {
	char		name[16];
	enum GF_TYPE	type;

	int	polynomial;
	int	exponent;
	int	size;
	int	mask;
	
	void	(* fmulrctest)(uint8_t *, uint8_t, int);
	void	(* fmaddrctest)(uint8_t *, const uint8_t *, uint8_t, int);

	uint8_t (* finv)(uint8_t);
	uint8_t	(* fadd)(uint8_t, uint8_t);
	uint8_t	(* fdiv)(uint8_t, uint8_t);
	uint8_t	(* fmul)(uint8_t, uint8_t);
	void	(* faddr)(uint8_t *, const uint8_t *, int);
	void	(* fdivrc)(uint8_t *, uint8_t, int);
	void	(* fmulrc)(uint8_t *, uint8_t, int);
	void	(* fmaddrc)(uint8_t *, const uint8_t *, uint8_t, int);
};

const struct galois_field __galois_fields[4];

void
ffdisplay(char* name, void *data, int length);

uint64_t
ffpow(const uint64_t base, const uint64_t previous, const int exponent,
						const uint64_t polynomial);

void
ffxor_region(uint8_t *region1, const uint8_t *region2, int length);

#endif
