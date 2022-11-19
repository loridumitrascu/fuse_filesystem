#include <stdio.h>
#include <string.h>
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