#! /bin/bash

echo "You only need this on CIMS machines, press <ENTER> to continue, or <Ctrl>-C to cancel"
read $opt
pwd=$(pwd)
imagedir=../qemu-image
imagename=cs202qemu-lab6.qcow2
fusedir=../
yj682lab6path=/home/yj682/cs202-lab6
yj682imagepath=$yj682lab6path/cs202snap.qcow2
yj682fusepath=$yj682lab6path/fuse.tgz
[ -d $imagedir ] || mkdir $imagedir 
[ -d $fusedir ] || mkdir -p $fusedir

echo "Copying fuse library...."
rsync -a $yj682fusepath $fusedir && cd $fusedir && tar xf fuse.tgz && cd $pwd

rm ../fuse/libfuse.so 2> /dev/null
ln -s /lib64/libfuse.so.2 ../fuse/libfuse.so

echo "Installation successful"
