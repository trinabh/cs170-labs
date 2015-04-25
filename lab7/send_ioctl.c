#include <fcntl.h>
 
 /* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int fd;
	size_t size;
	int i;
	int ret;

	if (argc < 3) {
		printf("Usage: %s [device] [ioctl]\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Could not open file %s\n", argv[1]);
		exit(2);
	}

	i = atoi(argv[2]);
	if (!i) {
		printf("Invalid ioctl: %s (%d)\n", argv[2], i);
		exit(3);
	}

	ret = ioctl(fd, i, &size);
	if (ret) {
		printf("ioctl error: %d\n", ret);
		exit(4);
	}

	close(fd);
	
	return 0;
}

