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

void disk_mount_the_filesystem(const char *disk_iso_path)
{
    // check if the disk_iso file exists.
    int result = access(disk_iso_path, F_OK);
    if (result < 0)
    {
        if (errno == ENOENT)
        {
            // if it doesn't exist, it_s an empty filesystem
            init_filesystem(disk_iso_path);
            log_message("Created the disk_iso\n");
        }
        else
        {
            log_message("Error at disk mounting...Disk_iso access error,%s %d\n", disk_iso_path, result);
        }
        return;
    }
    // if it exist, initialise the existing data
    // TO DO: call the remount function
    remount_disk_image(disk_iso_path);
    log_message("Created from the existing disk_iso\n");
}

void init_filesystem(const char *disk_iso_path)
{
    blocks_base = init_disk_image(disk_iso_path);
    // blocks_base is the pointer to the first block. We mark it as used in the blocks bitmap
    void *blocks_bitmap = get_bitmap_datablock_ptr();
    bit_map_set_bit(blocks_bitmap, 0, 1);
    // mark the blocks for the inode table as used
    bit_map_set_bit(blocks_bitmap, 1, 1);
    bit_map_set_bit(blocks_bitmap, 2, 1);
    // we initialise the root
    initialise_root();
}

void remount_disk_image(const char *disk_iso_path)
{
    blocks_base = init_disk_image(disk_iso_path);
    update_root();
}

// initialise the disk image that represents the FileSystem
void *init_disk_image(const char *disk_iso_path)
{
    void *disk_iso_base = NULL; // pointer to start of the mmapped file
    int fd = 0;                 // disk_iso_descriptor
    ssize_t size = SYS_SIZE;    // disk_iso size

    // open or create the disk_iso
    fd = open(disk_iso_path, O_RDWR | O_CREAT, 0644);
    assert(fd >= 0);

    // ensure that the disk_iso is exactly FileSysytemSize
    int ret_value = ftruncate(fd, size);

    assert(ret_value == 0);

    // mmap the disk_iso for writing and reading
    // MAP_SHARED BECAUSE WE WANT TO ENSURE THAT DATA WRITTEN IS PUT ON THE DISK (SHARE THE FILE WITH OTHER SO PROCESSES)
    disk_iso_base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(disk_iso_base != ((void *)-1));

    //we close the file descriptor. We no longer need it
    int ret = close(fd);
    assert(ret >= 0);

    return disk_iso_base;
}

void unmap_filesystem(void *disk_iso_base)
{
    // unmap the disk_iso
    int ret_value = munmap(disk_iso_base, SYS_SIZE);
    assert(ret_value == 0);
}

int disk_access(const char *entry_path)
{
    // check if the path is a valid and the file/dir exists
    log_message("%s\n", entry_path);
    int inode_number = get_file_inode_from_path(entry_path);
    if (inode_number < 0)
        return -1;

    // update the timestamps for given inode
    log_message("%d\n", inode_number);
    inode *inode = get_nth_inode(inode_number);
    inode->atime = time(NULL);
    inode->ctime = time(NULL);
    return 0;
}

int disk_check_permissions(const char *entry_path, int mask)
{
    int inode_number = get_file_inode_from_path(entry_path);
    inode *inode = get_nth_inode(inode_number);
    if ((inode->mode & mask) == mask)
    {
        return 0;
    }
    return -1;
}

int disk_get_attributes_from_path(const char *path, struct stat *stbuf)
{
    int inode_number = get_file_inode_from_path(path);

    int result = disk_get_attributes_from_inode(inode_number, stbuf);

    return result;
}

int disk_get_attributes_from_inode(int inode_number, struct stat *stbuf)
{
    if (inode_number < 0)
    {
        return -ENOENT;
    }
    inode *inode = get_nth_inode(inode_number);

    stbuf->st_atime = inode->atime;
    stbuf->st_ctime = inode->ctime;
    stbuf->st_mtime = inode->mtime;

    stbuf->st_uid = inode->uid;
    stbuf->st_gid = inode->gid;

    stbuf->st_mode = inode->mode;
    stbuf->st_nlink = inode->nlink;
    stbuf->st_size = inode->size;
    stbuf->st_blksize = BLOCK_SIZE;

    stbuf->st_ino = inode->inode_number;
    return 0;
}

int disk_mknod(const char *path, mode_t mode)
{
    char *parent_path = malloc(sizeof(char) * MAX_PATH);
    char *child_name = malloc(sizeof(char) * MAX_FILENAME);
    if (!parent_path || !child_name)
    {
        log_message("mknod: Malloc error!\n");
        return -1;
    }

    get_parent_path_and_child_name(path, parent_path, child_name);
    int parent_inode_number = get_file_inode_from_path(parent_path);
    if (parent_inode_number < 0)
    {
        log_message("mknod: parent_ino_number not found!\n");
        return -1;
    }

    inode *child_inode = alloc_inode();
    if (!child_inode)
    {
        log_message("mknod: child_inode not correctly allocated!\n");
        return -1;
    }

    child_inode->mode = mode;
    int child_inode_number = child_inode->inode_number;

    add_dir_to_inode_dentries(parent_inode_number, child_name, child_inode_number);

    free(parent_path);
    free(child_name);

    return 0;
}

int disk_change_utimens(const char *path,const struct timespec times[2])
{
    int inode_number = get_file_inode_from_path(path);
    if (inode_number < 0)
        return -ENOENT;
        
    inode *inode = get_nth_inode(inode_number);
    inode->atime = times[0].tv_sec;
    inode->mtime = times[1].tv_sec;

    return 0;
}


int disk_readdir(const char *path, void *buf, fuse_fill_dir_t filler)
{
    int inode_number = get_file_inode_from_path(path);
    if (inode_number < 0)
    {
        return -ENOENT;
    }
    inode *inode = get_nth_inode(inode_number);

    if (inode->is_dir == 0)
    {
        return -ENOENT;
    }

    void *dir_dentry_block = nth_block_pointer(inode->block_number);
    int dentries_number = inode->size / sizeof(dentry);
    dentry *dentries_base = (dentry *)dir_dentry_block;

    // define a structure st for file/dir attributes
    // we fill the buffer with the dentries and their st
    struct stat st;
    for (int i = 0; i < dentries_number; i++)
    {
        int fill_st = disk_get_attributes_from_inode(dentries_base[i].inode_number, &st);
        if (fill_st < 0)
            return -ENOENT;
        filler(buf, dentries_base[i].name, &st, 0);
    }

    // we fill the buffer for the . and .. entries
    // for the .
    int fill_st = disk_get_attributes_from_inode(inode_number, &st);
    if (fill_st < 0)
        return -ENOENT;
    filler(buf, ".", &st, 0);

    // for the ..
    if (strcmp(path, "/") == 0)
    {
        filler(buf, "..", &st, 0);
    }
    else
    {
        char *parent_path = malloc(sizeof(char) * MAX_PATH);
        char *child_name = malloc(sizeof(char) * MAX_FILENAME);
        if (!parent_path || !child_name)
        {
            log_message("mknod: Malloc error!\n");
            exit(EXIT_FAILURE);
        }
        get_parent_path_and_child_name(path, parent_path, child_name);

        int parent_inode_number = get_file_inode_from_path(parent_path);
        if (parent_inode_number < 0)
        {
            log_message("mknod: parent_ino_number not found!\n");
            return -ENOENT;
        }

        int fill_st = disk_get_attributes_from_inode(parent_inode_number, &st);
        filler(buf, "..", &st, 0);

        free(parent_path);
        free(child_name);
    }
    // update timestamps
    inode->atime = time(NULL);

    return 0;
}