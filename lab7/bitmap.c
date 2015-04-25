#include <errno.h>

#include "disk_map.h"
#include "panic.h"
#include "bitmap.h"

bool
block_is_free(uint32_t blockno)
{
	if (super == 0 || blockno >= super->s_nblocks)
		return 0;
	if (bitmap[blockno])
		return 1;
	return 0;
}

void
free_block(uint32_t blockno)
{
	if (blockno == 0)
		panic("attempt to free zero block");
	bitmap[blockno] = 1;
}

int
alloc_block(void)
{
	int i;
	for (i = 0; i < super->s_nblocks; ++i) {
		if (block_is_free(i)) {
			bitmap[i] = 0;
			return i;
		}
	}
	return -ENOSPC;
}
