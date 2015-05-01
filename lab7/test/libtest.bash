fail() {
	printf '\e[31mfail\e[m: %s\n' "$@"
	fusermount -u mnt 1>/dev/null 2>/dev/null
	exit 1
}

ok() {
	printf 'ok: %s\n' "$@"
}

generate_test_msg() {
	# Keep synced with static char *msg in fsdriver.c.
	printf 'This is a rather uninteresting message.\n\n' > build/msg
}

fuse_unmount() {
    ntry=3
    while [ ${ntry} -gt 0 ]
    do
        warn=`fusermount -u mnt 2>&1`
        notfnd=`echo "$warn" | grep 'not found in /etc/mtab'`
        notfndcims=`echo "$warn" | grep 'Invalid argument'`
        nosuchfile=`echo "$warn" | grep 'No such file or directory'`
        if [ -n "$warn" -a -z "$notfnd" -a -z "$notfndcims" -a -z "$nosuchfile" ]
        then
            echo "${warn}"
            sleep 0.2
            ntry=$((${ntry}-1))
        else
            break
        fi
    done
    if [ ${ntry} -eq 0 ]
    then
        fail "unable to unmount"
    fi
}

recreate_mnt() {
	rm -rf mnt || fail "couldn't remove mnt"
	mkdir mnt || fail "couldn't make mnt"
	chmod a+rwx mnt || fail "couldn't change mnt permissions"
}

make_fsimg() {
    test -x build/fsformat || (make || fail "please use 'make' to build first")
	build/fsformat testfs.img 32768 $@ || fail "couldn't make test image"
	#build/fsformat testfs.img 2048 $@ || fail "couldn't make test image"
}

fuse_mount() {
	build/fsdriver testfs.img mnt $@ &>/dev/null || fail "couldn't mount test image"
}
