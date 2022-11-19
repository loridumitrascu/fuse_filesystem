#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include "disk_image.h"
#include "utils.h"

#include "blocks.h"
#include "bitmap.h"
#include "inode.h"

int main(int argc,char* argv[])
{
    init_filesystem("disk_iso");

    inode* i1=alloc_inode();
    
    inode* i2=alloc_inode();

    free_inode(i1->inode_number);

    unmap_filesystem(blocks_base);
    
    return 0;
}