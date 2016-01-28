/**
 * Functions for handling pointers to byte data which does not necessarily
 * correspond to a built in type.  These may be used, for example, for 
 * bit-packing arrays.
 */

int compare_byte_data(void*, void*, unsigned long);
int read_bit_byte_data(void*, int);
void write_bit_byte_data(void*, int, int);

int byte_data_test();
