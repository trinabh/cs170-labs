#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define LOGSIZE 16384
#define LOGDATASIZE 8192

#define OFFSET1 512

#define OP_NOOP 0
#define OP_LINK 1
#define OP_UNLINK 2
#define OP_WRITE 3
#define OP_MKNOD 4
#define OP_TRUNCATE 5
#define OP_COMMIT 1000
#define OP_ABORT 1001

#define TX_COMMIT 0
#define TX_ABORT 1
#define TX_INVALID 2

struct log_entry {
	uint32_t id;
	uint32_t op;
	uint32_t t_no;
	uint32_t prev;
	char data[LOGDATASIZE];
} __attribute__((packed));

struct log {
	uint32_t head;
	uint32_t tail;
	uint32_t next_id;
	uint32_t t_no;
	bool newtx;
	struct log_entry entries[LOGSIZE];
} __attribute__((packed));

// log interface
void log_tx_add(struct log*, uint32_t, char*);
int log_tx_done(struct log*);
int log_tx_abandon(struct log*);
struct log_entry* log_last_entry(struct log*);
void log_entry_install(void*, struct log_entry*);
int log_replay(struct log*);

#endif //LOG_H
