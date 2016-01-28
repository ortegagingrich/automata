#include <stdio.h>

#include "byte_data.h"
#include "print.h"

void print_separator_line(char c){
	/**
	 * Prints a line of 80 copies of the specified character
	 */
	int i;
	for(i = 0; i < 80; i++){
		printf("%c", c);
	}
	printf("\n");
}


void print_byte_data_hex(void *data, unsigned long n_bytes){
	/**
	 * Prints the first n_bytes starting at data as hexidecimals.
	 */
	int i;
	for(i = 0; i < n_bytes; i++){
		unsigned char *c = data + i;
		printf("%02x", *c);
	}
}


void print_byte_data_bin(void *data, unsigned long n_bytes){
	/**
	 * Prints the first n_bytes start at data in binary.
	 */
	printf("|");
	int i;
	for(i = 0; i < 8 * n_bytes; i++){
		int bit = read_bit_byte_data(data, i);
		printf("%d", bit);
		if((i % 8) == 7){
			printf("|");
		}
	}
}
