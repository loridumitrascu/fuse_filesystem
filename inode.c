#define _DEFAULT_SOURCE
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"
#include "dentry.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "log.h"

void *get_inode_block_base_ptr()
{
    void *inode_table_base = nth_block_pointer(1); // the inode table is the 1st block (after the bitmaps)
    return inode_table_base;
}

int get_max_inode_number()
{
    return (BLOCK_SIZE - sizeof(int)) * 2 / sizeof(inode); // we have two blocks for the inode table
}

int get_first_free_inode_from_ibitmap()
{
    void *inode_bitmap_base = get_bitmap_inode_ptr();
    for (int i = 0; i < get_max_inode_number(); i++)
    {
        if (bit_map_get_bit(inode_bitmap_base, i) == 0)
            return i;
    }
    printf("Max inode number reached\n"); // TO DO - LOGFILE
    return -1;
}

inode *get_nth_inode(int index)
{
    return (inode *)(get_inode_block_base_ptr() + index * sizeof(inode));
}

inode *alloc_inode()
{
    int new_inode_number = get_first_free_inode_from_ibitmap();
    assert(new_inode_number >= 0); // fail if max number of inodes reached
    inode *new_inode = get_nth_inode(new_inode_number);

    new_inode->size = 0;
    new_inode->block_number = alloc_block();
    assert(new_inode->block_number >= 0); // fail if max number of blocks reached
    new_inode->inode_number = new_inode_number;
    new_inode->uid = getuid();
    new_inode->gid = getgid();
    new_inode->is_dir = 0; // we consider all inodes files at firs
    new_inode->ctime = time(NULL);
    new_inode->mtime = time(NULL);
    new_inode->atime = time(NULL);
    new_inode->nlink = 1;
    new_inode->number_of_blocks = 1;
    new_inode->mode = S_IFREG | 0644;

    void *inode_bitmap_base = get_bitmap_inode_ptr();
    bit_map_set_bit(inode_bitmap_base, new_inode_number, 1);

    return new_inode;
}

void free_inode(int inode_number)
{
    void *inode_bitmap_base = get_bitmap_inode_ptr();
    if (bit_map_get_bit(inode_bitmap_base, inode_number) == 0)
    {
        return;
    }
    // delete inode data from inode table
    inode *inode_to_delete = get_nth_inode(inode_number);
   
    // delete data from blocks
    truncate_down_to_size_for_inode(inode_number, 0); // it also calls the free_block function which sets all bits to zero in block bitmap for every freed block.It also memsets the blocks to 0

    // free the correspondent bit in the inode_bitmap
    bit_map_set_bit(inode_bitmap_base, inode_number, 0);

    // delete inode from inode_table
    memset(inode_to_delete, 0, sizeof(inode));
}

void truncate_to_size(int inode_number, int requested_size)
{
    inode *current_inode = get_nth_inode(inode_number);
    int inode_size = current_inode->size;
    if (inode_size > requested_size)
    {
        truncate_down_to_size_for_inode(inode_number, requested_size);
        return;
    }
    if (inode_size < requested_size)
    {
        truncate_up_to_size_for_inode(inode_number, requested_size);
        return;
    }
}

void truncate_up_to_size_for_inode(int inode_number, int requested_size)
{
    inode *current_inode = get_nth_inode(inode_number);
    int inode_size = current_inode->size;
    int block_start_number = current_inode->block_number;
    int current_block = get_last_block_extension_in_list(block_start_number);

    int remaining_size_curr_block = (BLOCK_SIZE - sizeof(int)) - (inode_size % (BLOCK_SIZE - sizeof(int)));
    int inode_size_new = inode_size + remaining_size_curr_block;
    int left_size_to_grow = requested_size - inode_size_new;
    int next_block;

    while (left_size_to_grow > 0)
    {
        left_size_to_grow -= (BLOCK_SIZE - sizeof(int));
        next_block = alloc_new_block_extension(current_block);
        // update number of blocks for inode
        current_inode->number_of_blocks++;
        current_block = next_block;
    }
    current_inode->size = requested_size;
}

void truncate_down_to_size_for_inode(int inode_number, int requested_size)
{
    inode *current_inode = get_nth_inode(inode_number);
    int current_block = current_inode->block_number;
    int first_block = current_block;
    int current_size = 0;
    int block_to_delete_index = current_block;
    if (requested_size <= (BLOCK_SIZE - sizeof(int)))
    {
        //we are requesting a size less than the first block
        void *current_block_base = nth_block_pointer(current_block);
        block_to_delete_index = get_next_block_index_number(current_block);
        memset(current_block_base + requested_size, 0, ((BLOCK_SIZE - sizeof(int)) - requested_size));
    }
    else
    {
        while (current_size < requested_size)
        {
            current_size += (BLOCK_SIZE - sizeof(int));
            int size_to_delete_from_block = requested_size - current_size;
            if (size_to_delete_from_block <= (BLOCK_SIZE - sizeof(int)))
            {
                void *current_block_base = nth_block_pointer(current_block);
                block_to_delete_index = get_next_block_index_number(current_block);
                reset_next_block_index_number(current_block);
                memset(current_block_base + size_to_delete_from_block, 0, ((BLOCK_SIZE - sizeof(int)) - size_to_delete_from_block));
                break;
            }
            else
            {
                current_block = get_next_block_index_number(current_block);
            }
        }
    }

    while (block_to_delete_index != -1)
    {
        int next_block_index = get_next_block_index_number(block_to_delete_index);
        free_block(block_to_delete_index);
        block_to_delete_index = next_block_index;
    }
    if(requested_size==0 && current_inode->inode_number!=0) 
    {
        free_block(first_block);
    }
    current_inode->size = requested_size;
}

void initialise_root()
{
    inode *root_inode = alloc_inode();
    root_inode->is_dir = 1;
    root_inode->nlink = 2;
    root_inode->mode = S_IFDIR | 0755;
    assert(root_inode->inode_number == 0);
    assert(root_inode->block_number == 3);
}

void update_root()
{
    inode *root_inode = get_nth_inode(0);
    root_inode->atime = time(NULL);
}