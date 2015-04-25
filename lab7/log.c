#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "passert.h"
#include "inode.h"

// Add an entry to the log
void
log_tx_add(struct log* log, uint32_t op, char* data)
{
	// Your code here.
}

// Add a commit entry to the log, marking the transaction complete.
// Return TX_COMMIT once the commit entry has persisted to disk.
int
log_tx_done(struct log* log)
{
	// Your code here.
	return TX_COMMIT;
}

// Abandons the current transaction: increase the log's transaction
// number.
int
log_tx_abandon(struct log* log)
{
	if (!log->newtx) {
		++log->t_no;
		log->newtx = true;
	}
	return TX_INVALID;
}


// Helper function to give the last entry in the log
struct log_entry* log_last_entry(struct log* log) {
	int p = (log->tail - 1 + LOGSIZE) % LOGSIZE;
	return &log->entries[p];
}

// Take single entry and install it on the disk.
// Set retdata to the appropriate return value.
void log_entry_install(void* retdata, struct log_entry* entry) {
	switch (entry->op) {
	case OP_NOOP:
		printf("no-op with message: %s\n", entry->data);
	case OP_COMMIT:
	case OP_ABORT:
		break;
	case OP_LINK:
		// Your code here
		break;
	case OP_UNLINK:
		// Your code here
		break;
	case OP_WRITE:
		// Your code here
		break;
        case OP_MKNOD:
		// Your code here
                break;
        case OP_TRUNCATE:
		// Your code here
                break;
	default:
		panic("log_entry_install: couldn't install opcode %u\n", entry->op);
	}
}

// Replay committed transactions.
int log_replay(struct log* log) {
	// Your code here
}

