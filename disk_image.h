#ifndef _DISKIMAGE_H
#define _DISKIMAGE_H

#include <sys/stat.h>
#include <fuse.h>
#include <string.h>

void disk_mount_the_filesystem(const char* disk_iso_path);

void* init_disk_image(const char* disk_iso_path);

void init_filesystem(const char* disk_iso_path);

void unmap_filesystem(void* disk_iso_base);

int disk_access(const char* entry_path);

int disk_check_permissions(const char* entry_path,int mask);

int disk_get_attributes_from_path(const char *path, struct stat *stbuf);

int disk_get_attributes_from_inode(int inode_number, struct stat *stbuf);

int disk_mknod(const char *path, mode_t mode);

int disk_readdir(const char* path,void* buf,fuse_fill_dir_t filler);

#endif