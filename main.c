#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include "bitmap.h"
#include "disk_image.h"

int main(int argc,char* argv[])
{
    void* base = init_disk_image("disk_iso");
    uint8_t bitmap1[2];
    memset(bitmap1,0,2);
    bitmap1[0]=(uint8_t)27;
    bitmap1[1]=(uint8_t)18;
    printf("%d %d\n",bitmap1[0],bitmap1[1]);
    
    for(int i=0;i<16;i++)
    {
        printf("%d ",bit_map_get_bit(bitmap1,i));
    }
    void* used_base=base;
    *((uint8_t*)used_base)=bitmap1[0];
    used_base=used_base+1;
    *((uint8_t*)used_base)=bitmap1[1];

    munmap(base, 256*4096);
    
    return 0;
}