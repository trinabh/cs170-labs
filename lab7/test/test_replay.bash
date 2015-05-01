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

cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}
fuse_unmount
fuse_mount
sleep 1
cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}
fuse_unmount
fuse_mount
sleep 1

cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 666 ${send_file_mnt} || fail "should not fail chmod a file"
oldatime=`stat --format='%X' ${send_file_mnt}`
${txn} ${commit} || fail "should not fail to commit"

cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}
fuse_unmount
fuse_mount
sleep 1
cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}
fuse_unmount
fuse_mount
sleep 1

chmod 666 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${commit} || fail "should not fail to commit"

cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}
fuse_unmount
fuse_mount
${txn} ${commit} || fail "should not fail to commit"
${txn} ${commit} || fail "should not fail to commit"
sleep 1
${txn} ${commit} || fail "should not fail to commit"
${txn} ${commit} || fail "should not fail to commit"

cat ${send_file_mnt} || fail "should not fail reading a file"
chmod 646 ${send_file_mnt} || fail "should not fail chmod a file"
${txn} ${crash_now}


fuse_unmount
fuse_mount

newatime=`stat --format='%X' ${send_file_mnt}`
test "$oldatime" == "$newatime" || fail "access time not recovered"
newmode=`stat --format="%a" ${send_file_mnt}`
test "$newmode" == "666" || fail "access mode not recovered"

echo "replay pass"
fuse_unmount &>/dev/null
