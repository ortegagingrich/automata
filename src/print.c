#include <stdio.h>

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


void print_byte_data(void *data, unsigned long n_bytes){
	/**
	 * Prints the first n_bytes starting at data as hexidecimals.
	 */
	int i;
	for(i = 0; i < n_bytes; i++){
		unsigned char *c = data + i;
		printf("%2x", *c);
	}
}