#ifndef _BLOCKS_H
#define _BLOCKS_H

int count_blocks_for_bytes(int nr_bytes); //return the number of blocks needed for n bytes file

void* nth_block_pointer(int n); //return the pointer to the starting zone of the block with index n

void* get_bitmap_datablock_ptr();
void* get_bitmap_inode_ptr();

int alloc_block(); //return the index of the new allocated block
void free_block(int n); //free the block with index n

int get_next_block_index_number(int current_block);

void set_next_block_index_number(int current_block,int next_block_inode);

void reset_next_block_index_number(int current_block);

int alloc_new_block_extension(int current_block);

int get_last_block_extension_in_list(int start_block);

#endif