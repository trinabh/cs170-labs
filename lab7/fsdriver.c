#include <fuse.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>

#include "fs_types.h"
#include "inode.h"
#include "dir.h"
#include "disk_map.h"
#include "bitmap.h"
#include "panic.h"
#include "passert.h"

#include "crashmod.h"
#include "log.h"

int	fs_getattr(const char *path, struct stat *stbuf);
int	fs_readlink(const char *path, char *target, size_t len);
int	fs_mknod(const char *path, mode_t mode, dev_t rdev);
int	fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int	fs_unlink(const char *path);
int	fs_rename(const char *srcpath, const char *dstpath);
int	fs_link(const char *srcpath, const char *dstpath);
int	fs_chmod(const char *path, mode_t mode);
int	fs_chown(const char *path, uid_t uid, gid_t gid);
int	fs_truncate(const char *path, off_t size);
int	fs_open(const char *path, struct fuse_file_info *fi);
int	fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int	fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int	fs_statfs(const char *path, struct statvfs *stbuf);
int	fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi);
int	fs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi);
int	fs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
int	fs_utimens(const char *path, const struct timespec tv[2]);
int	fs_parse_opt(void *data, const char *arg, int key, struct fuse_args *outargs);

int	fs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags, void *data);

struct fuse_operations fs_oper = {
	.getattr	= fs_getattr,
	.readlink	= fs_readlink,
	.mknod		= fs_mknod,
	.opendir	= fs_open,
	.readdir	= fs_readdir,
	.unlink		= fs_unlink,
	.rename		= fs_rename,
	.link		= fs_link,
	.chmod		= fs_chmod,
	.chown		= fs_chown,
	.truncate	= fs_truncate,
	.open		= fs_open,
	.read		= fs_read,
	.write		= fs_write,
	.statfs		= fs_statfs,
	.fsync		= fs_fsync,
	.fgetattr	= fs_fgetattr,
	.utimens	= fs_utimens,
	.ioctl	        = fs_ioctl,
};

enum {
	KEY_VERSION,
	KEY_HELP,
	KEY_TEST_OPS,
};

static struct fuse_opt fs_opts[] = {
	FUSE_OPT_KEY("-V",         KEY_VERSION),
	FUSE_OPT_KEY("--version",  KEY_VERSION),
	FUSE_OPT_KEY("-h",         KEY_HELP),
	FUSE_OPT_KEY("-ho",        KEY_HELP),
	FUSE_OPT_KEY("--help",     KEY_HELP),
	FUSE_OPT_KEY("--test-ops", KEY_TEST_OPS),
	FUSE_OPT_END,
};

// --------------------------------------------------------------
// FUSE callbacks
// --------------------------------------------------------------

int
fs_getattr(const char *path, struct stat *stbuf)
{
	struct inode *ino;
	struct fuse_context *context;
	uint32_t i, nblocks, *pdiskbno;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	memset(stbuf, 0, sizeof(*stbuf));
	inode_stat(ino, stbuf);

	return 0;
}

int
fs_readlink(const char *path, char *target, size_t len)
{
	struct inode *ino;
	size_t copylen;
	char *blk;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	if ((r = inode_get_block(ino, 0, &blk)) < 0)
		return r;
	copylen = MIN(ino->i_size, len - 1);
	memcpy(target, blk, copylen);
	target[copylen] = '\0';

	return 0;
}

int
fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	struct inode *ino;
	time_t curtime;
	struct fuse_context *ctxt;
	int r;

	if ((r = inode_create(path, &ino)) < 0)
		return r;
	ino->i_size = 0;
	ino->i_mode = mode;
	ino->i_nlink = 1;
	ino->i_rdev = rdev;

	curtime = time(NULL);
	ino->i_atime = curtime;
	ino->i_ctime = curtime;
	ino->i_mtime = curtime;

	ctxt = fuse_get_context();
	ino->i_owner = ctxt->uid;
	ino->i_group = ctxt->gid;
	flush_block(ino);

	return 0;

}

int
fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	struct inode *dir = (struct inode *)fi->fh;
	struct dirent dent;
	int r;

	while ((r = inode_read(dir, &dent, sizeof(dent), offset)) > 0) {
		offset += r;
		if (dent.d_name[0] == '\0')
			continue;
		if (filler(buf, dent.d_name, NULL, offset) != 0)
			return 0;
	}
	dir->i_atime = time(NULL);
	flush_block(dir);

	return 0;
}

int
fs_unlink(const char *path)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	if (S_ISDIR(ino->i_mode))
		return -EISDIR;
	ino->i_ctime = time(NULL);
	return inode_unlink(path);
}

int
fs_rename(const char *srcpath, const char *dstpath)
{

	int r;

link_retry:
	if ((r = inode_link(srcpath, dstpath)) < 0)
		switch(-r) {
		case EEXIST:
			if (strcmp(srcpath, dstpath) == 0)
				return 0;
			if ((r = inode_unlink(dstpath)) < 0)
				return r;
			goto link_retry;
		default:
			return r;
		}
	return inode_unlink(srcpath);
}

int
fs_link(const char *srcpath, const char *dstpath)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(srcpath, &ino)) < 0)
		return r;
	if (S_ISDIR(ino->i_mode))
		return -EPERM;
	ino->i_ctime = time(NULL);
	return inode_link(srcpath, dstpath);
}

int
fs_chmod(const char *path, mode_t mode)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	if (ino == diskaddr(super->s_root))
		return -EPERM;
	ino->i_mode = mode;
	ino->i_ctime = time(NULL);
	flush_block(ino);

	return 0;
}

int
fs_chown(const char *path, uid_t uid, gid_t gid)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	if (ino == diskaddr(super->s_root))
		return -EPERM;
	if (uid != -1)
		ino->i_owner = uid;
	if (gid != -1)
		ino->i_group = gid;
	ino->i_ctime = time(NULL);
	flush_block(ino);

	return 0;
}

int
fs_truncate(const char *path, off_t size)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	ino->i_mtime = time(NULL);
	return inode_set_size(ino, size);
}

int
fs_open(const char *path, struct fuse_file_info *fi)
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	fi->fh = (uint64_t)ino;
	return 0;
}

int
fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	struct inode *ino = (struct inode *)fi->fh;
	ino->i_atime = time(NULL);
	return inode_read(ino, buf, size, offset);
}

int
fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

	printf("write size: %zu\n", size);
	struct inode *ino = (struct inode *)fi->fh;
	ino = 0;
	int i = inode_open(path, &ino);
	ino->i_mtime = time(NULL);
	int x = inode_write(ino, buf, size, offset);
	printf("done (%d)\n", x);
	return x;
}

int
fs_statfs(const char *path, struct statvfs *stbuf)
{
	int i;

	memset(stbuf, 0, sizeof(*stbuf));
	stbuf->f_bsize = BLKSIZE;
	stbuf->f_frsize = BLKSIZE;
	stbuf->f_blocks = super->s_nblocks;
	stbuf->f_fsid = super->s_magic;
	stbuf->f_namemax = PATH_MAX;
	for (i = 0; i < super->s_nblocks; ++i)
		if (block_is_free(i))
			stbuf->f_bfree++;
	stbuf->f_bavail = stbuf->f_bfree;

	return 0;
}

int
fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	struct inode *ino = (struct inode *)fi->fh;
	inode_flush(ino);
	return 0;
}

int
fs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	struct inode *ino = (struct inode *)fi->fh;
	memset(stbuf, 0, sizeof(*stbuf));
	inode_stat(ino, stbuf);

	return 0;
}

int
fs_utimens(const char *path, const struct timespec tv[2])
{
	struct inode *ino;
	int r;

	if ((r = inode_open(path, &ino)) < 0)
		return r;
	ino->i_atime = tv[0].tv_sec;
	ino->i_mtime = tv[1].tv_sec;
	ino->i_ctime = time(NULL);
	flush_block(ino);

	return 0;
}

int
fs_parse_opt(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	static const char *usage_str =
"usage: fsdriver imagefile mountpoint [options]\n"
"Mount a CS202 file system image at a given mount point.\n\n"
"Special options:\n"
"    -h, -ho, --help        show this help message and exit\n"
"    --test-ops             test basic file system operations on a specific\n"
"                           disk image, but don't mount\n"
"    -V, --version          show version information and exit\n\n"
	;
	static const char *version_str =
"fsdriver (FUSE driver for the Spring 2015 CS202 file system)\n"
"Written by Isami Romanowski.\n\n"
"Copyright (C) 2012, 2013 The University of Texas at Austin\n"
"This is free software; see the source or the provided COPYRIGHT file for\n"
"copying conditions.  There is NO warranty; not even for MERCHANTABILITY or\n"
"FITNESS FOR A PARTICULAR PURPOSE.\n\n"
	;

	switch (key) {
	case KEY_HELP:
		fputs(usage_str, stderr);
		fuse_opt_add_arg(outargs, "-ho");
		exit(fuse_main(outargs->argc, outargs->argv, &fs_oper, NULL));
	case KEY_VERSION:
		fputs(version_str, stderr);
		fuse_opt_add_arg(outargs, "--version");
		exit(fuse_main(outargs->argc, outargs->argv, &fs_oper, NULL));
	case KEY_TEST_OPS:
		if (loaded_imgname == NULL) {
			fprintf(stderr,
"fsdriver: need image for testing basic file system operations\n");
			exit(-1);
		} else {
			//fs_test();
			panic("not supported");
			exit(0);
		}
	default:
		return 1;
	}
}


/////////////////////////////////////////////////////////
// You don't need to modify any functions below here.

int
fs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags, void *data) {
	struct log* l;
	struct inode* log_ino;
	char buf[8192];
	size_t size = 8192;
	off_t offset = 0;
	int x;
	printf("received ioctl: %d\n", cmd);
	switch(cmd) {
	case IOCTL_DUMP_LOG:
		printf("dumping log...\n");
		l = super->s_log;
		printf("head: %d tail: %d\n", l->head, l->tail);
		for (x = l->tail; x < l->head; x = (x + 1) % LOGSIZE) {
			printf("entry %d: id: %d, op: %d, t_no: %d, prev: %d, data: %s\n", x, l->entries[x].id, l->entries[x].op, l->entries[x].t_no, l->entries[x].prev, l->entries[x].data);
		}
		break;
	case IOCTL_TEST_LOG:
		printf("adding entry...\n");
		l = super->s_log;
		char c[LOGDATASIZE];
		strcpy(c, "New log item");
		log_tx_add(super->s_log, OP_NOOP, c);
		printf("done\n");
		break;
	case IOCTL_COMMIT:
		printf("attempting to commit...\n");
		l = super->s_log;
		x = log_tx_done(super->s_log);
		printf("done: %d\n", x);
		return x;
	case IOCTL_CRASH_NOW:
		set_counter(ID_global, 1);
		dec_counter(ID_global);
		break;
	}

	if (cmd >= IOCTL_INC_MIN && cmd < IOCTL_INC_MAX) {
		cmd = cmd - IOCTL_INC_MIN;
		set_counter_rel(cmd, INC_VALUE);
		print_counter(cmd);
		return 0;
	}
	if (cmd >= IOCTL_INC_T_MIN && cmd < IOCTL_INC_T_MAX) {
		cmd = cmd - IOCTL_INC_T_MIN;
		set_counter_rel(cmd, INC_T_VALUE);
		print_counter(cmd);
		return 0;
	}
	if (cmd >= IOCTL_GET_MIN && cmd < IOCTL_GET_MAX) {
		cmd = cmd - IOCTL_GET_MIN;
		print_counter(cmd);
		return 0;
	}
	return 0;
}

void
wipe_disk()
{
	time_t curtime = time(NULL);
        uid_t curuid = getuid();
        gid_t curgid = getgid();

        char *diskpos = (char *)super;
        diskpos += BLKSIZE;

        if (diskpos != (char *)bitmap)
                panic("Disk image corrupted");
	int nbitblocks = 32 * (super->s_nblocks + BLKBITSIZE - 1) / BLKBITSIZE;
        memset(bitmap, 0xFF, nbitblocks * BLKSIZE);
        diskpos += nbitblocks * BLKSIZE;

        struct inode *root_inode = diskaddr(super->s_root);
        if (diskpos != (char *)root_inode)
                panic("Disk image corrupted");
        root_inode->i_nlink = 1;
        root_inode->i_mode = S_IFDIR | 0777;
        diskpos += BLKSIZE;

        struct inode *txn_ino = (struct inode *)diskpos;
        int inum = ((uintptr_t)diskpos - (uintptr_t)super) / BLKSIZE;
	txn_ino->i_mode = S_IFREG | 0666;
	txn_ino->i_size = 0;
	txn_ino->i_nlink = 1;
	txn_ino->i_atime = curtime;
	txn_ino->i_ctime = curtime;
	txn_ino->i_mtime = curtime;
	txn_ino->i_owner = curuid;
	txn_ino->i_group = curgid;
        txn_ino->i_size = 0;
        diskpos += BLKSIZE;

        char *root_blk = diskpos;
        int i;
        for (i = 0; i < N_DIRECT; i++)
                txn_ino->i_direct[i] = 0;
        struct dirent txn_ent;
        strcpy(txn_ent.d_name, ".txn");
        txn_ent.d_inum = inum;
        memset(root_blk, 0, BLKSIZE);
        memcpy(root_blk, &txn_ent, sizeof(txn_ent));
        for (i = 0; i < N_DIRECT; i++)
                root_inode->i_direct[i] = 0;
        root_inode->i_direct[0] = ((uintptr_t)root_blk - (uintptr_t)super) / BLKSIZE;
        root_inode->i_size = BLKSIZE;
        diskpos += BLKSIZE;

	for (i = 0; i < ((uintptr_t)diskpos - (uintptr_t)super) / BLKSIZE; ++i)
		bitmap[i] = 0;
		//XXX: remove
		//bitmap[i/32] &= ~(1<<(i%32));
}

int
main(int argc, char **argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	const char *imgname = NULL, *mntpoint = NULL;
	char fsname_buf[17 + PATH_MAX];
	int r, fd;

	fuse_opt_add_arg(&args, argv[0]);
	if (argc < 2)
		panic("missing image or mountpoint parameter, see help");
	for (r = 1; r < argc; r++) {
		if (imgname == NULL && argv[r][0] != '-' && strcmp(argv[r - 1], "-o") != 0) {
			imgname = argv[r];
		} else if(mntpoint == NULL && argv[r][0] != '-' && strcmp(argv[r - 1], "-o") != 0) {
			mntpoint = argv[r];
			fuse_opt_add_arg(&args, argv[r]);
		} else {
			fuse_opt_add_arg(&args, argv[r]);
		}
	}

	// Use a fsname (which shows up in df) in the style of sshfs, another
	// FUSE-based file system, with format "fsname#fslocation".
	snprintf(fsname_buf, sizeof(fsname_buf), "-ofsname=CS202fs#%s", imgname);
	fuse_opt_add_arg(&args, "-s"); // Always run single-threaded.
	fuse_opt_add_arg(&args, "-odefault_permissions"); // Kernel handles access.
	fuse_opt_add_arg(&args, fsname_buf); // Set the filesystem name.

	if (imgname == NULL) {
		fuse_opt_parse(&args, NULL, fs_opts, fs_parse_opt);
		return -1;
	} else {
		map_disk_image(imgname, mntpoint);

		// Make sure the superblock fields are sane.
		assert(super->s_magic == FS_MAGIC);
		assert(super->s_root != 0);

                // wipe out the contents of the disk (except for the log)
                wipe_disk();

		// log basic values check
		printf("log check:\n");
		printf("super->s_log->head: %d\n", super->s_log->head);
		printf("super->s_log->tail: %d\n", super->s_log->tail);

		// if there was a partial transaction in progress when the
		// driver crashed, abandon it
		log_tx_abandon(super->s_log);

		// replay log
		printf("calling log replay:\n");
		log_replay(super->s_log);

		fuse_opt_parse(&args, NULL, fs_opts, fs_parse_opt);
		return fuse_main(args.argc, args.argv, &fs_oper, NULL);
	}
}
