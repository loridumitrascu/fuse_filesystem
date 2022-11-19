#ifndef _INODE_H
#define _INODE_H
#include <time.h>

typedef struct inode
{
    int dentries_size;
    int block_number;
    int inode_number;
    int mode;

    int uid;
    int gid;

    //int nlink;

    time_t ctime;
    time_t mtime;
    time_t atime;
}inode;

inode* get_nth_inode(int index);
int get_max_inode_number();
int get_first_free_inode_from_ibitmap();

void* get_inode_block_base_ptr();

inode* alloc_inode();
void free_inode();

#endif