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

#include "gf.h"

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

static inline void
encode(const struct galois_field *gf, uint8_t *dst, uint8_t **buffer, int len,
							int count)
{
	int i, c;

	for (i=0; i<count; i++) {
		c = rand() & gf->mask;
		gf->fmaddrc(dst, buffer[i], c, len);
	}
}

static void
selftest()
{
	int i,j,k,fset;
	int tlen = 16384+19;
	uint8_t	*test1, *test2, *test3;
	struct galois_field gf;

	fset = check_available_simd_extensions();
	fprintf(stderr, "CPU SIMD extensions detected: ");
	if (fset & HWCAPS_SIMD_SSE2)
		fprintf(stderr, "SSE2 ");
	if (fset & HWCAPS_SIMD_SSE41)
		fprintf(stderr, "SSE4.1 ");
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

	for (k=0; k<5; k++) {
		if (!((1 << k) & fset))
			continue;
		fprintf(stderr, "CPU SIMD extensions: ");
		if ((1 << k) == HWCAPS_SIMD_NONE)
			fprintf(stderr, "NONE\n");
		else if ((1 << k) == HWCAPS_SIMD_SSE2)
			fprintf(stderr, "SSE2\n");
		else if ((1 << k) == HWCAPS_SIMD_SSE41)
			fprintf(stderr, "SSE4.1\n");
		else if ((1 << k) == HWCAPS_SIMD_AVX2)
			fprintf(stderr, "AVX2\n");
		else if ((1 << k) == HWCAPS_SIMD_NEON)
			fprintf(stderr, "NEON\n");

		for (i=0; i<4; i++) {
			get_galois_field(&gf, i, (1 << k));
	
			fprintf(stderr, "%s fmulrc selftest...   ", gf.name);
			for (j=gf.size-1; j>=0; j--) {
				init_test_buffers(test1, test2, test3, tlen);
	
				gf.fmulrctest(test1, j, tlen);
				gf.fmulrc(test2, j, tlen);
	
				if (memcmp(test1, test2, tlen)){
					fprintf(stderr,"FAIL: results differ, c = %d\n", j);
					exit(-1);
				}
			}
			fprintf(stderr, "\tPASS\n");
			fprintf(stderr, "%s fmaddrc selftest...  ", gf.name);
			for (j=gf.size-1; j>=0; j--) {
				init_test_buffers(test1, test2, test3, tlen);
	
				gf.fmaddrctest(test1, test3, j, tlen);
				gf.fmaddrc(test2, test3, j, tlen);
	
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

int
main()
{
	int i,j,k,fset;
	struct timespec start, end;
	struct galois_field gf;
	uint8_t **generation;
	uint8_t *frame;
	double mbps;
	int len = 2048;
	int count = 16;
	int repeat = 1024*512;

	selftest();

	fset = check_available_simd_extensions();

	// Allocate generation and fill with random data
	generation = malloc(count*sizeof(uint8_t *));
	for (i=0; i<count; i++) {
		if (posix_memalign((void *)&generation[i], 32, len))
			exit(-1);
		for (j=0; j<len; j++)
			generation[i][j] = rand();
	}

	if (posix_memalign((void *)&frame, 32, len))
		exit(-1);

	fprintf(stderr, "\nEncoding benchmark, len=%d, count=%d, repetitions=%d\n", 
			len, count, repeat);

	for (k=0; k<5; k++) {
		if (!((1 << k) & fset))
			continue;
		fprintf(stderr, "CPU SIMD extensions: ");
		if ((1 << k) == HWCAPS_SIMD_NONE)
			fprintf(stderr, "NONE\n");
		else if ((1 << k) == HWCAPS_SIMD_SSE2)
			fprintf(stderr, "SSE2\n");
		else if ((1 << k) == HWCAPS_SIMD_SSE41)
			fprintf(stderr, "SSE4.1\n");
		else if ((1 << k) == HWCAPS_SIMD_AVX2)
			fprintf(stderr, "AVX2\n");
		else if ((1 << k) == HWCAPS_SIMD_NEON)
			fprintf(stderr, "NEON\n");
		for (i=0; i<4; i++) {
			get_galois_field(&gf, i, (1 << k));

			clock_gettime(CLOCK_MONOTONIC, &start);
			for (j=0; j<repeat; j++)
				encode(&gf, frame, generation, len, count);
			clock_gettime(CLOCK_MONOTONIC, &end);
			timespecsub(&end, &start);
			mbps = (double)repeat/((double)end.tv_sec + (double)end.tv_nsec*1e-9);
			mbps *= len;
			mbps /= 1024*1024;

			fprintf(stderr, "%s: %llu sec %llu nsec \t(%.2f MiB/s)\n",
				gf.name, (long long unsigned int)end.tv_sec,
				(long long unsigned int)end.tv_nsec, mbps);
		}
		fprintf(stderr, "\n");
	}

	for (i=0; i<count; i++)
		free(generation[i]);
	free(generation);
	free(frame);

	return 0;
}
