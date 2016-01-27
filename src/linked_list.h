/**
 * Data structures and functions for working with doubly linked lists of
 * pointers.  Note that the lists here are chains and cannot have loops.  Also
 * note that the data must be of the sort that can be freed using only the free
 * command; as such, this is really not intended to be used to hold structs
 * containing pointers.
 */

struct linked_list_node {
	void *data;
	struct linked_list_master *master;
	struct linked_list_node *last, *next;
};

typedef struct linked_list_master {
	unsigned long data_size;
	struct linked_list_node *begin, *end;
} LinkedList;


LinkedList *create_linked_list(unsigned long);
LinkedList *create_linked_list_from_array(unsigned long, void*, int);

void append_linked_list(LinkedList*, void*);
void *pop_linked_list(LinkedList*);

int count_linked_list(LinkedList*);
void *get_linked_list(LinkedList*, int);
void print_linked_list(LinkedList*);

void delete_linked_list(LinkedList*);

//test function
void linked_list_test();
