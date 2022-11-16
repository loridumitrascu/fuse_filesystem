#ifndef _DISKIMAGE_H
#define _DISKIMAGE_H

#define BlocksNumber 256
#define BlocksSize 4098
#define FileSystemSize (BlocksNumber*BlocksSize) //1 Mb

void* init_disk_image(const char* disk_iso_path);

void init_filesystem(const char* disk_iso_path);
#endif