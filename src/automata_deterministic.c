/**
 * Contains methods specifically related to checking and executing deterministic
 * finite automata.  The public methods are declared in automata.h.
 */
#include <stdio.h>
#include <stdlib.h>

#include "automata.h"
#include "byte_data.h"
#include "linked_list.h"


static void fill(struct automaton_node *node, void *new_state, void *touched, 
                    FiniteAutomaton *automaton, LinkedList *transition_chars){
	/**
	 * Helper function for recursively converting to a deterministic automaton.
	 */
	int identifier = node->identifier;
	
	//if already touched, do nothing
	if(read_bit_byte_data(touched, identifier)){
		return;
	}
	
	//touch the current node
	write_bit_byte_data(touched, identifier, 1);
	
	//process all epsilon and non-epsilon transitions
	int has_non_epsilon = 0;
	int i;
	for(i = 0; i < node->n_transitions; i++){
		struct automaton_transition *transition = node->transitions[i];
		if(transition->is_epsilon){
			struct automaton_node *next_node;
			next_node = automaton->nodes[transition->identifier];
			fill(next_node, new_state, touched, automaton, transition_chars);
		}else{
			has_non_epsilon = 1;
			//make sure the list of valid transition chars has this one
			char *c = malloc(sizeof(char));
			*c = transition->condition;
			if(!linked_list_contains(transition_chars, c)){
				append_linked_list(transition_chars, c);
			}else{
				free(c);
			}
		}
	}
	
	//include in the new state if it has a non-epsilon transition
	if(has_non_epsilon){
		write_bit_byte_data(new_state, identifier, 1);
	}
}


struct void_tuple {
	void *p1, *p2;
};


static void process(void *tentative_state, void *previous_node, char tchar,
                    LinkedList *node_list, FiniteAutomaton *automaton,
                    LinkedList *transition_chars, LinkedList *transitions){
	/*
	 * Good Luck.
	 */
	
	//first, fill out the tentative state
	void *new_state, *touched;
	new_state = calloc(1 + (automaton->n_nodes / 8), 1);
	touched = calloc(1 + (automaton->n_nodes / 8), 1);
	
	int i, j, k;
	for(i = 0; i < automaton->n_nodes; i++){
		if(read_bit_byte_data(tentative_state, i)){
			struct automaton_node *node = automaton->nodes[i];
			fill(node, new_state, touched, automaton, transition_chars);
		}
	}
	free(touched);
	
	//Add connection from previous node state to new state
	if(previous_node != NULL){
		struct void_tuple *new_transition;
		new_transition = malloc(sizeof(struct void_tuple));
		new_transition->p1 = previous_node;
		new_transition->p2 = new_state;
		append_linked_list(transitions, new_transition);
	}
	
	
	
	//check to see if there is already a node for this state
	if(linked_list_contains(node_list, new_state)){
		return;
	}
	
	//make new node
	append_linked_list(node_list, new_state);
	
	
	
	//look for outgoing transitions
	int n_chars = count_linked_list(transition_chars);
	for(i = 0; i < n_chars; i++){
		char c = *((char*) get_linked_list(transition_chars, i));
		
		void *new_tentative_state = calloc(1 + (automaton->n_nodes / 8), 1);
		
		for(j = 0; j < automaton->n_nodes; j++){
			if(read_bit_byte_data(new_state, j)){
				struct automaton_node *node = automaton->nodes[j];
				for(k = 0; k < node->n_transitions; k++){
					struct automaton_transition *t = node->transitions[k];
					if(t->identifier == c){
						/*
						 * We have found a transition of this character type, so
						 * add its destination to the new tentative state
						 */
						write_bit_byte_data(new_tentative_state, t->identifier, 1);
					}
				}
			}
		}
		
		process(new_tentative_state, new_state, c, node_list, automaton, 
		        transition_chars, transitions);
		free(new_tentative_state);
	}
	
}



int automaton_is_deterministic(FiniteAutomaton *automaton){
	/**
	 * Determines if the provided automaton is deterministic by checking to see
	 * if any of the transitions are epsilon transitions.
	 */
	int i, j;
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		for(j = 0; j < node->n_transitions; j++){
			struct automaton_transition *transition = node->transitions[j];
			if(transition->is_epsilon){
				return 0;
			}
		}
	}
	
	return 1;
}
