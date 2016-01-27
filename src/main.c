#include <stdio.h>
#include <stdlib.h>

#include "automata.h"
#include "linked_list.h"

void test();
void test2();

int main(int argc, char *argv[]){
	printf("Starting.\n");
	
	//test();
	//test2();
	linked_list_test();
	
	return 0;
}


void test(){
	FiniteAutomaton *j = create_automaton_char('j');
	FiniteAutomaton *o = create_automaton_char('o');
	FiniteAutomaton *g = create_automaton_char('g');
	
	print_automaton(j);
	print_automaton(o);
	print_automaton(g);
	
	FiniteAutomaton *a, *b, *c, *d, *e;
	a = create_automaton_concatenation(j, o);
	b = create_automaton_alternation(j, o);
	c = create_automaton_alternation(b, g);
	d = create_automaton_iteration(j);
	
	e = create_automaton_iteration(c);
	
	
	print_automaton(d);
	print_automaton(e);
	
	delete_automaton(a);
	delete_automaton(b);
	delete_automaton(c);
	delete_automaton(j);
	delete_automaton(o);
	delete_automaton(g);
}

void test2(){
	printf("%lu\n", sizeof(sizeof(struct automaton_transition)));
	printf("%lu\n", sizeof(long));
}



