#include <stdio.h>
#include <stdlib.h>

#include "automata.h"
#include "linked_list.h"
#include "byte_data.h"

int test();
int test2();

int main(int argc, char *argv[]){
	printf("Starting.\n");
	int status = 0;
	
	
	status += test();
	//status += test2();
	//status += linked_list_test();
	//status += byte_data_test();
	
	return status;
}


int test(){
	/*
	 * Warning: this test is leaky; All intermediate machines should be freed.
	 */
	FiniteAutomaton *j = create_automaton_char('j');
	FiniteAutomaton *o = create_automaton_char('o');
	FiniteAutomaton *g = create_automaton_char('g');
	FiniteAutomaton *e = create_automaton_char('e');
	FiniteAutomaton *blank = create_automaton_char(' ');
	FiniteAutomaton *dash = create_automaton_char('-');
	
	print_automaton(dash);
	
	//Warning:leaky
	FiniteAutomaton *jiter, *middle, *full_last, *last, *lastiter, *total;
	jiter = create_automaton_iteration(j);
	middle = create_automaton_alternation(e, blank);
	full_last = create_automaton_concatenation(o, create_automaton_concatenation(dash, g));
	last = create_automaton_concatenation(blank, create_automaton_alternation(o, full_last));
	lastiter = create_automaton_iteration(last);
	
	total = create_automaton_concatenation(jiter, create_automaton_concatenation(middle, lastiter));
	
	print_automaton(total);
	
	FiniteAutomaton *det;
	det = create_automaton_deterministic(total);
	print_automaton(det);
	
	char test_string[] = "jje o-g o o o-g";
	int result = automaton_test_string(det, test_string, 15);
	printf("String match: \"%s\" : %d\n", test_string, result);
	
	/*FiniteAutomaton *a, *b, *c, *d, *e, *f;
	
	a = create_automaton_concatenation(j, o);
	b = create_automaton_alternation(j, o);
	c = create_automaton_alternation(b, g);
	d = create_automaton_iteration(j);
	
	e = create_automaton_iteration(c);
	
	
	//print_automaton(d);
	print_automaton(e);
	
	f = create_automaton_deterministic(e);
	print_automaton(f);
	
	
	
	
	delete_automaton(a);
	delete_automaton(b);
	delete_automaton(c);
	delete_automaton(d);
	delete_automaton(e);
	delete_automaton(f);
	delete_automaton(j);
	delete_automaton(o);
	delete_automaton(g);
	*/
	return 0;
}

int test2(){
	printf("%lu\n", sizeof(sizeof(struct automaton_transition)));
	printf("%lu\n", sizeof(long));
	
	return 0;
}



