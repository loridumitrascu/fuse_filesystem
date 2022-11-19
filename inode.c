#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

void* get_inode_block_base_ptr()
{
    void* inode_table_base=nth_block_pointer(1);//the inode table is the 1st block (after the bitmaps)
    return inode_table_base;
}

int get_max_inode_number()
{
    return (BLOCK_SIZE-sizeof(int))*2/sizeof(inode); //we have two blocks for the inode table
}

int get_first_free_inode_from_ibitmap()
{
    void *inode_bitmap_base=get_bitmap_inode_ptr();
    for(int i=1;i<get_max_inode_number();i++)
    {
        if(bit_map_get_bit(inode_bitmap_base,i)==0)
            return i;
    }
    printf("Max inode number reached\n"); //TO DO - LOGFILE
    return -1;
}

inode* get_nth_inode(int index)
{
    return (inode*)(get_inode_block_base_ptr()+index*sizeof(inode));
}

inode* alloc_inode()
{
    int new_inode_number=get_first_free_inode_from_ibitmap();
    assert(new_inode_number>=0); //fail if max number of inodes reached
    inode* new_inode=get_nth_inode(new_inode_number);

    new_inode->dentries_size=0;
    new_inode->block_number=alloc_block();
    assert(new_inode->block_number>=0); //fail if max number of blocks reached
    new_inode->inode_number=new_inode_number;
    new_inode->uid=-1;
    new_inode->gid=-1;
    new_inode->ctime=time(NULL);
    new_inode->mtime=time(NULL);
    new_inode->atime=time(NULL);

    void *inode_bitmap_base=get_bitmap_inode_ptr();
    bit_map_set_bit(inode_bitmap_base,new_inode_number,1);

    return new_inode;
}

void free_inode(int index)
{
    inode* inode_to_delete=get_nth_inode(index);
    int inode_number_to_delete=inode_to_delete->block_number;

    free_block(inode_number_to_delete); //free_block automatically sets the corespondent bit in datablock_bitmap to 0

    void *inode_bitmap_base=get_bitmap_inode_ptr();
    bit_map_set_bit(inode_bitmap_base,inode_number_to_delete,0);
}