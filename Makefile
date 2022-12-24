CC     = gcc
CFLAGS = -g `pkg-config fuse --cflags` -Wall -D_FILE_OFFSET_BITS=64 
LFLAGS = -lfuse -I/usr/include/fuse, -pthread
OBJS = bitmap.o blocks.o dentry.o disk_image.o fuse.o inode.o log.o

all: fuse_fs

fuse_fs: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LFLAGS)
	@echo 'To mount: ./run.sh'

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f *.o filesys.log fuse_fs $(disk_path)
	@if [ -d $(mount_point) ]; then rmdir $(mount_point); fi
	@echo 'Clean completed'

mount: checkdir
	@./fuse_fs -f -o use_ino $(mount_point) $(disk_path)	

debugmount:checkdir
	@./fuse_fs -d -f -o use_ino $(mount_point) $(disk_path)

umount:
	@sudo umount -f $(mount_point)
	@echo 'Unmounted the filesystem'

checkdir:
	@if [ -d "./$(mount_point)" ]; then echo "mountpoint exists"; else mkdir ./$(mount_point); fi

.PHONY: clean all mount umount checkdir debugmount