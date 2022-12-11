#define _XOPEN_SOURCE 700
#define FUSE_USE_VERSION 29
#define _BSD_SOURCE
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

static struct fuse_operations operations =
    {
        //.getattr = fs_getattr,
        .init = fs_init,
        .access   = fs_access,
        //.getattr  = nufs_getattr,
        //.mknod    = nufs_mknod,
        //.readdir = fs_readdir,
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

