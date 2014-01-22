/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "gf.h"
#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"

#ifdef __MACH__
#include <mach/mach_time.h>

#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)
#define CLOCK_MONOTONIC 0

static double orwl_timebase = 0.0;
static uint64_t orwl_timestart = 0;

void clock_gettime(void *clk_id, struct timespec *t) {
	// be more careful in a multithreaded environement
	double diff;
	mach_timebase_info_data_t tb;

	// maintain signature of clock_gettime() but make gcc happy
	clk_id = NULL;

	if (!orwl_timestart) {
		mach_timebase_info(&tb);
		orwl_timebase = tb.numer;
		orwl_timebase /= tb.denom;
		orwl_timestart = mach_absolute_time();
	}
	diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
	t->tv_sec = diff * ORWL_NANO;
	t->tv_nsec = diff - (t->tv_sec * ORWL_GIGA);
}
#endif

#define timespecclear(tvp)	((tvp)->tv_sec = (tvp)->tv_nsec = 0)
#define timespecisset(tvp)	((tvp)->tv_sec || (tvp)->tv_nsec)
#define timespeccmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))
#define timespecadd(vvp, uvp)						\
	({								\
		(vvp)->tv_sec += (uvp)->tv_sec;				\
		(vvp)->tv_nsec += (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec >= 1000000000) {			\
			(vvp)->tv_sec++;				\
			(vvp)->tv_nsec -= 1000000000;			\
		}							\
	})
#define timespecsub(vvp, uvp)						\
	({								\
		(vvp)->tv_sec -= (uvp)->tv_sec;				\
		(vvp)->tv_nsec -= (uvp)->tv_nsec;			\
		if ((vvp)->tv_nsec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_nsec += 1000000000;			\
		}							\
	})
#define timespecmset(vvp, msec)						\
	({								\
		(vvp)->tv_sec = msec/1000;				\
		(vvp)->tv_nsec = msec-((vvp)->tv_sec*1000);		\
		(vvp)->tv_nsec *= 1000000;				\
	})
#define timespeccpy(vvp, uvp)						\
	({								\
		(vvp)->tv_sec = (uvp)->tv_sec;				\
		(vvp)->tv_nsec = (uvp)->tv_nsec;			\
	})

#define u8_to_float(x)							\
	({								\
		(float)(x)/255.0;               			\
	})
#define float_to_u8(x)							\
	({								\
		(u8)((x)*255.0);                			\
	})

typedef void (*madd_t)(uint8_t *, const uint8_t *, uint8_t, int);

struct algorithms {
	madd_t fun;
	int hwcaps;
	const char name[256];
};

struct gf {
	enum GF_TYPE type;
	const char name[256];
	int mask;
	struct algorithms maddrc[7];
};

struct gf gf[] = {
	{
		.type = GF256,
		.name = {"GF256"},
		.mask = 0xff,
		.maddrc = {
			{
				.fun 	= ffmadd256_region_c_log,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "log table"
			},
			{
				.fun 	= ffmadd256_region_c_table,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "lookup table"
			},
			{
				.fun 	= ffmadd256_region_c_gpr,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "imul GPR"
			},
			{
				.fun 	= ffmadd256_region_c_sse2,
				.hwcaps = HWCAPS_SIMD_SSE2,
				.name 	= "imul SSE2"
			},
			{
				.fun 	= ffmadd256_region_c_avx2_branchfree,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "imul AVX2"
			},
			{
				.fun 	= ffmadd256_region_c_ssse3,
				.hwcaps = HWCAPS_SIMD_SSSE3,
				.name 	= "shuffle SSSE3"
			},
			{
				.fun 	= ffmadd256_region_c_avx2,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "shuffle AVX2"
			},
		}
	},
	{
		.type = GF16,
		.name = {"GF16"},
		.mask = 0x0f,
		.maddrc = {
			{
				.fun 	= ffmadd16_region_c_log,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "log table"
			},
			{
				.fun 	= ffmadd16_region_c_table,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "lookup table"
			},
			{
				.fun 	= ffmadd16_region_c_gpr,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "imul GPR"
			},
			{
				.fun 	= ffmadd16_region_c_sse2,
				.hwcaps = HWCAPS_SIMD_SSE2,
				.name 	= "imul SSE2"
			},
			{
				.fun 	= ffmadd16_region_c_avx2_branchfree,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "imul AVX2"
			},
			{
				.fun 	= ffmadd16_region_c_ssse3,
				.hwcaps = HWCAPS_SIMD_SSSE3,
				.name 	= "shuffle SSSE3"
			},
			{
				.fun 	= ffmadd16_region_c_avx2,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "shuffle AVX2"
			},
		}
	},
	{
		.type = GF4,
		.name = {"GF4"},
		.mask = 0x03,
		.maddrc = {
			{
				.fun 	= ffmadd4_region_c_table,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "lookup table"
			},
			{
				.fun 	= ffmadd4_region_c_gpr,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "imul GPR"
			},
			{
				.fun 	= ffmadd4_region_c_sse2_imul,
				.hwcaps = HWCAPS_SIMD_SSE2,
				.name 	= "imul SSE2"
			},
			{
				.fun 	= ffmadd4_region_c_avx2_imul,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "imul AVX2"
			},
			{
				.fun 	= ffmadd4_region_c_ssse3_shuffle,
				.hwcaps = HWCAPS_SIMD_SSSE3,
				.name 	= "shuffle SSSE3"
			},
			{
				.fun 	= ffmadd4_region_c_avx2_shuffle,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "shuffle AVX2"
			},
			{
				.fun 	= NULL,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= ""
			},
		}
	},
	{
		.type = GF2,
		.name = {"GF2"},
		.mask = 0x01,
		.maddrc = {
			{
				.fun 	= ffmadd2_region_c_gpr,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= "XOR GPR"
			},
			{
				.fun 	= ffmadd2_region_c_sse2,
				.hwcaps = HWCAPS_SIMD_SSE2,
				.name 	= "XOR SSE2"
			},
			{
				.fun 	= ffmadd2_region_c_avx2,
				.hwcaps = HWCAPS_SIMD_AVX2,
				.name 	= "XOR AVX2"
			},
			{
				.fun 	= NULL,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= ""
			},
			{
				.fun 	= NULL,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= ""
			},
			{
				.fun 	= NULL,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= ""
			},
			{
				.fun 	= NULL,
				.hwcaps = HWCAPS_SIMD_NONE,
				.name 	= ""
			},
		}
	}
};

static void
init_test_buffers(uint8_t *test1, uint8_t *test2, uint8_t *test3, int size)
{
	int i;
	for (i=0; i<size; i++) {
		test1[i] = rand();
		test2[i] = test1[i];
		test3[i] = rand();
	}
}

static void
selftest()
{
	int i,j,k,fset;
	int tlen = 16384+19;
	uint8_t	*test1, *test2, *test3;
	struct galois_field field;

	fset = check_available_simd_extensions();
	fprintf(stderr, "CPU SIMD extensions detected: \n");
	if (fset & HWCAPS_SIMD_MMX)
		fprintf(stderr, "MMX ");
	if (fset & HWCAPS_SIMD_SSE)
		fprintf(stderr, "SSE ");
	if (fset & HWCAPS_SIMD_SSE2)
		fprintf(stderr, "SSE2 ");
	if (fset & HWCAPS_SIMD_SSSE3)
		fprintf(stderr, "SSSE3 ");
	if (fset & HWCAPS_SIMD_SSE41)
		fprintf(stderr, "SSE41 ");
	if (fset & HWCAPS_SIMD_SSE42)
		fprintf(stderr, "SSE42 ");
	if (fset & HWCAPS_SIMD_AVX)
		fprintf(stderr, "AVX ");
	if (fset & HWCAPS_SIMD_AVX2)
		fprintf(stderr, "AVX2 ");
	if (fset & HWCAPS_SIMD_NEON)
		fprintf(stderr, "NEON ");
	fprintf(stderr, "\n\n");

	if (posix_memalign((void *)&test1, 32, tlen))
		exit(-1);
	if (posix_memalign((void *)&test2, 32, tlen))
		exit(-1);
	if (posix_memalign((void *)&test3, 32, tlen))
		exit(-1);

	// FIXME SIMD detection does not work as expected
	for (i=0; i<4; i++) {
		fprintf(stderr, "%s:\n", gf[i].name);
		for (j=0; j<7; j++) {
			if (!gf[i].maddrc[j].fun)
				continue;

			if (!(fset & gf[i].maddrc[j].hwcaps)) {
				fprintf(stderr, "%s:\t\t Necessary SIMD "
						"instructions not supported\n",
						gf[i].maddrc[j].name);
				continue;
			}

			get_galois_field(&field, gf[i].type, 0);
			fprintf(stderr, "- selftest (%s)    ", 
					gf[i].maddrc[j].name);

			for (k=field.size-1; k>=0; k--) {
				init_test_buffers(test1, test2, test3, tlen);

				field.fmaddrctest(test1, test3, k, tlen);
				gf[i].maddrc[j].fun(test2, test3, k, tlen);
	
				if (memcmp(test1, test2, tlen)){
					fprintf(stderr,"FAIL: results differ, c = %d\n", j);
					exit(-1);
				}
			}
			fprintf(stderr, "\tPASS\n");
		}
		fprintf(stderr, "\n");
	}

	free(test1);
	free(test2);
	free(test3);
}

static void
enc(madd_t madd, int mask, uint8_t *dst, uint8_t *generation, int len, 
		int count)
{
	int i;
	int c;

	for (i=0; i<count; i++) {
		//c = i & mask;
		c = rand() & mask;
		madd(dst, &generation[len*i], c, len);
	}
}

static void
benchmark(int len, int count, int repeat)
{
	int i,j,k,l,m,rep,fset;
	struct timespec start, end;
	struct galois_field field;
	uint8_t *generation;
	uint8_t *frame;
	double gbps;
	
	fset = check_available_simd_extensions();

	fprintf(stderr, "\nEncoding benchmark, len=%d, count=%d, repetitions=%d\n", 
			len, count, repeat);

	if (posix_memalign((void *)&frame, 32, len))
		exit(-1);
	if (posix_memalign((void *)&generation, 32, len*count))
		exit(-1);

	for (i=0; i<4; i++) {
		get_galois_field(&field, i, 0);
		fprintf(stderr, "length \t");
		for (j=0; j<7; j++) {
			if (!gf[i].maddrc[j].fun)
				continue;
			else
				fprintf(stderr, "%s \t", gf[i].maddrc[j].name);
		}
		fprintf(stderr, "\n");

		for (l=128, rep=repeat; l<=len; l*=2, rep/=2) {
			fprintf(stderr, "%d\t", l);
			for (j=0; j<7; j++) {
				if (!gf[i].maddrc[j].fun)
					continue;

				if (!(fset & gf[i].maddrc[j].hwcaps)) {
					fprintf(stderr, "n/a      \t");
					continue;
				}

				if (rep < 1)
					continue;

				for (k=0; k<count; k++) {
					for (m=0; m<l; m++)
						generation[k*l+m] = rand();
				}

				clock_gettime(CLOCK_MONOTONIC, &start);
				for (m=0; m<rep; m++) {
					enc(gf[i].maddrc[j].fun, gf[i].mask,
						frame, generation, l, count);
				}
				clock_gettime(CLOCK_MONOTONIC, &end);
				timespecsub(&end, &start);

				gbps = (double)rep/((double)end.tv_sec +
						(double)end.tv_nsec*1e-9);
				gbps *= l;
				gbps /= 1024*1024*1024;

				fprintf(stderr, "%.6f \t", gbps);
			}
			fprintf(stderr, "\n");
		}
	}
			
	free(generation);
	free(frame);
	
}

int
main()
{
	int len = 1024*1024*8;
	int count = 16;
	int repeat = 1024*1024;

	selftest();

	benchmark(len, count, repeat);

	return 0;
}
