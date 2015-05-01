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
echo "secret" > mnt/hello || fail "should not crash here"
test -f mnt/hello || fail "create file fail"
content=`cat mnt/hello`
test "${content}" == "secret" || fail "write file fail"

${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello && fail "uncommitted stuff found"

ok "no uncommitted file"

fuse_unmount
recreate_mnt
make_fsimg
fuse_mount

echo "secret" > mnt/hello || fail "should not crash here"
${txn} ${commit} || fail "should not crash here"
${txn} ${crash_now}
fuse_unmount
fuse_mount

test -f mnt/hello || fail "committed file lost"
content=`cat mnt/hello`
test "${content}" == "secret" || fail "write to file lost"

echo "echo crash pass"
fuse_unmount &>/dev/null
