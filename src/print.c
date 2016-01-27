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
