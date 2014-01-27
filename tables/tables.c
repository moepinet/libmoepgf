#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>

enum GF_TYPE {
	GF2	= 0,
	GF4	= 1,
	GF16	= 2,
	GF256	= 3
};

#define SBUF_SIZE 1024*1024
static char sbuf[SBUF_SIZE];

struct galois_field {
	enum GF_TYPE type;
	uint32_t ppoly;
	uint32_t mask;
	int size;
	int exponent;
	uint8_t **pt;
};

struct {
	uint16_t gf2[1];
	uint16_t gf4[1];
	uint16_t gf16[2];
	uint16_t gf256[16];
} primitive_polynomials =
{
	.gf2	= {3},
	.gf4	= {7},
	.gf16	= {19,25},
	.gf256	= {285,299,301,333,351,355,357,361,369,391,397,425,451,463,487,501}
};

static void
print_usage(const char *name)
{
	fprintf(stdout, "\nUsage: %s -s <field_size> -p <ppoly>\n\n", name);
}

static int
print(const char *fmt, ...)
{
	static int init_done = 0;
	static int len = 0;

	if (!init_done) {
		memset(sbuf, 0, SBUF_SIZE);
		init_done = 1;
	}

	va_list args;

	va_start(args, fmt);
	len += vsnprintf(sbuf, SBUF_SIZE-len, fmt, args);
	va_end(args);

	if (len >= SBUF_SIZE)
		return -1;

	return 0;
}

static void
print_banner()
{
	fprintf(stdout, "/*\n");
	fprintf(stdout, " * This file is part of moep80211gf.\n");
	fprintf(stdout, " *\n");
	fprintf(stdout, " * Copyright (C) 2014   Stephan M. Guenther <moepi@moepi.net>\n");
	fprintf(stdout, " * Copyright (C) 2014   Maximilian Riemensberger <riemensberger@tum.de>\n");
	fprintf(stdout, " * Copyright (C) 2013   Alexander Kurtz <alexander@kurtz.be>\n");
	fprintf(stdout, " *\n");
	fprintf(stdout, " * moep80211gf is free software: you can redistribute it and/or modify it under\n");
	fprintf(stdout, " * the terms of the GNU General Public License as published by the Free Software\n");
	fprintf(stdout, " * Foundation, version 2 of the License.\n");
	fprintf(stdout, " *\n");
	fprintf(stdout, " * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY\n");
	fprintf(stdout, " * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR\n");
	fprintf(stdout, " * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n");
	fprintf(stdout, " *\n");
	fprintf(stdout, " * You should have received a copy of the GNU General Public License * along\n");
	fprintf(stdout, " * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.\n");
	fprintf(stdout, " */\n\n");
}

static int
ld(int x)
{
	int ret = 0;

	while (x >>= 1)
		ret++;

	return ret;
}

static int
is_primitive(uint16_t poly, enum GF_TYPE gf_type)
{
	int i, count;
	uint16_t *p;

	switch (gf_type) {
	case GF2:
		count = 1;
		p = primitive_polynomials.gf2;
		break;
	case GF4:
		count = 1;
		p = primitive_polynomials.gf4;
		break;
	case GF16:
		count = 2;
		p = primitive_polynomials.gf16;
		break;
	case GF256:
		count = 16;
		p = primitive_polynomials.gf256;
		break;
	}

	for (i=0; i<count; i++) {
		if (p[i] == poly)
			return 1;
	}

	return 0;
}

static uint8_t
mul(struct galois_field *gf, uint8_t x, uint8_t y)
{
	int i;
	uint8_t z = 0;
	uint8_t *p = gf->pt[y];

	for (i=0; i<gf->exponent; i++)
		z ^= (x & (1 << i)) ? p[i] : 0;

	return z;
}

static void
generate_polynomial_div_table(struct galois_field *gf)
{
	int i,j;
	
	gf->pt = malloc(gf->size * sizeof(uint8_t *));

	for (i=0; i<gf->size; i++) {
		gf->pt[i] = malloc(gf->exponent);
		memset(gf->pt[i], 0, gf->exponent);
	}

	for (i=0; i<gf->size; i++) {	
		gf->pt[i][0] = i;
		for (j=1; j<gf->exponent; j++) {
			gf->pt[i][j] = (gf->pt[i][j-1] << 1) & gf->mask;
			if (gf->pt[i][j-1] & (1 << (gf->exponent-1)))
				gf->pt[i][j] ^= gf->ppoly & gf->mask;
		}
	}
}

static void
print_polynomial_div_table(struct galois_field *gf)
{
	int i,j;
	
	fprintf(stdout, "#define GF%d_POLYNOMIAL_DIV_TABLE { \\\n", gf->size);

	for (i=0; i<gf->size; i++) {
		fprintf(stdout, "{");
		for (j=0; j<gf->exponent; j++) {
			fprintf(stdout, "0x%02x", gf->pt[i][j]);
			if (j < gf->exponent-1)
				fprintf(stdout, ",");
		}
		fprintf(stdout, "}");
		if (i < gf->size-1)
			fprintf(stdout, ",");
		fprintf(stdout, "\\\n");
	}
	fprintf(stdout, "}\n\n");
}

static void
print_mul_table(struct galois_field *gf)
{
	int i,j;
	
	fprintf(stdout, "#define GF%d_MUL_TABLE { \\\n", gf->size);

	for (i=0; i<gf->size; i++) {
		fprintf(stdout, "{");
		for (j=0; j<gf->size; j++) {
			if ((j%16) == 0 && j > 0)
				fprintf(stdout, "\\\n");
			fprintf(stdout, "0x%02x", mul(gf,i,j));
			if (j < gf->size-1)
				fprintf(stdout, ",");
		}
		fprintf(stdout, "}");
		if (i < gf->size-1)
			fprintf(stdout, ",");
		fprintf(stdout, "\\\n");
	}
	fprintf(stdout, "}\n\n");
}

static void
print_lookup_table(struct galois_field *gf)
{
	int i,j,k,wcount,wsize;
	uint8_t temp, w;
	uint8_t **table;
	
	wsize	= gf->exponent;
	wcount	= 8 / wsize;

	fprintf(stdout, "#define GF%d_LOOKUP_TABLE { \\\n", gf->size);

	table = malloc(gf->size * sizeof(uint8_t *));
	for (i=0; i<gf->size; i++)
		table[i] = malloc(256);

	for (i=0; i<gf->size; i++) {
		fprintf(stdout, "{");
		for (j=0; j<256; j++) {
			if ((j%16) == 0 && j > 0)
				fprintf(stdout, "\\\n");
			temp = 0;
			for (k=0; k<wcount; k++) {
				w = j >> (k*wsize);
				w &= gf->mask;
				w = mul(gf, i, w);
				temp |= w << (k*wsize);
			}
			table[i][j] = temp;
			fprintf(stdout, "0x%02x", table[i][j]);
			if (j < 255)
				fprintf(stdout, ",");
		}
		fprintf(stdout, "}");
		if (i < gf->size-1)
			fprintf(stdout, ",");
		fprintf(stdout, "\\\n");
	}
	fprintf(stdout, "}\n\n");

	for (i=0; i<gf->size; i++)
		free(table[i]);
	free(table);
}

static void
print_inv_table(struct galois_field *gf)
{
	int i, j, k;

	fprintf(stdout, "#define GF%d_INV_TABLE {\\\n", gf->size);

	k = 1;
	fprintf(stdout, "0x00,");
	for (i=1; i<gf->size; i++) {
		for (j=1; j<gf->size; j++) {
			if (mul(gf,i,j) == 1) {
				if ((k%16) == 0)
					fprintf(stdout, "\\\n");
				fprintf(stdout, "0x%02x", j);
				k++;
			}
			else
				continue;
			if (k < gf->size)
				fprintf(stdout, ",");
		}
	}
	fprintf(stdout, "\\\n}\n\n");
}

static void
print_2d_table(uint8_t **t, int x, int y)
{
	int i,j;

	for (i=0; i<x; i++) {
		for (j=0; j<y; j++) {
			if ((j%y) == 0)
				fprintf(stdout, "\n{");
			fprintf(stdout, "0x%02x", t[i][j]);
			if (j < 15)
				fprintf(stdout, ",");
		}
		fprintf(stdout, "}");
		if (i < x-1)
			fprintf(stdout, ",");
		fprintf(stdout, "\\");
	}
	fprintf(stdout, "\n}\n");
}

static void
print_shuffle_table(struct galois_field *gf)
{
	int i,j,t;
	uint8_t hw, lw, entry;
	uint8_t **ht, **lt;
	uint8_t **ht_arm, **lt_arm;

	ht = malloc(gf->size * sizeof(uint8_t *));
	lt = malloc(gf->size * sizeof(uint8_t *));
	ht_arm = malloc(gf->size * sizeof(uint8_t *));
	lt_arm = malloc(gf->size * sizeof(uint8_t *));
	for (i=0; i<gf->size; i++) {
		ht[i] = malloc(16);
		lt[i] = malloc(16);
		ht_arm[i] = malloc(16);
		lt_arm[i] = malloc(16);
	}

	for (i=0; i<gf->size; i++) {
		for (j=0; j<16; j++) {
			switch (gf->type) {
			case GF4:
				hw = j >> 2;
				lw = j & 0x03;
				hw = mul(gf,i,hw);
				lw = mul(gf,i,lw);
				lt[i][j] = (hw << 2) | lw;
				ht[i][j] = lt[i][j] << 4;
				break;
			case GF16:
				lt[i][j] = mul(gf,i,j);
				ht[i][j] = lt[i][j] << 4;
				break;
			case GF256:
				lt[i][j] = mul(gf,i,j & 0x0f);
				ht[i][j] = mul(gf,i,j << 4);
				break;
			default:
				fprintf(stderr, "invalid GF for shuffle table\n");
				exit(-1);
			}
		}
	}

	fprintf(stdout, "#ifdef __x86_64__\n");
	fprintf(stdout, "#define GF%d_SHUFFLE_LOW_TABLE { \\", gf->size);
	print_2d_table(lt, gf->size, 16);
	fprintf(stdout, "#define GF%d_SHUFFLE_HIGH_TABLE { \\", gf->size);
	print_2d_table(ht, gf->size, 16);
	fprintf(stdout, "#endif //__x86_64__\n");

	fprintf(stdout, "\n");

	for (i=0; i<gf->size; i++) {
		for (j=0; j<16; j++) {
			lt_arm[i][(2*j)%15] = lt[i][j];
			ht_arm[i][(2*j)%15] = ht[i][j];
		}
		t = lt_arm[i][0];
		lt_arm[i][0] = lt_arm[i][15];
		lt_arm[i][15] = t;
		t = ht[i][0];
		ht_arm[i][0] = ht_arm[i][15];
		ht_arm[i][15] = t;
	}
	fprintf(stdout, "#ifdef __arm__\n");
	fprintf(stdout, "#define GF%d_SHUFFLE_LOW_TABLE { \\", gf->size);
	print_2d_table(lt_arm, gf->size, 16);
	fprintf(stdout, "#define GF%d_SHUFFLE_HIGH_TABLE { \\", gf->size);
	print_2d_table(ht_arm, gf->size, 16);
	fprintf(stdout, "#endif //__arm__\n");

	for (i=0; i<gf->size; i++) {
		free(ht[i]);
		free(lt[i]);
		free(ht_arm[i]);
		free(lt_arm[i]);
	}
	free(ht);
	free(lt);
	free(ht_arm);
	free(lt_arm);
}

void
generate_log_tables(struct galois_field *gf)
{
	int i, j, k, g, x, l, size, found;
	uint8_t *alog, *log;
	uint8_t test1[1024];
	uint8_t test2[1024];

	size = sizeof(test1);

	alog = malloc(gf->size * 2);
	log = malloc(gf->size);

	found = 0;
	for (g=0; g<gf->size && found == 0; g++) {
		memset(log, 0, gf->size);
		memset(alog, 1, gf->size);
		for (i=1; i<gf->size; i++) {
			for (k=0; k<gf->size-i; k++)
				alog[i+k] = mul(gf,alog[i+k], g);
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

			for (k=0; k<sizeof(test1); k++)
				test1[k] = mul(gf,test1[k], j);
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
	if (found == 0) {
		fprintf(stderr, "no generator found\n");
		exit(-1);
	}

	fprintf(stdout, "#define GF%d_ALOG_TABLE {", gf->size);
	for (i=0; i<gf->size*2-1; i++) {
		if (i % 16 == 0)
			fprintf(stdout, "\\\n");
		fprintf(stdout, "0x%02x,", alog[i]);
	}
	fprintf(stdout, "\\\n}\n");
	fprintf(stdout, "#define GF%d_LOG_TABLE {", gf->size);
	for (i=0; i<gf->size; i++) {
		if (i % 16 == 0)
			fprintf(stdout, "\\\n");
		fprintf(stdout, "0x%02x,", log[i]);
	}
	fprintf(stdout, "\\\n}\n");

	free(log);
	free(alog);
}

int
main(int argc, char **argv)
{
	struct galois_field gf;
	uint8_t **pt;
	memset(&gf, 0, sizeof(gf));
	
	if (argc != 3)
		goto exit_fail;

	gf.size = atoi(argv[1]);
	switch (gf.size) {
		case 2:
			gf.type = GF2;
			break;
		case 4:
			gf.type = GF4;
			break;
		case 16:
			gf.type = GF16;
			break;
		case 256:
			gf.type = GF256;
			break;
		default:
			fprintf(stdout, "invalid field size: %d\n\n", gf.size);
			return 0;
	}

	gf.ppoly = (uint32_t)atoi(argv[2]);
	if (!is_primitive(gf.ppoly, gf.type)) {
		fprintf(stdout, "polynomial %d is not primitive\n\n", gf.ppoly);
		return 0;
	}

	gf.exponent = ld(gf.size);
	gf.mask = gf.size - 1;

	generate_polynomial_div_table(&gf);
	print_banner();
	print_polynomial_div_table(&gf);
	print_mul_table(&gf);
	print_inv_table(&gf);
	if (gf.type != GF256) //identical to mul table for GF256
		print_lookup_table(&gf);
	print_shuffle_table(&gf);
	generate_log_tables(&gf);



	return 0;

exit_fail:
	print_usage(argv[0]);
	exit(0);
	
	return 0;
}

