#ifndef FSDRIVER_H
#define FSDRIVER_H

int fs_readlink_install(const char *path, char *target, size_t len, time_t curtime);
int fs_readdir_install(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, uint32_t i_num, time_t curtime);
int fs_mknod_install(const char *path, mode_t mode, dev_t rdev, uid_t uid, gid_t gid, time_t curtime);
int	fs_unlink_install(const char *path, time_t curtime);
int	fs_rename_install(const char *srcpath, const char *dstpath, time_t curtime);
int	fs_link_install(const char *srcpath, const char *dstpath, time_t curtime);
int fs_chmod_install(const char *path, mode_t mode, time_t curtime);
int fs_chown_install(const char *path, uid_t uid, gid_t gid, time_t curtime);
int fs_read_install(const char *path, char *buf, size_t size, off_t offset, uint32_t i_num, time_t curtime);
int fs_utimens_install(const char *path, const struct timespec tv[2], time_t curtime);
int fs_write_install(const char *path, const char *buf, size_t size, off_t offset, uint32_t i_num, time_t curtime);
int fs_truncate_install(const char *path, off_t size, time_t curtime);

#endif
