/**
 * Contains methods specifically related to checking and executing deterministic
 * finite automata.  The public methods are declared in automata.h.
 */
#include <stdio.h>
#include <stdlib.h>

#include "automata.h"


int automaton_is_deterministic(FiniteAutomaton *automaton){
	/**
	 * Determines if the provided automaton is deterministic by checking to see
	 * if any of the transitions are epsilon transitions.
	 */
	int i, j;
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		for(j = 0; j < node->n_transitions; j++){
			struct automaton_transition *transitions = node->transitions[j];
			if(transitions->is_epsilon){
				return 0;
			}
		}
	}
	
	return 1;
}
