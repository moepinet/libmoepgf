#include <stdio.h>

int ncmul8 (int a, int b)
{
	int i,c = 0;
	for (i=0; i<8; i++)
		if (a & (1<<i))
			c ^= (b << i);
	return c;
}

int main (int argc, void *argv[])
{
	int g = 285, h = 29, q = 284, p = 76;
	int a = 0, b = 0, c = 0, d = 0;

	for (a=0; a<256; a++) {
		for (b=0; b<256; b++) {
			c = ncmul8(a,b);		// ncmul
			d = c >> 8;			// rshift
			d = ncmul8(d,q);		// ncmul
			d = d >> 8;			// rshift
			d = ncmul8(d,h);		// ncmul
			c = c ^ d;			// xor
			c = c & 0xff;			// and

			printf("0x%02x,",c);
			if ((b % 16 == 15))
				printf("\n");
		}
	}

	return 0;

}



