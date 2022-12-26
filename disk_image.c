#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "disk_image.h"
#include "bitmap.h"
#include "utils.h"
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

    // we close the file descriptor. We no longer need it
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
    stbuf->st_blocks = inode->number_of_blocks;

    stbuf->st_ino = inode->inode_number;
    return 0;
}

int disk_mknod(const char *path, mode_t mode)
{
    // makes a regular file
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
        return -ENOENT;
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

int disk_chmod(const char *path, mode_t mode)
{
    int inode_number = get_file_inode_from_path(path);
    if (inode_number < 0)
    {
        log_message("Invalid path for chmod\n");
        return -ENOENT;
    }
    inode *inode = get_nth_inode(inode_number);
    inode->mode = mode;

    // update timestamps
    inode->atime = time(NULL);
    inode->mtime = time(NULL);
    return 0;
}

int disk_rename(const char *from, const char *to)
{
    // check if source is valid
    int inode_number = get_file_inode_from_path(from);
    if (inode_number < 0)
    {
        log_message("Source for rename function does not exist!\n");
        return -ENOENT;
    }

    // check if destination already exists
    inode_number = get_file_inode_from_path(to);
    if (inode_number >= 0)
    {
        log_message("Destination for rename function already exists!\n");
        return -EEXIST;
    }

    int res = disk_link(from, to);
    if (res != 0)
    {
        log_message("Error while linking for rename!\n");
        return -ENOENT;
    }

    res = disk_unlink(from);
    if (res != 0)
    {
        log_message("Error while unlinking for rename!\n");
        return -ENOENT;
    }

    return 0;
}

int disk_truncate(const char *path, off_t size)
{
    int inode_number = get_file_inode_from_path(path);
    if(inode_number<0)
    {
        return -ENOENT;
    }

    truncate_to_size(inode_number, size);

    inode *inode = get_nth_inode(inode_number);

    //update timestamps
    inode->atime = time(NULL);
    inode->mtime = time(NULL);
    
    return 0;
}

int disk_link(const char *from, const char *to)
{
    // Creates a hardlink
    int source_inode_number = get_file_inode_from_path(from);
    if (source_inode_number < 0)
    {
        log_message("Invalid source for link operation\n");
        return -ENOENT;
    }

    int destination_inode = get_file_inode_from_path(to);
    if (destination_inode > 0)
    {
        log_message("Destination already exists for link operation\n");
        return -EEXIST;
    }

    char parent_dest_path[MAX_PATH], dest_name[MAX_FILENAME];
    get_parent_path_and_child_name(to, parent_dest_path, dest_name);

    int parent_dest_inode_number = get_file_inode_from_path(parent_dest_path);
    if (parent_dest_inode_number < 0)
    {
        log_message("Impossible to reach destination\n");
        return -ENOENT;
    }
    // update source's number of links
    inode *source_inode = get_nth_inode(source_inode_number);
    source_inode->nlink++;
    // add the source inode to destinantion
    add_dir_to_inode_dentries(parent_dest_inode_number, dest_name, source_inode_number);

    // update timestamps
    source_inode->atime = time(NULL);
    source_inode->mtime = time(NULL);

    return 0;
}

int disk_mkdir(const char *path, mode_t mode)
{
    // A directory is a file with 2 nlinks and mode S_IFDIR
    int result = disk_mknod(path, mode);
    if (result < 0)
        return -1;

    int inode_number = get_file_inode_from_path(path);
    inode *inode = get_nth_inode(inode_number);
    inode->is_dir = 1;
    inode->mode = mode | S_IFDIR;
    inode->nlink = 2;
    return 0;
}

int disk_change_utimens(const char *path, const struct timespec times[2])
{
    // change the atime and mtime
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

int write_data_in_file(inode *file, const char *buf, size_t size, off_t offset)
{
    // grow file size if necessary
    int requested_size = offset + size;
    if (requested_size > file->size)
        truncate_up_to_size_for_inode(file->inode_number, requested_size);

    // get the first file's block we need to write to
    int first_block_count = (int)offset / (BLOCK_SIZE - sizeof(int));
    int first_block_index = get_nth_block_in_list(file->block_number, first_block_count);
    if (first_block_index < 0)
    {
        return -ENOENT;
    }
    void *current_block = nth_block_pointer(first_block_index);

    // get starting point in current_block
    int offset_in_block = ((int)offset) % (BLOCK_SIZE - sizeof(int));
    void *start_point = current_block + offset_in_block;

    // begin writting to file
    int is_first_block = 1;
    int remaining_size = size;
    int buf_offset = 0;
    int current_block_index = first_block_index;
    while (remaining_size > 0)
    {
        int available_block_size;
        if (is_first_block == 1)
        {
            available_block_size = BLOCK_SIZE - sizeof(int) - offset_in_block;
            is_first_block = 0;
        }
        else
        {
            available_block_size = BLOCK_SIZE - sizeof(int);
        }

        int length = remaining_size > available_block_size ? available_block_size : remaining_size;

        strncpy((char *)start_point, buf + buf_offset, length);

        remaining_size -= available_block_size;
        buf_offset += length;

        // update the block we need to write to
        int next_block_index = get_next_block_index_number(current_block_index);
        current_block_index = next_block_index;
        if (next_block_index == -1 && remaining_size > 0)
        {
            return -ENOENT;
        }
        if (next_block_index != -1)
            start_point = nth_block_pointer(next_block_index);
    }
    return 0;
}

int disk_write(const char *path, const char *buf, size_t size, off_t offset)
{
    int inode_number = get_file_inode_from_path(path);
    if (inode_number < 0)
    {
        return -ENOENT;
    }
    inode *file_inode = get_nth_inode(inode_number);

    if (file_inode->is_dir == 1)
    {
        return -ENOENT;
    }

    // write data into file
    int ret_value = write_data_in_file(file_inode, buf, size, offset);
    if (ret_value < 0)
    {
        return -ENOENT;
    }

    // update time stamps
    file_inode->atime = time(NULL);
    file_inode->mtime = time(NULL);

    return size;
}

int read_data_from_file(inode *file, char *buf, size_t size, off_t offset)
{
    // get the first file's block we need to read from
    int first_block_count = (int)offset / (BLOCK_SIZE - sizeof(int));
    int first_block_index = get_nth_block_in_list(file->block_number, first_block_count);
    if (first_block_index < 0)
    {
        return -ENOENT;
    }
    void *current_block = nth_block_pointer(first_block_index);

    // get starting point in current_block
    int offset_in_block = ((int)offset) % (BLOCK_SIZE - sizeof(int));
    void *start_point = current_block + offset_in_block;

    // begin reading from file
    int is_first_block = 1;
    int remaining_size = size;
    int buf_offset = 0;
    int current_block_index = first_block_index;

    // check to see if we read past EOF
    if (file->size < offset + remaining_size)
    {
        // we shrink the remaining_size until EOF
        remaining_size = file->size - offset;
    }

    while (remaining_size > 0)
    {
        int available_block_size;
        if (is_first_block == 1)
        {
            available_block_size = BLOCK_SIZE - sizeof(int) - offset_in_block;
            is_first_block = 0;
        }
        else
        {
            available_block_size = BLOCK_SIZE - sizeof(int);
        }

        int length = remaining_size > available_block_size ? available_block_size : remaining_size;

        strncpy(buf + buf_offset, (char *)start_point, length);

        remaining_size -= available_block_size;
        buf_offset += length;

        // update the block we need to read from
        int next_block_index = get_next_block_index_number(current_block_index);
        current_block_index = next_block_index;
        if (remaining_size > 0 && next_block_index == -1)
        {
            // nothing left to read from file.
            return 0;
        }
        if (next_block_index != -1)
            start_point = nth_block_pointer(next_block_index);
    }
    return 0;
}

int disk_read(const char *path, char *buf, size_t size, off_t offset)
{
    int inode_number = get_file_inode_from_path(path);
    if (inode_number < 0)
    {
        return -ENOENT;
    }
    inode *file_inode = get_nth_inode(inode_number);

    if (file_inode->is_dir == 1)
    {
        return -ENOENT;
    }

    // read data from file
    int ret_value = read_data_from_file(file_inode, buf, size, offset);
    if (ret_value < 0)
    {
        return ret_value;
    }
    // update time stamps
    file_inode->atime = time(NULL);

    return size;
}

int disk_unlink(const char *path)
{
    int child_inode_number = get_file_inode_from_path(path);
    if (child_inode_number < 0)
    {
        log_message("Inexistent file for unlink!\n");
        return -ENOENT;
    }

    char parent_path[MAX_PATH];
    char child_name[MAX_FILENAME];
    get_parent_path_and_child_name(path, parent_path, child_name);
    int parent_inode_number = get_file_inode_from_path(parent_path);
    if (parent_inode_number < 0)
    {
        log_message("Invalid parent for %s unlink: inode not found for %s\n", path, parent_path);
        return -ENOENT;
    }

    inode *child_inode = get_nth_inode(child_inode_number);

    // update time stamps
    if (child_inode->nlink > 1)
    {
        child_inode->atime = time(NULL);
        child_inode->mtime = time(NULL);
    }

    // delete the file or decrements its number of hardlinks
    int result = delete_file_from_inode_dentries(parent_inode_number, child_name);
    if (result < 0)
    {
        return -ENOENT;
    }

    return 0;
}

int disk_rmdir(const char *path)
{
    int dir_inode_number = get_file_inode_from_path(path);
    if (dir_inode_number < 0)
    {
        return -ENOENT;
    }

    inode *dir_inode = get_nth_inode(dir_inode_number);

    // rmdir deletes a dir only if it's empty
    if (dir_inode->size != 0)
    {
        // dir not empty
        return -ENOTEMPTY;
    }

    char parent_path[MAX_PATH];
    char child_name[MAX_FILENAME];
    get_parent_path_and_child_name(path, parent_path, child_name);
    int parent_inode_number = get_file_inode_from_path(parent_path);
    if (parent_inode_number < 0)
    {
        return -ENOENT;
    }
    int result = delete_dir_from_inode_dentries(parent_inode_number, child_name);
    if (result < 0)
    {
        return -ENOENT;
    }

    // update timestamps for parent
    inode *parent_dir = get_nth_inode(parent_inode_number);
    parent_dir->atime = time(NULL);
    parent_dir->mtime = time(NULL);

    return 0;
}