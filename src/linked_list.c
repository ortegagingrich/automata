/**
 * Functions for creating/manipulating linked lists.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
#include "print.h"


/*
 * Methods for creating linked lists
 */

LinkedList *create_linked_list(unsigned long data_size){
	/**
	 * Creates and returns an empty linked list with the specified data size.
	 */
	LinkedList *list = malloc(sizeof(LinkedList));
	list->data_size = data_size;
	list->begin = NULL;
	list->end = NULL;
	
	return list;
}


LinkedList *create_linked_list_from_array(unsigned long data_size, void *array, int n){
	/**
	 * Creates and returns a linked list by reading n objects of size data_size
	 * from the array indicated by the pointer.
	 */
	LinkedList *list = create_linked_list(data_size);
	
	int i;
	for(i = 0; i < n; i++){
		//allocate memory for data copy
		void *item = malloc(data_size);
		void *address = array + i*data_size;
		memcpy(item, address, data_size);
		append_linked_list(list, item);
	}
	
	
	return list;
}


/*
 * Methods for manipulating linked lists
 */

void append_linked_list(LinkedList *list, void *item){
	/**
	 * Appends the data at the pointer item to the provided list.
	 */
	if(list == NULL){
		return;
	}
	//make a new node
	struct linked_list_node *new_node = malloc(sizeof(struct linked_list_node));
	new_node->master = list;
	new_node->data = item;
	new_node->next = NULL;
	
	
	//if first node in empty list
	if(list->begin == NULL){
		new_node->last = NULL;
		list->begin = new_node;
	}else{
		new_node->last = list->end;
		new_node->last->next = new_node;
	}
	list->end = new_node;
	
}


/*
 * Methods for retrieving data from linked lists
 */

int count_linked_list(LinkedList *list){
	/*
	 * Returns the number of nodes in the provided list.
	 */
	if(list == NULL){
		return -1;
	}
	int count = 0;
	struct linked_list_node *node = list->begin;
	while(node != NULL){
		count++;
		node = node->next;
	}
	return count;
}


void *get_linked_list(LinkedList *list, int n){
	/**
	 * Returns the nth element of the provided linked list if it exists.
	 * Otherwise, the NULL pointer is returned
	 */
	if(list == NULL){
		printf("Null List\n");
		return NULL;
	}
	
	int i;
	struct linked_list_node *node = list->begin;
	for(i = 0; i < n; i++){
		if(node == NULL){
			printf("Null Node\n");
			return NULL;
		}
		node = node->next;
	}
	void *item = node->data;
	
	return item;
}


void print_linked_list(LinkedList *list){
	/**
	 * Prints the specified linked list to the console.  Each element's data
	 * is printed byte-wise as a hexadecimal.
	 */
	printf("\n");
	int n = count_linked_list(list);
	printf("Linked List of length %d:\n", n);
	print_separator_line('-');
	
	int i;
	for(i = 0; i < n; i++){
		printf("%3d || ", i);
		
		void *data = get_linked_list(list, i);
		print_byte_data(data, list->data_size);
		
		printf("\n");
	}
	
	print_separator_line('-');
}

/*
 * Methods for deleting linked lists
 */
void delete_linked_list(LinkedList *list){
	/**
	 * Frees all memory associated with the provided linked list.
	 */
	//TODO: replace this with functionality from pop function.
	while(list->end != NULL){
		struct linked_list_node *node = list->end;
		list->end = node->last;
		free(node);
	}
	
	free(list);
}


/*
 * Tests
 */
void linked_list_test(){
	/**
	 * Entry point for tests
	 */
	printf("Linked List Tests:\n\n");
	
	int array[] = {1, 2, 3, 4, 42, 11, 6845};
	LinkedList *list = create_linked_list_from_array(sizeof(int), array, 7);
	
	print_linked_list(list);
	
	delete_linked_list(list);
}




















