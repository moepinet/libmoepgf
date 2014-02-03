#include <stdlib.h>
#include <string.h>

#include "coding_buffer.h"

int
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

void
cb_free(struct coding_buffer *cb)
{
	free(cb->pcb);
	free(cb->slot);
	memset(cb, 0, sizeof(*cb));
}

