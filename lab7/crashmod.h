#ifndef CRASHMOD_H
#define CRASHMOD_H

// global utils
#define CAT2(a, b) a ## b
#define CAT(a, b) CAT2(a, b)

// commits
#define IOCTL_COMMIT 4000

// old
#define IOCTL_CRASH_NOW 5001
#define IOCTL_DUMP_LOG 5002
#define IOCTL_TEST_LOG 5003

// function IDs
#define FS_COUNT 15
#define PREFIX ID_

#define ID_global 0
#define ID_inode_block_walk 1
#define ID_inode_get_block 2
#define ID_inode_create 3
#define ID_inode_open 4
#define ID_inode_read 5
#define ID_inode_write 6
#define ID_inode_free_block 7
#define ID_inode_truncate_blocks 8
#define ID_inode_set_size 9
#define ID_inode_flush 10
#define ID_inode_free 11
#define ID_inode_unlink 12
#define ID_inode_link 13
#define ID_inode_stat 14

// increment - 1
#define INC_VALUE 1
#define IOCTL_INC_MIN 6000
#define IOCTL_INC_MAX (IOCTL_INC_MIN + FS_COUNT)

// increment - 1000
#define INC_T_VALUE 1000
#define IOCTL_INC_T_MIN 7000
#define IOCTL_INC_T_MAX (IOCTL_INC_T_MIN + FS_COUNT)

// get
#define IOCTL_GET_MIN 8000
#define IOCTL_GET_MAX (IOCTL_GET_MIN + FS_COUNT)

// counter modifying functions
void set_counter(int, int);
void set_counter_rel(int, int);
int dec_counter(int);
int print_counter(int);

#endif //CRASHMOD_H

