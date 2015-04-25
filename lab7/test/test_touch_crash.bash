#!/bin/bash

send_file=.txn
send_file_mnt=mnt/${send_file}

ioctl_exec=send_ioctl
txn="./${ioctl_exec} ${send_file_mnt}"

crash_now=5001
commit=4000

. test/libtest.bash
gcc send_ioctl.c -o ${ioctl_exec}

fuse_unmount
generate_test_msg
make_fsimg build/msg
fuse_mount #--test-ops

# touch file and commit
touch mnt/hello

test -f mnt/hello || fail "touch fail"

${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello && fail "uncommitted write found"

fuse_unmount
generate_test_msg
make_fsimg build/msg
fuse_mount

touch mnt/hello

test -f mnt/hello || fail "touch fail"

${txn} ${commit} || fail "commit fail"
${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello || fail "commit lost"

echo "touch crash pass"
