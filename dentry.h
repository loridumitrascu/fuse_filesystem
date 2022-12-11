#ifndef _DENTRY_H
#define _DENTRY_H

#include "utils.h"

typedef struct dentry{
    int inode_number;
    char name[MAX_FILENAME];
}dentry;

char* get_filename_from_path(const char* path); //TO DO: grija la dezalocari

int get_file_inode_from_parent_dir(int directory_inode,const char* filename);

int get_file_inode_from_path(const char* path);

void add_dir_to_inode_dentries(int inode_number, char* dir_name, int new_dir_inode);

void delete_dir_from_inode_dentries(int inode_number, char* dir_name);

void get_parent_path_and_child_name(const char* path, char* parent_path, char* child_name);

#endif