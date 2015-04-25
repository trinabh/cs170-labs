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
generate_test_msg
make_fsimg build/msg
fuse_mount #--test-ops

touch mnt/hello || fail "should not crash here"
${txn} ${commit} || fail "should not crash here"

test -f mnt/hello || fail "touch file failed"

# mv hello to hello2, crash in the middle
${txn} ${unlink_crash}
old_content=`cat mnt/hello`
mv mnt/hello mnt/hello2 && fail "should crash here"

fuse_unmount
fuse_mount

test -f mnt/hello || fail "missing mnt/hello"
test -f mnt/hello2 && fail "wrong file found"

new_content=`cat mnt/hello`
test "${old_content}" == "${new_content}" || fail "incorrect content"

mv mnt/hello mnt/hello2 || fail "should not crash here"
${txn} ${commit} || fail "should not crash here"

${txn} ${crash_now}

fuse_unmount
fuse_mount

test -f mnt/hello && fail "mv fail"
test -f mnt/hello2 || fail "mv fail"

new_content=`cat mnt/hello2`
test "${old_content}" == "${new_content}" || fail "incorrect content"

echo "mv crash pass"
