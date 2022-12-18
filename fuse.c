#define _XOPEN_SOURCE 700
#define FUSE_USE_VERSION 29
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fuse.h>
#include <errno.h>

#include "disk_image.h"
#include "utils.h"
#include "log.h"
#include "blocks.h"
#include "bitmap.h"
#include "inode.h"
#include "dentry.h"

void *fs_init(struct fuse_conn_info *conn)
{   
    //function that initialise the file system
    disk_mount_the_filesystem("disk_iso");

    log_message("Mounted the filesystem\n");
}

int fs_access(const char* path,int mask)
{
    //function that checks the existance of the inode for a dir or a file
    //it updates the timestamps
    int ret_value = disk_access(path);
    if(ret_value==-1)
    {
        return -ENOENT;
    }
    //check if the user has the correct permissions to access the file
    if(mask!=F_OK)
    {
        int ret_value = disk_check_permissions(path,mask);
        if(ret_value==-1)
        {
            return -EACCES;
        }
    }
    return 0;
}

int fs_getattr(const char *path, struct stat *stbuf)
{
    int result = disk_get_attributes_from_path(path, stbuf);
    //we add +1 to display the correct inode because in our filesystem inodes start from 0. 
    //In any other filesystem, they start from 1.
    stbuf->st_ino=stbuf->st_ino+1;
    return result;
}

int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int result = disk_mknod(path, mode);
    return result;
}

int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
    int result = disk_readdir(path,buf,filler);
    return result;
}
static struct fuse_operations operations =
{
        .getattr = fs_getattr,
        .init = fs_init,
        .access   = fs_access,
        .mknod    = fs_mknod,
        .readdir = fs_readdir,
};

int main(int argc,char* argv[])
{
    /*
    TO DO:: trebuie facute la final
    unmap_filesystem(blocks_base);
    remove("disk_iso");
    */

    FILE *fis = log_open();
    fuse_main(argc, argv, &operations, NULL);
    log_close(fis);
    return 0;
}

