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
recreate_mnt
make_fsimg
fuse_mount


touch mnt/hello || fail "touch fail"

test -f mnt/hello || fail "touch fail"

rm -f mnt/hello || fail "rm fail"

test -f mnt/hello && fail "rm fail"

touch mnt/hello || fail "touch fail"

echo "good content" > mnt/hello || fail "write fail"

content=`cat mnt/hello`

test "${content}" == "good content" || fail "write fail"

echo "bad content" > mnt/hello2 || fail "write fail"

old_inum=`ls -i mnt/hello | cut -d' ' -f1`

mv mnt/hello mnt/hello2 || fail "mv fail"

test -f mnt/hello && fail "mv fail"

test -f mnt/hello2 || fail "mv fail"

new_inum=`ls -i mnt/hello2 | cut -d' ' -f1`

test ${new_inum} -eq ${old_inum} || fail "mv fail"

${txn} ${commit} || fail "commit fail"

echo "basic operations pass"
fuse_unmount &>/dev/null
