BUILD := build

FUSE_CFLAGS := -DFUSE_USE_VERSION=26 $(shell pkg-config --cflags fuse 2>/dev/null)
FUSE_CFLAGS += -I ../fuse/include/fuse -D_FILE_OFFSET_BITS=64
FUSE_LDFLAGS := $(shell pkg-config --libs fuse 2>/dev/null)
FUSE_LDFLAGS += -L ../fuse/ -lfuse -pthread

EXTRA_CFLAGS ?=

CC	:= gcc
CFLAGS	:= -MD -O1 -g -c $(FUSE_CFLAGS) $(EXTRA_CFLAGS)

FSDRIVER_OBJS	:=	log.o \
			crashmod.o \
			bitmap.o \
			dir.o \
			disk_map.o \
			inode.o \
			panic.o \
			fsdriver.o
FSDRIVER_OBJS	:= $(patsubst %.o,$(BUILD)/%.o,$(FSDRIVER_OBJS))

all: $(BUILD)/fsdriver $(BUILD)/fsformat send_ioctl
	@:


$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/fsformat: $(BUILD)/fsformat.o
	$(CC) -o $@ $(BUILD)/fsformat.o

$(BUILD)/fsdriver: $(FSDRIVER_OBJS)
	$(CC) -o $@ $(FSDRIVER_OBJS) $(FUSE_LDFLAGS)

send_ioctl: send_ioctl.c
	$(CC) -o $@ $<

-include $(BUILD)/*.d

clean:
	rm -rf $(BUILD)
	rm -f send_ioctl

grade: always
	@echo $(MAKE) clean
	@$(MAKE) -s --no-print-directory clean
	@test -d mnt || mkdir mnt
	./grade-lab

always:
	-@fusermount -u mnt

.PHONY: clean always

LAB_NUM=7
LAB_NAME= lab$(LAB_NUM)

handin:
	$(V)/bin/bash ./check-lab.sh . || false
	$(MAKE) LAB=$(LAB_NUM) LAB_NAME=$(LAB_NAME) -C .. handin
