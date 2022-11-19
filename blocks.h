#ifndef _BLOCKS_H
#define _BLOCKS_H

int count_blocks_for_bytes(int nr_bytes); //return the number of blocks needed for n bytes file

void* nth_block_pointer(int n); //return the pointer to the starting zone of the block with index n

void* get_bitmap_datablock_ptr();
void* get_bitmap_inode_ptr();

int alloc_block(); //return the index of the new allocated block
void free_block(int n); //free the block with index n

#endif