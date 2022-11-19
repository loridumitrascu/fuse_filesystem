#ifndef _DENTRY_H
#define _DENTRY_H

#include "utils.h"

typedef struct dentry{
    int inode_number;
    char name[MAX_FILENAME];
}dentry;

char* get_filename_from_path(const char* path); //TO DO: grija la dezalocari

int get_file_inode_from_parent_dir(int directory_inode,const char* filename);

#endif