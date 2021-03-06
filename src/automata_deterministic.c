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
	
	//include in the new state if it has a non-epsilon transition or if finish
	if(has_non_epsilon||node->is_ending_state){
		write_bit_byte_data(new_state, identifier, 1);
	}
}



static void process(void *tentative_state, void *previous_node, char tchar,
                    LinkedList *node_list, FiniteAutomaton *automaton,
                    LinkedList *transition_chars, 
                    LinkedList *trans_from, LinkedList *trans_to,
                    LinkedList *trans_con, LinkedList *finish){
	/*
	 * Good Luck.
	 */
	unsigned long data_size = 1 + (automaton->n_nodes / 8);
	
	//first, fill out the tentative state
	void *new_state, *touched;
	new_state = calloc(data_size, 1);
	touched = calloc(data_size, 1);
	
	int i, j, k;
	for(i = 0; i < automaton->n_nodes; i++){
		if(read_bit_byte_data(tentative_state, i)){
			struct automaton_node *node = automaton->nodes[i];
			fill(node, new_state, touched, automaton, transition_chars);
		}
	}
	free(touched);
	
	if(byte_data_is_zero(new_state, data_size)){
		return;
	}
	
	//Add connection from previous node state to new state
	if(previous_node != NULL){
		append_linked_list(trans_from, previous_node);
		append_linked_list(trans_to, new_state);
		char *c = malloc(sizeof(char));
		*c = tchar;
		append_linked_list(trans_con, c);
	}
	
	
	
	//check to see if there is already a node for this state, or if it is zero
	if(linked_list_contains(node_list, new_state)){
		return;
	}
	
	
	//make new node
	append_linked_list(node_list, new_state);
	
	//check to see if this is a finished state
	char *fstate = malloc(sizeof(char));
	*fstate = 0;
	for(i = 0; i < automaton->n_nodes; i++){
		if(read_bit_byte_data(new_state, i)){
			struct automaton_node *node = automaton->nodes[i];
			if(node->is_ending_state){
				*fstate = 1;
				break;
			}
		}
	}
	append_linked_list(finish, fstate);
	
	
	
	//look for outgoing transitions
	int n_chars = count_linked_list(transition_chars);
	for(i = 0; i < n_chars; i++){
		char c = *((char*) get_linked_list(transition_chars, i));
		
		void *new_tentative_state = calloc(data_size, 1);
		
		for(j = 0; j < automaton->n_nodes; j++){
			if(read_bit_byte_data(new_state, j)){
				struct automaton_node *node = automaton->nodes[j];
				for(k = 0; k < node->n_transitions; k++){
					struct automaton_transition *t = node->transitions[k];
					if(t->condition == c){
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
		        transition_chars, trans_from, trans_to, trans_con, finish);
		free(new_tentative_state);
	}
	
}



FiniteAutomaton *create_automaton_deterministic(FiniteAutomaton *ndfa){
	/**
	 * Creates a deterministic finite automaton equivalent to the provided
	 * non-deterministic finite automaton.
	 */
	if(ndfa == NULL){
		return NULL;
	}
	
	/*
	 * First, we must collect all data needed to make the new automaton.
	 * We need:
	 * 	1) A list of states for the new nodes.
	 * 	2) A list of transitions.
	 */
	
	//determine state data size is bytes
	unsigned long data_size = 1 + (ndfa->n_nodes / 8);
	
	//List objects
	LinkedList *states = create_linked_list(data_size);
	LinkedList *finish = create_linked_list(sizeof(char));
	LinkedList *trans_from = create_linked_list(data_size);
	LinkedList *trans_to = create_linked_list(data_size);
	LinkedList *trans_con = create_linked_list(sizeof(char));
	LinkedList *tchars = create_linked_list(sizeof(char));
	
	
	//make tentative starting state
	void *starting = calloc(data_size, 1);
	write_bit_byte_data(starting, ndfa->starting_state, 1);
	
	//recursively collect data about new nodes
	process(starting, NULL, ' ', states, ndfa, tchars, trans_from, trans_to, 
	        trans_con, finish);
	free(starting);
	
	/*print_linked_list(states);
	print_linked_list(finish);
	print_linked_list(tchars);
	print_linked_list(trans_from);
	print_linked_list(trans_to);
	print_linked_list(trans_con);*/
	
	/*
	 * Now that we have all of the information we need, make the new automaton
	 * object.
	 */
	FiniteAutomaton *automaton = malloc(sizeof(FiniteAutomaton));
	
	
	automaton->lookup_table = NULL;
	automaton->transition_chars = NULL;
	automaton->n_transition_chars = 0;
	
	int n = count_linked_list(states);
	automaton->n_nodes = n;
	automaton->starting_state = 0;
	automaton->nodes = malloc(n * sizeof(struct automaton_node*));
	
	//make nodes
	int i, j;
	for(i = 0; i < n; i++){
		void *state = get_linked_list(states, i);
		struct automaton_node *node = malloc(sizeof(struct automaton_node));
		node->identifier = i;
		node->is_ending_state = *((char *)get_linked_list(finish, i));
		
		//count transitions
		int nt = 0;
		for(j = 0; j < count_linked_list(trans_from); j++){
			void *from = get_linked_list(trans_from, j);
			if(compare_byte_data(from, state, data_size)){
				nt++;
			}
		}
		
		//make transitions
		node->n_transitions = nt;
		node->transitions = malloc(nt * sizeof(struct automaton_transition*));
		
		int tcount = 0;
		for(j = 0; j < count_linked_list(trans_from); j++){
			void *from = get_linked_list(trans_from, j);
			if(compare_byte_data(from, state, data_size)){
				void *to = get_linked_list(trans_to, j);
				struct automaton_transition *transition;
				transition = malloc(sizeof(struct automaton_transition));
				transition->is_epsilon = 0;
				
				char c = *((char*) get_linked_list(trans_con, j));
				transition->condition = c;
				
				int staten = linked_list_find(states, to);
				transition->identifier = staten;
				
				node->transitions[tcount] = transition;
				tcount++;
			}
		}
		
		automaton->nodes[i] = node;
	}
	
	
	//clean up and exit
	delete_linked_list_deep(states);
	delete_linked_list_deep(finish);
	delete_linked_list_deep(tchars);
	delete_linked_list(trans_from);
	delete_linked_list(trans_to);
	delete_linked_list_deep(trans_con);
	return automaton;
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


/*
 * Lookup Table/execution methods
 */


static void generate_lookup_table(FiniteAutomaton *automaton){
	/**
	 * Creates or updates the lookup table for the specified automaton.
	 */
	if(!automaton_is_deterministic(automaton)){
		printf("Cannot generate a lookup table for a non-deterministic ");
		printf("automaton.  Please convert to a deterministic automaton.");
		return;
	}
	
	printf("Generating lookup data for automata of size %d.\n", automaton->n_nodes);
	
	//Just to make sure there is no residual data
	free(automaton->lookup_table);
	free(automaton->transition_chars);
	
	/*
	 * Index all characters used
	 */
	
	//make a temporary list of transition characters
	LinkedList *char_list = create_linked_list(sizeof(char));
	automaton->n_transition_chars = 0;
	int i, j, k;
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		for(j = 0; j < node->n_transitions; j++){
			struct automaton_transition *t = node->transitions[j];
			char *c = malloc(sizeof(char));
			*c = t->condition;
			if(!linked_list_contains(char_list, c)){
				append_linked_list(char_list, c);
				automaton->n_transition_chars++;
			}else{
				free(c);
			}
		}
	}
	
	//make char index
	int ntc = automaton->n_transition_chars;
	automaton->transition_chars = malloc(ntc * sizeof(char));
	for(i = 0; i < ntc; i++){
		char *c = get_linked_list(char_list, i);
		automaton->transition_chars[i] = *c;
	}
	delete_linked_list_deep(char_list);
	
	//allocate lookup table
	int nspots = ntc * automaton->n_nodes;
	automaton->lookup_table = malloc(nspots * sizeof(int));
	
	/*
	 * Fill Lookup table
	 */
	//default values, indicating no move
	for(i = 0; i < nspots; i++){
		automaton->lookup_table[i] = -1;
	}
	
	//loop over transitions
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		for(j = 0; j < node->n_transitions; j++){
			struct automaton_transition *transition = node->transitions[j];
			char c = transition->condition;
			
			//find index
			int char_index = -1;
			for(k = 0; k < ntc; k++){
				if(c == automaton->transition_chars[k]){
					char_index = k;
					break;
				}
			}
			//if no match
			if(char_index == -1){
				printf("This should never happen.\n");
				exit(1);
			}
			
			//fill in correct entry
			int address = i * ntc + char_index;
			automaton->lookup_table[address] = transition->identifier;
		}
	}
}


static int read_lookup_table(FiniteAutomaton *automaton, int id, char cond){
	/**
	 * Finds and returns the node of the appropriate transition if it exists,
	 * otherwise, returns -1.
	 */
	if(automaton == NULL){
		printf("Are you stupid? \n");
		exit(1);
	}
	if(automaton->lookup_table == NULL){
		generate_lookup_table(automaton);
	}
	
	//get char index
	int char_index = -1;
	int i;
	for(i = 0; i < automaton->n_transition_chars; i++){
		if(cond == automaton->transition_chars[i]){
			char_index = i;
			break;
		}
	}
	
	//if no character of this type in table
	if(char_index == -1){
		return -1;
	}
	
	//read from the array
	int address = id * automaton->n_transition_chars + char_index;
	return automaton->lookup_table[address];
}


/*
 * Regex Methods
 */
int automaton_test_string(FiniteAutomaton *automaton, char* string, int length){
	/**
	 * Uses the provided automaton (assuming it is deterministic) to test the
	 * provided string of the specified length.  Returns 0 for failure and 1 
	 * for success.
	 */
	if(!automaton_is_deterministic(automaton)){
		printf("Cannot test string \"%s\" with a non-deterministic ",string);
		printf("automaton.  Please convert to a deterministic automaton.\n");
		return 0;
	}
	
	//check if lookup table exists yet
	if(automaton->lookup_table == NULL){
		generate_lookup_table(automaton);
	}
	
	
	//do simulation
	int state = automaton->starting_state;
	int i;
	for(i = 0; i < length; i++){
		char next_char = string[i];
		int next_id = read_lookup_table(automaton, state, next_char);
		if(next_id < 0){
			//no path
			return 0;
		}
		state = next_id;
	}
	
	//check if we have a finished state
	if(automaton->nodes[state]->is_ending_state){
		return 1;
	}else{
		return 0;
	}
}


