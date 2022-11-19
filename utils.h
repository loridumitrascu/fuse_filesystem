#ifndef _UTILS_H
#define _UTILS_H

#define BLOCK_COUNT 256
#define BLOCK_SIZE 4096 
#define SYS_SIZE BLOCK_SIZE * BLOCK_COUNT  // = 1MB

extern void* blocks_base; //gets the address after we start the mapping

#endif