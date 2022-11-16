#ifndef _BITMAP_H
#define _BITMAP_H

#define byte_index (bit_position/8) 
#define bit_index_in_byte (bit_position%8) 
#define mask1 (1<<(7-bit_index_in_byte)) 
#define mask0 (255^mask1) //exemplu: 1111 1111 xor 0001 0000 = 1110 1111  -> punem 0 pe pozitia pe care avem nevoie de 0
//#define mask0 (~mask1)

//set the value in the given bitmap to bit_value for the bit at position bit_position
void bit_map_set_bit(void* bitmap,int bit_position,int bit_value);

//get the value from the given bitmap for the bit at position bit_position
int bit_map_get_bit(void* bitmap,int bit_position);

#endif