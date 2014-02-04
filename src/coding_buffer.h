#ifndef __CODING_BUFFER_H_
#define __CODING_BUFFER_H_

#include <stdint.h>

struct coding_buffer {
	int scount;
	int ssize;
	uint8_t *pcb;
	uint8_t **slot;
};

int	cb_init(struct coding_buffer *cb, int scount, int ssize, int alignment);
void	cb_free(struct coding_buffer *cb);

#endif //__CODING_BUFFER_H_
