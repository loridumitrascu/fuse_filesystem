#ifndef _DISKIMAGE_H
#define _DISKIMAGE_H

#include <sys/stat.h>
#include <fuse.h>
#include <string.h>

#include "inode.h"

void disk_mount_the_filesystem(const char *disk_iso_path);

void *init_disk_image(const char *disk_iso_path);

void remount_disk_image(const char *disk_iso_path);

void init_filesystem(const char *disk_iso_path);

void unmap_filesystem(void *disk_iso_base);

int disk_access(const char *entry_path);

int disk_check_permissions(const char *entry_path, int mask);

int disk_get_attributes_from_path(const char *path, struct stat *stbuf);

int disk_get_attributes_from_inode(int inode_number, struct stat *stbuf);

int disk_mknod(const char *path, mode_t mode);

int disk_chmod(const char *path, mode_t mode);

int disk_link(const char *from, const char *to);

int disk_mkdir(const char *path, mode_t mode);

int disk_change_utimens(const char *path, const struct timespec times[2]);

int disk_readdir(const char *path, void *buf, fuse_fill_dir_t filler);

int write_data_in_file(inode *file, const char *buf, size_t size, off_t offset);

int disk_write(const char *path, const char *buf, size_t size, off_t offset);

int read_data_from_file(inode *file, char *buf, size_t size, off_t offset);

int disk_read(const char *path, char *buf, size_t size, off_t offset);

#endif