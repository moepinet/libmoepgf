#include <stdio.h>
#include <stdint.h>

uint64_t ncmul (uint64_t a, uint64_t b)
{
	uint64_t i,c = 0;
	for (i=0; i<32; i++)
		if (a & (1<<i))
			c ^= (b << i);
	return c;
}

int main (int argc, void *argv[])
{
	uint64_t g = 285, h = 29, q = 284, p = 76;
	uint64_t a = 0, b = 0, c = 0, d = 0;

	for (a=0; a<256; a++) {
		for (b=0; b<256; b++) {
			// scalar with four byte white space
			//c = ncmul(a,b);			// ncmul
			//d = ncmul(c,q);			// ncmul
			//d = ncmul(d >> 16,h);		// ncmul,rshift
			//c = c ^ d;			// xor
			//c = c & 0xff;			// and

			// vectorized with one byte white space (extend to 64!)
			c = ncmul(a,b);		// [0 a1 0 a0] * [0 0 0 b0]
			d = (c & 0xff00ff00) >> 8;
			d = ncmul(d,q);
			d = (d & 0xff00ff00) >> 8;
			d = ncmul(d,h);
			c = c ^ d;
			c = c & 0x00ff00ff;

			printf("0x%02x,",(int)c);
			if ((b % 16 == 15))
				printf("\n");
		}
	}

	return 0;

}



