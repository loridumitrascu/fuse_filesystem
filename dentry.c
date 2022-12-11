#include "dentry.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "blocks.h"

char *get_filename_from_path(const char *path)
{
    return strrchr(path, '/') + 1;
}

int get_file_inode_from_parent_dir(int directory_inode, const char *filename) 
{
    inode *parent = get_nth_inode(directory_inode);
    // get parent block
    void *parent_dentry_block = nth_block_pointer(parent->block_number);
    // get number of dentries for parent
    int dentries_number = parent->size / sizeof(dentry);
    // get the dentries pointer
    dentry *dentries_base = (dentry *)parent_dentry_block;

    for (int i = 0; i < dentries_number; i++)
    {
        if (strcmp((dentries_base + i)->name, filename) == 0)
        {
            // directory or file found. Return inode
            return (dentries_base + i)->inode_number;
        }
    }
    return -1;
}

int get_file_inode_from_path(const char *path)
{
    if (strcmp(path, "/") == 0)
        return 0;

    int curr_parent_inode = 0;
    char *aux_path = strdup(path);
    char *curr_name = strtok(aux_path + 1, "/");

    while (curr_name != NULL)
    {
        int curr_inode = get_file_inode_from_parent_dir(curr_parent_inode, curr_name);
        if(curr_inode<0)
        {
            return -1;
        }
        curr_parent_inode = curr_inode;
        curr_name = strtok(NULL, "/");
    }
    free(aux_path);
    return curr_parent_inode;
}

void add_dir_to_inode_dentries(int inode_number, char *dir_name, int new_dir_inode)
{
    inode *current_inode = get_nth_inode(inode_number);
    void *dentry_block = nth_block_pointer(current_inode->block_number);
    dentry *dentries_base = (dentry *)dentry_block;
    truncate_up_to_size_for_inode(inode_number, current_inode->size + sizeof(dentry));

    dentry new_dentry;
    strcpy(new_dentry.name, dir_name);
    new_dentry.name[strlen(new_dentry.name)] = '\0';
    new_dentry.inode_number = new_dir_inode;

    dentries_base[current_inode->size / sizeof(dentry)] = new_dentry;
}

void delete_dir_from_inode_dentries(int inode_number, char *dir_name)
{
    inode *curr_inode = get_nth_inode(inode_number);
    void *current_dentry_block = nth_block_pointer(curr_inode->block_number);
    int dentries_number = curr_inode->size / sizeof(dentry);
    dentry *dentries_base = (dentry *)current_dentry_block;

    int i;
    for (i = 0; i < dentries_number; i++)
    {
        if (strcmp((dentries_base + i)->name, dir_name) == 0)
        {
            free_inode(dentries_base[i].inode_number);
            break;
        }
    }

    for (; i < dentries_number - 1; i++)
    {
        dentries_base[i] = dentries_base[i + 1];
    }

    if (i < dentries_number) // only if we found our dentry
        truncate_down_to_size_for_inode(inode_number, curr_inode->size - sizeof(dentry));
}

void get_parent_path_and_child_name(const char* path, char* parent_path, char* child_name)
{
    //Kind Reminder: vezi alocare childname la apelarea functiei
    char* aux=strdup(get_filename_from_path(path));
    strcpy(child_name, aux);
    free(aux);
    child_name[strlen(child_name)]='\0';
    strncpy(parent_path, path, strlen(path)-strlen(child_name));
    if(strlen(parent_path)>1) //parent!=root
        parent_path[strlen(parent_path)-1]='\0';
    else 
        parent_path[strlen(parent_path)]='\0';
}