#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "bitmap.h"


void bit_map_set_bit(void* bitmap,int bit_position,int bit_value)
{
    assert((bit_value==0)||(bit_value==1));
    if(bit_value==1)
    {
        ((uint8_t*)bitmap)[byte_index]=(((uint8_t*)bitmap)[byte_index])|mask1;
    }
    else
    {
        ((uint8_t*)bitmap)[byte_index]=(((uint8_t*)bitmap)[byte_index])&mask0;
    }
}

int bit_map_get_bit(void* bitmap,int bit_position)
{
    return (((uint8_t*)bitmap)[byte_index])>>(7-bit_index_in_byte)&1;
}

