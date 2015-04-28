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

#include <fuse.h>
#include "fs_types.h"

#define LOGSIZE (1024 * 16)
#define MAX_WRITE_BUFFER_SIZE BLKSIZE

typedef enum {
	OP_INIT,
	// XXX no need to implement pure read only operations
	OP_GETATTR,
	OP_READLINK,
	OP_MKNOD,
	OP_OPEN,
	OP_READDIR,
	OP_UNLINK,
	OP_RENAME,
	OP_LINK,
	OP_CHMOD,
	OP_CHOWN,
	OP_TRUNCATE,
	OP_READ,
	OP_WRITE,
	OP_STATFS,
	OP_FSYNC,
	OP_FGETATTR,
	OP_UTIMENS,
	OP_IOCTL,
	OP_COMMIT,
	OP_ABORT,
} operation_t;

#define TX_COMMIT 0
#define TX_ABORT 1
#define TX_INVALID 2

typedef union {
	// used by fs_init
	struct init_t {
		char msg[1024];
	} init_args;

	// used by fs_readlink
	struct readlink_args_t {
		char path[PATH_MAX];
		size_t len;
	} readlink_args;

	// used by fs_mknod
	struct mknod_args_t {
		char path[PATH_MAX];
		mode_t mode;
		dev_t rdev;
		uid_t uid;
		gid_t gid;
	} mknod_args;

	// used by fs_readdir
	struct readdir_args_t {
		char path[PATH_MAX];
		off_t offset;
		uint32_t i_num;
	} readdir_args;

	// used by fs_unlink
	struct unlink_args_t {
		char path[PATH_MAX];
	} unlink_args;

	// used by fs_rename
	struct rename_args_t {
		char srcpath[PATH_MAX];
		char dstpath[PATH_MAX];
	} rename_args;

	// used by fs_link
	struct link_args_t {
		char srcpath[PATH_MAX];
		char dstpath[PATH_MAX];
	} link_args;

	// used by fs_chmod
	struct chmod_args_t {
		char path[PATH_MAX];
		mode_t mode;
	} chmod_args;

	// used by fs_chown
	struct chown_args_t {
		char path[PATH_MAX];
		uid_t uid;
		gid_t gid;
	} chown_args;

	// used by fs_truncate
	struct truncate_args_t {
		char path[PATH_MAX];
		off_t size;
	} truncate_args;

	// used by fs_read
	struct read_args_t {
		char path[PATH_MAX];
		size_t size;
		off_t offset;
		uint32_t i_num;
	} read_args;

	// used by fs_write
	struct write_args_t {
		char path[PATH_MAX];
		char buf[MAX_WRITE_BUFFER_SIZE];
		size_t size;
		off_t offset;
		uint32_t i_num;			// The block number of the inode
	} write_args;

	// used by fs_utimens
	struct utimens_args_t {
		char path[PATH_MAX];
		struct timespec tv[2];
	} utimens_args;

} log_args_t;

struct log_entry {
	uint32_t txn_id;	// transaction id
	operation_t op;		// op code of this entry
	time_t time;		// time stamp when the log_entry is added

	log_args_t args;	// stores the actual data of this log entry
} __attribute__((aligned(SECTORSIZE)));

struct log {
	uint32_t txn_id;	// the current transaction id; see comment below
	uint32_t nentries;	// number of entries in the log
	char padding[SECTORSIZE - sizeof(uint32_t) - sizeof(uint32_t)];
	struct log_entry entries[LOGSIZE];
} __attribute__((aligned(SECTORSIZE)));

// Some words about txn_id:
//    Each transaction has a different txn_id; all log entries belonging
//    to the same transaction have the same txn_id.
//
//    To guarantee that two different transactions have different
//    txn_ids, we increment txn_id after each commit, and make sure that
//    the first sector of the log has the updated txn_id.
//


// log interface
void log_tx_add(operation_t op, const log_args_t *log_args, time_t time);
int log_tx_done();
void log_entry_install(const struct log_entry*);
int log_tx_abandon();
void log_replay();

#endif //LOG_H
