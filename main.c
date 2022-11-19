#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "disk_image.h"
#include "utils.h"

#include "blocks.h"
#include "bitmap.h"
#include "inode.h"
#include "dentry.h"

int main(int argc,char* argv[])
{
    init_filesystem("disk_iso");

    unmap_filesystem(blocks_base);
    remove("disk_iso");
    
    return 0;
}