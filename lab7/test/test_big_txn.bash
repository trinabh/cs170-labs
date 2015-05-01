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

last_it=
n_iterations=128
n_files=64

for it in $(seq 0 $((${n_iterations}-1)))
do
        last_it=${it}
        for fn in $(seq 0 $((${n_files}-1)))
        do
                echo -n "${fn}+${it}" >> mnt/hello${fn} || fail "should not crash here ${it}, ${fn}"
        done
done

${txn} ${commit} || fail "should not crash here"

${txn} ${crash_now}

fuse_unmount
fuse_mount

for fn in $(seq 0 $((${n_files}-1)))
do
        correct_content=
        for it in $(seq 0 ${last_it})
        do
                correct_content="${correct_content}${fn}+${it}"
        done
        content=`cat mnt/hello${fn}`
        if [ "${content}" != "${correct_content}" ]
        then
                echo "file: mnt/hello${fn}"
                echo "correct:"
                echo ${correct_content}
                echo "bad:"
                echo ${content}
                fail "content not correct"
        fi
done

echo "big txn pass"
fuse_unmount &>/dev/null
