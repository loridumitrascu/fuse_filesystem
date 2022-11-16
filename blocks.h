#define BLOCK_COUNT 256
#define BLOCK_SIZE 4096 
#define SYS_SIZE BLOCK_SIZE * BLOCK_COUNT  // = 1MB

int count_blocks_for_bytes(int nr_bytes); //return the number of blocks needed for n bytes file

void blocks_init(const char* disk_iso_path); //initialise the filesystem
void blocks_free();

void* nth_block_pointer(int n); //return the pointer to the starting zone of the block with index n

void* get_bitmap_datablock_ptr();
void* get_bitmap_inode_ptr();

int alloc_block(); //return the index of the new allocated block
void free_block(int n); //free the block with index n