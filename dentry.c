#include "dentry.h"
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include "inode.h"
#include "blocks.h"
#include "log.h"

char *get_filename_from_path(const char *path)
{
    return strrchr(path, '/') + 1;
}

int get_file_inode_from_parent_dir(int directory_inode, const char *filename)
{
    inode *parent = get_nth_inode(directory_inode);
    if (parent->is_dir == 0)
    {
        return -1;
    }
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
        if (curr_inode < 0)
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

    //-1 because the current_inode->size modified its value in the truncate_up_to_size_for_inode.
    dentries_base[current_inode->size / sizeof(dentry) - 1] = new_dentry;
}

int delete_file_from_inode_dentries(int inode_number, char *file_name)
{
    inode *curr_inode = get_nth_inode(inode_number);
    void *current_dentry_block = nth_block_pointer(curr_inode->block_number);
    int dentries_number = curr_inode->size / sizeof(dentry);
    dentry *dentries_base = (dentry *)current_dentry_block;

    int i;
    int found=0;
    for (i = 0; i < dentries_number; i++)
    {
        if (strcmp((dentries_base + i)->name, file_name) == 0)
        {
            found=1;
            inode *file = get_nth_inode(dentries_base[i].inode_number);
            if (file->nlink == 1)
            {
                free_inode(dentries_base[i].inode_number);
            }
            else
            {
                if(file->nlink>1)
                {
                    file->nlink--;
                }
            }
            break;
        }
    }

    if(found==0)
    {
        return -1;
    }

    //move the dentries from past the file index we removed with 1 step behind 
    for (; i < dentries_number - 1; i++)
    {
        dentries_base[i].inode_number=dentries_base[i+1].inode_number;
        strcpy(dentries_base[i].name,dentries_base[i+1].name);
        dentries_base[i].name[strlen(dentries_base[i].name)]='\0';
    }
        truncate_down_to_size_for_inode(inode_number, curr_inode->size - sizeof(dentry));
    return 0;
}

void get_parent_path_and_child_name(const char *path, char *parent_path, char *child_name)
{
    // Kind Reminder: vezi alocare childname la apelarea functiei
    char *aux = strdup(get_filename_from_path(path));
    strcpy(child_name, aux);
    free(aux);
    child_name[strlen(child_name)] = '\0';
    strncpy(parent_path, path, strlen(path) - strlen(child_name));
    if (strlen(parent_path) > 1) // parent!=root
        parent_path[strlen(parent_path) - 1] = '\0';
    else
        parent_path[strlen(parent_path)] = '\0';
}


int delete_dir_from_inode_dentries(int inode_number, char* dir_name)
{
    inode *curr_inode = get_nth_inode(inode_number);
    void *current_dentry_block = nth_block_pointer(curr_inode->block_number);
    int dentries_number = curr_inode->size / sizeof(dentry);
    dentry *dentries_base = (dentry *)current_dentry_block;

    int i;
    int found=0;
    for (i = 0; i < dentries_number; i++)
    {
        if (strcmp((dentries_base + i)->name, dir_name) == 0)
        {
            found=1;
            inode *dir = get_nth_inode(dentries_base[i].inode_number);
            if (dir->nlink == 2)
            {
                free_inode(dentries_base[i].inode_number);
            }
            else
            {
                if(dir->nlink>2)
                {
                    dir->nlink--;
                }
            }
            break;
        }
    }

    if(found==0)
    {
        return -1;
    }

    //move the dentries from past the file index we removed with 1 step behind 
    for (; i < dentries_number - 1; i++)
    {
        dentries_base[i].inode_number=dentries_base[i+1].inode_number;
        strcpy(dentries_base[i].name,dentries_base[i+1].name);
        dentries_base[i].name[strlen(dentries_base[i].name)]='\0';
    }
    truncate_down_to_size_for_inode(inode_number, curr_inode->size - sizeof(dentry));
    return 0;
}