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

touch mnt/file
oldatime=`stat --format='%X' mnt/file`
oldmtime=`stat --format='%Y' mnt/file`
oldctime=`stat --format='%Z' mnt/file`

sleep 1
echo "time is the fire in which we burn" > mnt/file
newmtime=`stat --format='%Y' mnt/file`
if [ "$oldmtime" == "$newmtime" ]; then
	fail "mtime $newmtime was not updated from $oldmtime"
else
	ok "mtime updated to $newmtime from $oldmtime"
fi

sleep 1
cat mnt/file
newatime=`stat --format='%Y' mnt/file`
if [ "$oldatime" == "$newatime" ]; then
	fail "atime $newatime was not updated from $oldatime"
else
	ok "atime updated to $newatime from $oldatime"
fi

sleep 1
chmod a+rx mnt/file
newctime=`stat --format='%Z' mnt/file`
if [ "$oldctime" == "$newctime" ]; then
	fail "ctime $newctime was not updated from $oldctime"
else
	ok "ctime updated to $newctime from $oldctime"
fi

${txn} ${commit} || fail "should not crash here"

${txn} ${crash_now}

fuse_unmount
fuse_mount

newmtime2=`stat --format='%Y' mnt/file`
if [ "$newmtime" == "$newmtime2" ]; then
	ok "mtime consistent after crash"
else
	fail "mtime not consistent after crash"
fi

newatime2=`stat --format='%Y' mnt/file`
if [ "$newatime" == "$newatime2" ]; then
	ok "atime consistent after crash"
else
	fail "atime not consistent after crash"
fi

newctime2=`stat --format='%Z' mnt/file`
if [ "$newctime" == "$newctime2" ]; then
	ok "ctime consistent after crash"
else
	fail "ctime not consistent after crash"
fi

sleep 1
echo "time is the fire in which we burn" > mnt/file
cat mnt/file
chmod a+rx mnt/file

${txn} ${crash_now}

fuse_unmount
fuse_mount

newmtime2=`stat --format='%Y' mnt/file`
if [ "$newmtime" == "$newmtime2" ]; then
	ok "mtime consistent after crash"
else
	fail "mtime not consistent after crash"
fi

newatime2=`stat --format='%Y' mnt/file`
if [ "$newatime" == "$newatime2" ]; then
	ok "atime consistent after crash"
else
	fail "atime not consistent after crash"
fi

newctime2=`stat --format='%Z' mnt/file`
if [ "$newctime" == "$newctime2" ]; then
	ok "ctime consistent after crash"
else
	fail "ctime not consistent after crash"
fi

echo "time crash pass"
fuse_unmount &>/dev/null
