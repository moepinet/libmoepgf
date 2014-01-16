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
#include <argp.h>
#include <errno.h>

#include "gf16.h"
#include "gf.h"
#include "gf256.h"
#include "gf4.h"
#include "gf2.h"
#include "gf256tables285.h"
#include "gf16tables19.h"

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
/*
const char *argp_program_version = "ptmsimple 1.0";
const char *argp_program_bug_address = "<leclaire@in.tum.de>";

static char args_doc[] = "IF FREQ";
	
static char doc[] =
"ptmsimple - a simple packet transfer module for moep80211\n\n"
"  IF                         Use the radio interface with name IF\n"
"  FREQ                       Use the frequency FREQ [in Hz] for the radio\n"
"                             interface; You can use M for MHz or even just\n"
"                             give the channel number.";

static struct argp_option options[] = {
	{"fset", 'f', "FSET", 0, "Specify SIMD feature set to be used"},
	{"size", 's', "SIZE", 0, "Set packet size in B"},
	{"length", 'l', "LENGTH", 0, "Set generation length"},
	{}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

static struct argp argp = {
	options,
	parse_opt,
	args_doc,
	doc
};


struct arguments {
	int fset;
	int size;
	int length;
} args;


static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	char *endptr = NULL;
			
	switch (key) {
	case 'fset':

		break;
//	case 'a':
//		if (!(args->addr = ieee80211_aton(arg)))
//			argp_failure(state, 1, errno, "Invalid hardware address");
//		break;
//	case 'm':
//		args->mtu = strtol(arg, &endptr, 0);
//		if (endptr != NULL && endptr != arg + strlen(arg))
//			argp_failure(state, 1, errno, "Invalid mtu: %s", arg);
//		if (args->mtu <= 0)
//			argp_failure(state, 1, errno, "Invalid mtu: %d", args->mtu);
//		break;
//	case 't':
//		args->tap = arg;
//		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}
*/

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
				.fun 	= ffmadd256_region_c_sse41,
				.hwcaps = HWCAPS_SIMD_SSE41,
				.name 	= "shuffle SSE4.1"
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
				.fun 	= ffmadd16_region_c_sse41,
				.hwcaps = HWCAPS_SIMD_SSE41,
				.name 	= "shuffle SSE4.1"
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
				.fun 	= ffmadd4_region_c_sse41_shuffle,
				.hwcaps = HWCAPS_SIMD_SSE41,
				.name 	= "shuffle SSE4.1"
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

static inline void
encode(const struct galois_field *gf, uint8_t *dst, uint8_t **buffer, int len,
							int count)
{
	int i;
	int c;

	for (i=0; i<count; i++) {
//		gf->fmaddrc(dst, buffer[i], gf->finv(i&gf->mask), len);
		c = rand() & gf->mask;
		gf->fmaddrc(dst, buffer[i], c, len);
	}
}

#include "gf4tables7.h"

static void
generate_gf4_shuffle_tables()
{
	int i,j;
	uint8_t shufl[4][16];
	uint8_t mul[4][4] = GF4_MUL_TABLE;
	uint8_t hw, lw;
	uint8_t tmp;
	uint8_t *test;
	uint8_t *test2;

	if (posix_memalign((void *)&test, 32, 32))
		exit(-1);
	if (posix_memalign((void *)&test2, 32, 32))
		exit(-1);


	for (i=0; i<4; i++) {
		for (j=0; j<16; j++) {
			hw = j >> 2;
			lw = j & 0x03;

			hw = mul[i][hw];
			lw = mul[i][lw];

			shufl[i][j] = (hw << 2) | lw;

			fprintf(stderr, "0x%02x,", shufl[i][j]);
		}
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n");
	for (i=0; i<4; i++) {
		for (j=0; j<16; j++) {
			fprintf(stderr, "0x%02x,", shufl[i][j] << 4);
		}
		fprintf(stderr, "\n");
	}

	for (i=0; i<32; i++) {
		tmp = (i % 4);
		test[i] = (tmp << 6) | (tmp << 4) | (tmp << 2) | tmp;
	}

	memset(test2, 0, 32);

//	ffmul4_region_c_avx2_shuffle(test, 2, 32);
	ffmadd4_region_c_slow(test2, test, 2, 32);

	for (i=0; i<32; i++) {
		fprintf(stderr, "0x%02x\n", test2[i]);
	}	
}

static void
generate_logtables(struct galois_field *gf)
{
	uint8_t *log;
	uint8_t *alog;
	uint8_t test1[1024];
	uint8_t test2[1024];
	int size = sizeof(test1);
	int i, j, g, x, l, found = 0;
	
	log = malloc(gf->size);
	alog = malloc(gf->size*2);

	found = 0;
	for (g=0; g<gf->size && found == 0; g++) {
		// Generate tables
		memset(log, 0, gf->size);
		memset(alog, 1, gf->size);
		for (i=1; i<gf->size; i++) {
			gf->fmulrctest(alog+i, g, gf->size-i);
		}
		for (i=gf->size; i<2*gf->size; i++) {
			alog[i] = alog[i-gf->size+1];
		}
		for (i=0; i<gf->size-1; i++) {
			log[alog[i]] = i;
		}

		// Test tables
		for (j=gf->size-1; j>=0; j--) {
			// Initialize test data
			for (i=0; i<size; i++) {
				test1[i] = rand() & gf->mask;
				test2[i] = test1[i];
			}

			gf->fmulrctest(test1, j, sizeof(test1));
			if (j == 0)
				memset(test2, 0, sizeof(test2));
			else
				l = log[j];
			for (i=0; i<size; i++) {
				if (test2[i] == 0)
					continue;
				x = l + log[test2[i]];
				test2[i] = alog[x];
			}
			if (memcmp(test1, test2, sizeof(test1)) != 0)
				break;
		}
		if (j < 0 && memcmp(test1, test2, sizeof(test1)) == 0)
			found = 1;
	}
	if (found)
		fprintf(stderr, "generator was %d\n", g);
	else
		fprintf(stderr, "no generator found\n");

	for (i=0; i<gf->size*2-1; i++) {
		if (i % 16 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "0x%02x,", alog[i]);
	}
	fprintf(stderr, "\n");
	for (i=0; i<gf->size; i++) {
		if (i % 16 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "0x%02x,", log[i]);
	}
	fprintf(stderr, "\n");

	free(log);
	free(alog);
}

static void
generate_multable(struct galois_field *gf) {
	int i,j;
	uint8_t **mul;

	mul = malloc(gf->size * sizeof(mul));
	for (i=0; i<gf->size; i++)
		mul[i] = malloc(gf->size);

	for (i=0; i<gf->size; i++) {
		fprintf(stderr,"{");
		for (j=0; j<gf->size; j++) {
			if (j%16 == 0)
				fprintf(stderr,"\\\n");
			mul[i][j] = i;
			gf->fmulrctest(&mul[i][j],j,1);
			fprintf(stderr,"0x%02x,", mul[i][j]);
		}
		fprintf(stderr,"\\\n},");
	}

	for (i=0; i<gf->size; i++)
		free(mul[i]);
	free(mul);
}

static void
generate_gf4_multable()
{
	int i,j;
	uint8_t mtab[4][256];
	uint8_t mul[4][4] = GF4_MUL_TABLE;
	uint8_t w0,w1,w2,w3;

	for (i=0; i<4; i++) {
		for (j=0; j<256; j++) {
			if ((j%16) == 0)
				fprintf(stderr, "\n");
			w0 = j & 0x03;
			w1 = (j >> 2) & 0x03;
			w2 = (j >> 4) & 0x03;
			w3 = (j >> 6) & 0x03;

			w0 = mul[i][w0];
			w1 = mul[i][w1];
			w2 = mul[i][w2];
			w3 = mul[i][w3];

			mtab[i][j] = (w3 << 6) | (w2 << 4) | (w1 << 2) | w0;
			fprintf(stderr, "0x%02x,", mtab[i][j]);
		}
		fprintf(stderr, "\n");
	}
}

static void
generate_gf16_multable()
{
	int i,j;
	uint8_t mtab[16][256];
	uint8_t mul[16][16] = GF16_MUL_TABLE;
	uint8_t w0,w1;

	for (i=0; i<16; i++) {
		for (j=0; j<256; j++) {
			if ((j%16) == 0)
				fprintf(stderr, "\\\n");
			w0 = j & 0x0f;
			w1 = (j >> 4) & 0x0f;

			w0 = mul[i][w0];
			w1 = mul[i][w1];

			mtab[i][j] = (w1 << 4) | w0;
			fprintf(stderr, "0x%02x,", mtab[i][j]);
		}
		fprintf(stderr, "\\\n},{");
	}
}

static void
selftest()
{
	int i,j,k,fset;
//	int tlen = 16384+19;
	int tlen = 123;
	uint8_t	*test1, *test2, *test3;
	struct galois_field gf;

	get_galois_field(&gf, GF256, 1);
//	generate_logtables(&gf);
//	generate_multable(&gf);
	//generate_gf4_shuffle_tables();
	generate_gf16_multable();

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

				if (i == GF256) {
					gf.fmaddrc = ffmadd256_region_c_avx2;
				}
				gf.fmaddrctest(test1, test3, j, tlen);
				gf.fmaddrc(test2, test3, j, tlen);
	
				if (memcmp(test1, test2, tlen)){
					fprintf(stderr,"FAIL: results differ, c = %d\n", j);
				//	exit(-1);
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
enc(madd_t madd, int mask, uint8_t *dst, uint8_t **generation, int len, int count)
{
	int i;
	int c;

	for (i=0; i<count; i++) {
//		gf->fmaddrc(dst, buffer[i], gf->finv(i&gf->mask), len);
		c = rand() & mask;
		madd(dst, generation[i], c, len);
	}
}

static void
benchmark(int len, int count, int repeat)
{
	int i,j,k,l,fset;
	struct timespec start, end;
	uint8_t **generation;
	uint8_t *frame;
	double mbps;
	
	fset = check_available_simd_extensions();

	fprintf(stderr, "\nEncoding benchmark, len=%d, count=%d, repetitions=%d\n", 
			len, count, repeat);

	if (posix_memalign((void *)&frame, 32, len))
		exit(-1);
	generation = malloc(count*sizeof(uint8_t *));
	for (k=0; k<count; k++) {
		if (posix_memalign((void *)&generation[k], 32, len))
			exit(-1);
	}

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

			for (k=0; k<count; k++) {
				for (l=0; l<len; l++)
					generation[k][l] = rand();
			}

			clock_gettime(CLOCK_MONOTONIC, &start);
			for (k=0; k<repeat; k++) {
				enc(gf[i].maddrc[j].fun, gf[i].mask, frame,
						generation, len, count);
			}
			clock_gettime(CLOCK_MONOTONIC, &end);
			timespecsub(&end, &start);

			mbps = (double)repeat/((double)end.tv_sec +
					(double)end.tv_nsec*1e-9);
			mbps *= len;
			mbps /= 1024*1024;

			fprintf(stderr, "%s:\t\t%llu sec %llu nsec \t(%.2f MiB/s)\n",
				gf[i].maddrc[j].name,
				(long long unsigned int)end.tv_sec,
				(long long unsigned int)end.tv_nsec, mbps);
	
		}
		fprintf(stderr, "\n");
	}
			
	for (k=0; k<count; k++)
		free(generation[k]);
	free(generation);
	free(frame);
	
}

int
main()
{
	int len = 2048;
	int count = 16;
	int repeat = 1024*512;

	selftest();

	benchmark(len, count, repeat);

	return 0;
}
