#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <errno.h>

#include "disk_image.h"
#include "bitmap.h"
#include "utils.h"
#include "inode.h"
#include "blocks.h"
#include "dentry.h"
#include "log.h"

void disk_mount_the_filesystem(const char* disk_iso_path)
{
    //check if the disk_iso file exists. 
    int result = access(disk_iso_path,F_OK);
    if(result==ENOENT)
    {
        //if it doesn't exist, it_s an empty filesystem
        init_filesystem(disk_iso_path);
        log_message("Created the disk_iso\n");
    }
    else
    {
        if(result>0)
        {
            //if it exist, initialise the existing data
            //TO DO: call the remount function
            log_message("Created from the existing disk_iso\n");
        }
        else
        {
            log_message("Error at disk mounting...Disk_iso access error\n");
            return;
        }
    }
}

void init_filesystem(const char* disk_iso_path)
{
    blocks_base=init_disk_image(disk_iso_path);
    //blocks_base is the pointer to the first block. We mark it as used in the blocks bitmap
    void* blocks_bitmap=get_bitmap_datablock_ptr();
    bit_map_set_bit(blocks_bitmap,0,1);
    //mark the blocks for the inode table as used    
    bit_map_set_bit(blocks_bitmap,1,1);
    bit_map_set_bit(blocks_bitmap,2,1);
    //we initialise the root
    initialise_root();

}

//initialise the disk image that represents the FileSystem
void* init_disk_image(const char* disk_iso_path)
{
    void* disk_iso_base = NULL; //pointer to start of the mmapped file
    int fd=0; //disk_iso_descriptor
    ssize_t size = SYS_SIZE; //disk_iso size

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

void unmap_filesystem(void* disk_iso_base)
{
    //unmap the disk_iso 
    int ret_value = munmap(disk_iso_base, SYS_SIZE);
    assert(ret_value==0);
}

int disk_access(const char* entry_path)
{
    //check if the path is a valid and the file/dir exists
   int inode_number = get_file_inode_from_path(entry_path);
   if(inode_number<0)
        return -1;
    
    //update the timestamps for given inode
    inode* inode = get_nth_inode(inode_number);
    inode->atime=time(NULL);
    inode->ctime=time(NULL);

    return 0;
}

int disk_check_permissions(const char* entry_path,int mask)
{
    int inode_number = get_file_inode_from_path(entry_path);
    inode* inode = get_nth_inode(inode_number);
    if((inode->mode & mask)==mask)
    {
        return 0;
    }
    return -1;
}