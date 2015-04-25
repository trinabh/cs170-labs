#!/bin/bash

. test/libtest.bash

fuse_unmount
generate_test_msg
make_fsimg build/msg
echo "makeimage pass"
