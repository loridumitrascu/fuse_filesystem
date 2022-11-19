#ifndef _DISKIMAGE_H
#define _DISKIMAGE_H

void* init_disk_image(const char* disk_iso_path);

void init_filesystem(const char* disk_iso_path);

void unmap_filesystem(void* disk_iso_base);
#endif