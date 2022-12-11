#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "blocks.h"
#include "bitmap.h"
#include "utils.h"

void* blocks_base=0;

int count_blocks_for_bytes(int nr_bytes)
{
    int effective_size=BLOCK_SIZE-sizeof(int);   //an int is reserved at the end of each block as a pointer for the next extended block if needed.
    int effective_number=nr_bytes/effective_size;
    int rest=nr_bytes%effective_size;
    if(rest!=0)
        effective_number++;
    return effective_number;
}

void* nth_block_pointer(int n)
{
    assert(n!=-1);
    return blocks_base+n*BLOCK_SIZE;
}

void* get_bitmap_datablock_ptr()
{
    return nth_block_pointer(0);
}

void* get_bitmap_inode_ptr()
{
    return get_bitmap_datablock_ptr()+BLOCK_COUNT/8; //the inode bitmap starts right after the datablock bitmap 
                                                   //which has a fixed size equal to total numer of blocks
}

int alloc_block()
{
    //firstly we find the first free block
    void* datab_ptr=get_bitmap_datablock_ptr();
    int i;
    for(i=3;i<BLOCK_COUNT;i++) //we start the search with the index 1 block because the 0 block is reserved for our bitmaps
    {
        if(bit_map_get_bit(datab_ptr,i)==0)
        {
            bit_map_set_bit(datab_ptr,i,1);

            //Initialise the value for the next block index with -1 (it's not set in the beginning)
            set_next_block_index_number(i,-1);
            return i;
        }
    }
    printf("No block available!\n");
    return -1;
}

void free_block(int n)
{
    void* datab_ptr=get_bitmap_datablock_ptr();
    bit_map_set_bit(datab_ptr,n,0);
    memset(nth_block_pointer(n),0,BLOCK_SIZE); //making sure that the block is clean
}

int get_next_block_index_number(int current_block)
{
    void *current_block_base = nth_block_pointer(current_block);
    int* res=current_block_base+BLOCK_SIZE-sizeof(int);
    return *res;
}

void set_next_block_index_number(int current_block,int next_block_index)
{
    void *current_block_base = nth_block_pointer(current_block);
    int* value_pointer= current_block_base+BLOCK_SIZE-sizeof(int);
    *value_pointer= next_block_index;
}

void reset_next_block_index_number(int current_block)
{
    void *current_block_base = nth_block_pointer(current_block);
    int* value_pointer= current_block_base+BLOCK_SIZE-sizeof(int);
    *value_pointer=-1;
}

int alloc_new_block_extension(int current_block)
{
    int new_block_index= alloc_block();
    assert(new_block_index!=-1);
    set_next_block_index_number(current_block,new_block_index);
    return new_block_index;
}

int get_last_block_extension_in_list(int start_block)
{
    assert(start_block!=-1);
    int next_block_index=get_next_block_index_number(start_block);
    if(next_block_index==-1)
    {
        return start_block;
    }

    int previous_block_index;
    while(next_block_index!=-1)
    {
        previous_block_index=next_block_index;
        next_block_index=get_next_block_index_number(next_block_index);
    }
    return previous_block_index;
}