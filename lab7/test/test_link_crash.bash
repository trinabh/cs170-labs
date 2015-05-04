#!/bin/bash

send_file=.txn
send_file_mnt=mnt/${send_file}

ioctl_exec=send_ioctl
txn="./${ioctl_exec} ${send_file_mnt}"
unlink_crash=6012
crash_now=5001
commit=4000

. test/libtest.bash
gcc send_ioctl.c -o ${ioctl_exec}

fuse_unmount
recreate_mnt
make_fsimg
fuse_mount

touch mnt/hello || fail "should not crash here"
${txn} ${commit} || fail "should not crash here"

test -f mnt/hello || fail "touch file failed"
nlink=`stat --format="%h" mnt/hello`
test ${nlink} -eq 1 || fail "touch failed"

ln mnt/hello mnt/hello2 || fail "link failed"

test -f mnt/hello || fail "link failed"
test -f mnt/hello2 || fail "link failed"

nlink=`stat --format="%h" mnt/hello`
test ${nlink} -eq 2 || fail "incorrect link count"
nlink=`stat --format="%h" mnt/hello2`
test ${nlink} -eq 2 || fail "incorrect link count"

${txn} ${commit}
${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello || fail "link failed"
test -f mnt/hello2 || fail "link failed"

nlink=`stat --format="%h" mnt/hello`
test ${nlink} -eq 2 || fail "incorrect link count"
nlink=`stat --format="%h" mnt/hello2`
test ${nlink} -eq 2 || fail "incorrect link count"

echo "link crash pass"

rm -f mnt/hello

sleep 1

test -f mnt/hello && fail "unlink file failed"
test -f mnt/hello2 || fail "unlink file failed"

nlink=`stat --format="%h" mnt/hello2`
test ${nlink} -eq 1 || fail "incorrect link count"

${txn} ${commit}
${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello && fail "unlink file failed"
test -f mnt/hello2 || fail "unlink file failed"

nlink=`stat --format="%h" mnt/hello2`
test ${nlink} -eq 1 || fail "incorrect link count"

echo "unlink crash pass"
fuse_unmount &>/dev/null
