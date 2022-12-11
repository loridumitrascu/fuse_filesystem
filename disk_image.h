#ifndef _DISKIMAGE_H
#define _DISKIMAGE_H

void disk_mount_the_filesystem(const char* disk_iso_path);

void* init_disk_image(const char* disk_iso_path);

void init_filesystem(const char* disk_iso_path);

void unmap_filesystem(void* disk_iso_base);

int disk_access(const char* entry_path);

int disk_check_permissions(const char* entry_path,int mask);
#endif