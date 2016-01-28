/**
 * Functions for handling pointers to byte data which does not necessarily
 * correspond to a built in type.  These may be used, for example, for 
 * bit-packing arrays.
 */
#include <stdio.h>
#include <stdlib.h>

#include "byte_data.h"
#include "print.h"


int compare_byte_data(void *data1, void *data2, unsigned long length){
	/**
	 * Checks to see if the strings of bytes of the specified length (in bytes)
	 * from the provided pointers are equal.
	 */
	int i;
	for(i = 0; i < length; i++){
		unsigned char *d1, *d2;
		d1 = data1 + i;
		d2 = data2 + i;
		if(*d1 != *d2){
			return 0;
		}
	}
	
	//if we get this far, they must match
	return 1;
}


int read_bit_byte_data(void *data, int offset){
	/**
	 * Reads and returns (as an integer), the specified bit offset from the
	 * specified block of data.
	 */
	
	//first, determine which byte to look in
	int byte_offset = offset / 8;
	int bit_offset = 7 - (offset % 8);
	
	unsigned char *byte;
	byte = (void*) (data + byte_offset);
	
	//now extract the relevant bit
	if(*byte>>bit_offset & 0x01){
		return 1;
	}else{
		return 0;
	}
}


void write_bit_byte_data(void *data, int offset, int bit){
	/**
	 * Writes the specified bit in the block of data.
	 */
	
	//first, determine which byte to look in
	int byte_offset = offset / 8;
	int bit_offset = 7 - (offset % 8);
	
	unsigned char *byte;
	byte = (void*) (data + byte_offset);
	
	if(bit){
		//write a one
		//check if already one
		if(!read_bit_byte_data(data, offset)){
			unsigned char delta = 1<<bit_offset;
			*byte += delta;
		}
	}else{
		//write a zero, if not already
		if(read_bit_byte_data(data, offset)){
			unsigned char delta = 1<<bit_offset;
			*byte -= delta;
		}
	}
}


/*
 * Tests
 */
int byte_data_test(){
	/**
	 * Entry point for tests.
	 */
	printf("Byte Data Tests:\n\n");
	
	int *i3 = malloc(4*sizeof(int));
	i3[0] = 0x12345678;
	i3[1] = 2;
	i3[2] = 3;
	i3[3] = 42;
	
	
	print_byte_data(i3, sizeof(int));
	printf("\n");
	
	write_bit_byte_data(i3, 1, 0);
	
	print_byte_data(i3, sizeof(int));
	printf("\n");
	
	int i;
	int n = 32;
	for(i = 0; i < n; i++){
		int bit = read_bit_byte_data(i3, i);
		printf("%d", bit);
	}
	printf("\n");
	
	free(i3);
	
	return 0;
}
