#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include "disk_image.h"
#include "bitmap.h"

void init_filesystem(const char* disk_iso_path)
{
    void* fs_base=init_disk_image(disk_iso_path);
    //void* blocks_bitmap=get_bitmap_datablock_ptr();
    //bit_map_set_bit(blocks_bitmap,0,1);
    
    //TO DO: initialise inodes 
}

//initialise the disk image that represents the FileSystem
void* init_disk_image(const char* disk_iso_path)
{
    void* disk_iso_base = NULL; //pointer to start of the mmapped file
    int fd=0; //disk_iso_descriptor
    ssize_t size = FileSystemSize; //disk_iso size

    //open or create the disk_iso
    fd = open(disk_iso_path,O_RDWR|O_CREAT,0644);
    assert(fd>=0);

    //ensure that the disk_iso is exactly FileSysytemSize
    int ret_value = ftruncate(fd,size);
    
    assert(ret_value==0);

    //mmap the disk_iso for writing and reading
    //MAP_SHARED BECAUSE WE WANT TO ENSURE THAT DATA WRITTEN IS PUT ON THE DISK (SHARE THE FILE WITH OTHER SO PROCESSES)
    disk_iso_base = mmap (NULL,size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0); 
    assert(disk_iso_base!=((void *) -1));

    return disk_iso_base;
}
