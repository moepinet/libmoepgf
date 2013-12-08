#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gf.h"
#include "gf2.h"
#include "gf16.h"
#include "gf256.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)
#define CLOCK_MONOTONIC 0
static double orwl_timebase = 0.0;
static uint64_t orwl_timestart = 0;

void clock_gettime(int clk_id, struct timespec *t) {
  // be more careful in a multithreaded environement
  if (!orwl_timestart) {
    mach_timebase_info_data_t tb = { 0 };
    mach_timebase_info(&tb);
    orwl_timebase = tb.numer;
    orwl_timebase /= tb.denom;
    orwl_timestart = mach_absolute_time();
  }
  double diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
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

#define u8_to_float(x)				\
	({					\
		(float)(x)/255.0;               \
	})
#define float_to_u8(x)				\
	({					\
		(u8)((x)*255.0);                \
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
encode(uint8_t *dst, uint8_t **buffer, int len, int count, int field)
{
	int i, c;

	for (i=0; i<count; i++) {
		c = rand() & __galois_fields[field].mask;
		__galois_fields[field].fmaddrc(dst, buffer[i], c, len);
	}
}

static void
selftest()
{
	int i,j;
	int tlen = 64+19;
	uint8_t	*test1, *test2, *test3;

	if (posix_memalign((void *)&test1, 32, tlen))
		exit(-1);
	if (posix_memalign((void *)&test2, 32, tlen))
		exit(-1);
	if (posix_memalign((void *)&test3, 32, tlen))
		exit(-1);

	for (i=0; i<4; i++) {
		fprintf(stderr, "%s fmulrc selftest... ", __galois_fields[i].name);
		for (j=__galois_fields[i].size-1; j>=0; j--) {
			init_test_buffers(test1, test2, test3, tlen);

			__galois_fields[i].fmulrctest(test1, j, tlen);
			__galois_fields[i].fmulrc(test2, j, tlen);

			if (memcmp(test1, test2, tlen)){
				fprintf(stderr,"FAIL: results differ, c = %d\n", j);
				exit(-1);
			}
		}
		fprintf(stderr, "PASS\n");
		fprintf(stderr, "%s fmaddrc selftest... ", __galois_fields[i].name);
		for (j=__galois_fields[i].size-1; j>=0; j--) {
			init_test_buffers(test1, test2, test3, tlen);

			__galois_fields[i].fmaddrctest(test1, test3, j, tlen);
			__galois_fields[i].fmaddrc(test2, test3, j, tlen);

			if (memcmp(test1, test2, tlen)){
				fprintf(stderr,"FAIL: results differ, c = %d\n", j);
				exit(-1);
			}
		}
		fprintf(stderr, "PASS\n");
	}

	free(test1);
	free(test2);
	free(test3);
}

int
main(int argc, char **argv)
{
	int i,j;
	struct timespec start, end;

	selftest();

	// -----------------------------------------------------------------
	// Throughput benchmark
	// -----------------------------------------------------------------

	int len = 2048;
	int count = 16;
	int repeat = 1024*128;
	uint8_t **generation;
	uint8_t *frame;
	double mbps;

	// Allocate generation and fill with ranomd data
	generation = malloc(count*sizeof(uint8_t *));
	for (i=0; i<count; i++) {
		if (posix_memalign((void *)&generation[i], 32, len))
			exit(-1);
		for (j=0; j<len; j++)
			generation[i][j] = rand();
	}

	// Allocate frame buffer
	if (posix_memalign((void *)&frame, 32, len))
		exit(-1);

	fprintf(stderr, "\nEncoding benchmark, len=%d, count=%d\n", len, count);

	// dry-run to avoid caching panelties for gf2
	encode(frame, generation, len, count, GF2);
	for (i=0; i<4; i++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (j=0; j<repeat; j++)
			encode(frame, generation, len, count, i);
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespecsub(&end, &start);
		mbps = (double)repeat/((double)end.tv_sec + (double)end.tv_nsec*1e-9);
		mbps *= len;
		mbps /= 1024*1024;

		fprintf(stderr, "%llu sec %llu nsec (%.2f MiB/s)\n",
			(uint64_t)end.tv_sec, (uint64_t)end.tv_nsec, mbps);
	}

	for (i=0; i<count; i++)
		free(generation[i]);
	free(generation);
	free(frame);

	return 0;
}
