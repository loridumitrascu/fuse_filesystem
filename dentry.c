#include "dentry.h"
#include <string.h>
#include <string.h>
#include "inode.h"
#include "blocks.h"

char* get_filename_from_path(const char* path)
{
    return strrchr(path,'/')+1;  //TO DO: ce se intampla cand am doar root-ul
}

int get_file_inode_from_parent_dir(int directory_inode,const char* filename)
{
    inode* parent = get_nth_inode(directory_inode);
    //get parent block 
    void* parent_dentry_block=nth_block_pointer(parent->block_number);
    //get number of dentries for parent
    int dentries_number=parent->dentries_size/sizeof(dentry);
    //get the dentries pointer
    dentry* dentries_base=(dentry*)parent_dentry_block;

    for(int i=0;i<dentries_number;i++)
    {
        if(strcmp((dentries_base+i)->name,filename)==0)
        {
            //directory or file found. Return inode
            return (dentries_base+i)->inode_number;
        }
    }

    return -1;
}