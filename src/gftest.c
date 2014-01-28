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
#include <unistd.h>

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

struct args {
	int count;
	int maxsize;
	int random;
	int repeat;
} args;

struct coding_buffer {
	int scount;
	int ssize;
	uint8_t *pcb;
	uint8_t **slot;
};

static int
cb_init(struct coding_buffer *cb, int scount, int ssize, int alignment)
{ 
	int totlen, i;

	ssize = ((ssize + alignment - 1) / alignment) * alignment;

	totlen = ssize * scount;

	if (posix_memalign((void *)&cb->pcb, alignment, totlen))
		return -1;

	memset(cb->pcb, 0, totlen);

	if (NULL == (cb->slot = malloc(scount * sizeof(cb->pcb)))) {
		free(cb->pcb);
		return -1;
	}

	for (i=0; i<scount; i++)
		cb->slot[i] = &cb->pcb[i*ssize];

	cb->ssize = ssize;
	cb->scount = scount;

	return 0;
}

static void
cb_free(struct coding_buffer *cb)
{
	free(cb->pcb);
	free(cb->slot);
	memset(cb, 0, sizeof(*cb));
}

static void
fill_random(struct coding_buffer *cb)
{
	int i,j;

	for (i=0; i<cb->scount; i++) {
		for (j=0; j<cb->ssize; j++)
			cb->slot[i][j] = rand();
	}
}

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
	int i,k,fset;
	int tlen = (1 << 15);
	uint8_t	*test1, *test2, *test3;
	struct algorithm *alg;
	struct galois_field gf;
	LIST_HEAD(alglist);

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

	for (i=0; i<4; i++) {
		gf_get(&gf, i, GF_SELFTEST);
		gf_get_algorithms(&alglist, gf.type);
		fprintf(stderr, "%s:\n", gf.name);

		list_for_each_entry(alg, &alglist, list) {
			fprintf(stderr, "- selftest (%s)    ",
							gf_a2name(alg->type));
			if (!(fset & (1 << alg->hwcaps))) {
				fprintf(stderr, "\tNecessary SIMD "
						"instructions not supported\n");
				continue;
			}

			for (k=gf.size-1; k>=0; k--) {
				init_test_buffers(test1, test2, test3, tlen);

				gf.maddrc(test1, test3, k, tlen);
				alg->maddrc(test2, test3, k, tlen);
	
				if (memcmp(test1, test2, tlen)){
					fprintf(stderr,"FAIL: results differ, c = %d\n", k);
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
encode_random(madd_t madd, int mask, uint8_t *dst, struct coding_buffer *cb)
{
	int i,c;

	for (i=0; i<cb->scount; i++) {
		c = rand() & mask;
		madd(dst, cb->slot[i], c, cb->ssize);
	}
}

static void
encode_permutation(madd_t madd, int mask, uint8_t *dst, struct coding_buffer *cb)
{
	int i,c;

	for (i=0; i<cb->scount; i++) {
		c = i % mask;
		madd(dst, cb->slot[i], c, cb->ssize);
	}
}

static void
benchmark(struct args *args)
{
	int i,l,m,rep,fset;
	struct timespec start, end;
	struct galois_field gf;
	struct algorithm *alg;
	struct coding_buffer cb;
	uint8_t *frame;
	double gbps;
	LIST_HEAD(list);
	void (*encode)(madd_t, int, uint8_t *, struct coding_buffer *);

	encode = encode_random;
	if (args->random == 0)
		encode = encode_permutation;
	
	fset = check_available_simd_extensions();

	fprintf(stderr, 
		"\nEncoding benchmark, maxsize=%d, count=%d, repetitions=%d\n", 
		args->maxsize, args->count, args->repeat);

	if (posix_memalign((void *)&frame, 32, args->maxsize))
		exit(-1);

	for (i=0; i<4; i++) {
		gf_get(&gf, i, 0);
		gf_get_algorithms(&list, gf.type);

		fprintf(stderr, "%s\n", gf.name);
		fprintf(stderr, "size \t");

		list_for_each_entry(alg, &list, list)
			fprintf(stderr, "%s \t", gf_a2name(alg->type));
		fprintf(stderr, "\n");

		for (l=128, rep=args->repeat; l<=args->maxsize; l*=2, rep/=2) {
			if (cb_init(&cb, args->count, l, 32))
				exit(-1);

			fprintf(stderr, "%d\t", l);
			list_for_each_entry(alg, &list, list) {
				if (!(fset & (1 << alg->hwcaps))) {
					fprintf(stderr, "n/a      \t");
					continue;
				}

				if (rep < 1024) {
					fprintf(stderr, "rep too small\t");
					continue;
				}

				fill_random(&cb);

				clock_gettime(CLOCK_MONOTONIC, &start);
				for (m=0; m<rep; m++)
					encode(alg->maddrc,gf.mask,frame,&cb);
				clock_gettime(CLOCK_MONOTONIC, &end);
				timespecsub(&end, &start);

				gbps = (double)rep/((double)end.tv_sec +
						(double)end.tv_nsec*1e-9);
				gbps *= l;
				gbps /= 1024*1024*1024;

				fprintf(stderr, "%.6f \t", gbps);
			}
			fprintf(stderr, "\n");

			cb_free(&cb);
		}
		fprintf(stderr, "\n");
	}
			
	free(frame);
}

int
main(int argc, char **argv)
{
	int opt;

	args.count = 16;
	args.maxsize = 1024*1024*8;
	args.repeat = 1024*1024;
	args.random = 1;

	while (-1 != (opt = getopt(argc, argv, "m:c:r:d"))) {
		switch (opt) {
		case 'm':
			args.maxsize = atoi(optarg);
			if (args.maxsize < 128) {
				fprintf(stderr, "minimum maxsize is 128 Byte\n\n");
				exit(-1);
			}
			break;
		case 'c':
			args.count = atoi(optarg);
			if (args.count < 1) {
				fprintf(stderr, "minimum count is 1\n\n");
				exit(-1);
			}
			break;
		case 'r':
			args.repeat = atoi(optarg);
			if (args.repeat < 1024) {
				fprintf(stderr, "minimum repeat is 1024\n\n");
				exit(-1);
			}
			break;
		case 'd':
			args.random = 0;
			break;
		default:
			fprintf(stderr, "unknown option %c\n\n", (char)opt);
			exit(-1);
		}
	}

	selftest();
	benchmark(&args);

	return 0;
}
