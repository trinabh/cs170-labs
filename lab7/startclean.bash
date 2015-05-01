#!/bin/bash

. test/libtest.bash

fuse_unmount
echo "file system unmounted"
make_fsimg
echo "testfs.img clean"
recreate_mnt
echo "mount point mnt clean"
