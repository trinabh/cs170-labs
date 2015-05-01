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

# create file and crash without commit, should have no file
touch mnt/hello || fail "should not crash here"
test -f mnt/hello || fail "create file fail"

size=$((${RANDOM}%8192))
truncate -s ${size} mnt/hello || fail "fail to truncate file"
t_size=`stat mnt/hello --format="%s"`
test ${size} -eq ${t_size} || fail "fail to truncate file"

${txn} ${commit}

new_size=$((${RANDOM}%(1+${size})))
truncate -s ${new_size} mnt/hello || fail "fail to truncate file"
t_size=`stat mnt/hello --format="%s"`
test ${new_size} -eq ${t_size} || fail "fail to truncate file"

${txn} ${crash_now}

fuse_unmount
fuse_mount

t_size=`stat mnt/hello --format="%s"`
test ${t_size} -eq ${size} || fail "file size incorrect after crash"

echo "truncate crash pass"
fuse_unmount &>/dev/null
