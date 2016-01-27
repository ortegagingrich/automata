#include <stdio.h>
#include <stdlib.h>

#include "automata.h"
#include "print.h"


static FiniteAutomaton *create_automaton_empty(int size){
	/**
	 * Creates an empty automaton of the given size that, without modification,
	 * accepts nothing.
	 */
	FiniteAutomaton *automaton = malloc(sizeof(FiniteAutomaton));
	automaton->starting_state = 0;
	automaton->n_nodes = size;
	automaton->nodes = malloc(size * sizeof(struct automaton_node*));
	
	//make empty nodes (no transitions)
	int i;
	for(i = 0; i < size; i++){
		struct automaton_node *node = malloc(sizeof(struct automaton_node));
		
		node->identifier = i;
		node->n_transitions = 0;
		node->is_ending_state = 0;
		node->transitions = NULL;
		
		automaton->nodes[i] = node;
	}
	
	return automaton;
}


static void copy_nodes(struct automaton_node **old_nodes,
                       struct automaton_node **new_nodes,
                       int n_nodes, int offset){
	/**
	 * Copies the first n_nodes nodes from old_nodes to new_nodes starting at
	 * position offset.  If offset is zero, it starts at the beginning.  Note 
	 * that the copy is a deep copy; all transition objects are new.
	 */
	int i, j;
	for(i = 0; i < n_nodes; i++){
		int new_i = i + offset;
		struct automaton_node *old_node, *new_node;
		old_node = old_nodes[i];
		new_node = malloc(sizeof(struct automaton_node));
		
		new_node->identifier = new_i;
		new_node->is_ending_state = old_node->is_ending_state;
		int nt = old_node->n_transitions;
		new_node->n_transitions = nt;
		
		//make transitions
		new_node->transitions = malloc(nt*sizeof(struct automaton_transition*));
		for(j = 0; j < nt; j++){
			struct automaton_transition *old_transition, *new_transition;
			old_transition = old_node->transitions[j];
			new_transition = malloc(sizeof(struct automaton_transition));
			
			*new_transition = *old_transition;
			new_transition->identifier += offset;
			
			new_node->transitions[j] = new_transition;
		}
		
		new_nodes[new_i] = new_node;
	}
}


static void delete_nodes(struct automaton_node **nodes, int n_nodes){
	/**
	 * Frees all memory associated with the nodes provided
	 */
	int i, j;
	for(i = 0; i < n_nodes; i++){
		struct automaton_node *node = nodes[i];
		
		for(j = 0; j < node->n_transitions; j++){
			free(node->transitions[j]);
		}
		free(node->transitions);
		free(node);
	}
	free(nodes);
}


static void encapsulate(FiniteAutomaton *automaton){
	/**
	 * Replaces the automaton at the provided address with a new automaton
	 * identical, except with only one ending state.  Returns the identifier of
	 * the new ending state.
	 */
	if(automaton == NULL){
		return;
	}
	int n = automaton->n_nodes;
	struct automaton_node **old_nodes = automaton->nodes;
	struct automaton_node **new_nodes = malloc((n + 1)*sizeof(struct automaton_node*));
	
	//make single endstate node
	struct automaton_node *end = malloc(sizeof(struct automaton_node));
	end->identifier = n;
	end->n_transitions = 0;
	end->is_ending_state = 1;
	end->transitions = NULL;
	new_nodes[n] = end;
	
	//copy old nodes
	copy_nodes(old_nodes, new_nodes, n, 0);
	
	//replace ending states with new transitions
	int i, j;
	for(i = 0; i < n; i++){
		struct automaton_node *node = new_nodes[i];
		if(node->is_ending_state){
			//make not an ending state
			node->is_ending_state = 0;
			
			//add new transition to real ending state
			int nt = node->n_transitions + 1;
			node->n_transitions = nt;
			
			struct automaton_transition **new_transitions = 
						malloc(nt * sizeof(struct automaton_transition*));
			//copy existing transitions
			for(j = 0; j < nt - 1; j++){
				new_transitions[j] = node->transitions[j];
			}
			//new transition
			struct automaton_transition *transition;
			transition = malloc(sizeof(struct automaton_transition));
			transition->is_epsilon = 1;
			transition->identifier = n;
			new_transitions[nt-1] = transition;
			
			free(node->transitions);
			node->transitions = new_transitions;
		}
	}
	
	
	//replace node object
	automaton->n_nodes++;
	automaton->nodes = new_nodes;
	
	//free memory from old nodes
	delete_nodes(old_nodes, n);
}


static void reduce(FiniteAutomaton *automaton){
	/**
	 * Reroutes the transitions in the given automaton to eliminate transitions
	 * to and from non-finishing nodes with one epsilon transition and no other
	 * transitions.  The starting node, however, will always be kept.  It also
	 * removes all nodes with no inbound or outbound transitions.
	 */
	int i, j, k;
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		
		//check to see if there is any reason to keep the current node
		if(node->identifier == automaton->starting_state){continue;}
		if(node->is_ending_state){continue;}
		if(node->n_transitions != 1){
			if(node->n_transitions == 0){
				//check to see if there are any inbound transitions
				int empty = 1;
				for(j = 0; j < automaton->n_nodes; j++){
					struct automaton_node *from_node = automaton->nodes[j];
					for(k = 0; k < from_node->n_transitions; k++){
						struct automaton_transition *t = from_node->transitions[k];
						if(t->identifier == node->identifier){
							empty = 0;
						}
					}
				}
				if(empty){
					//remove the empty node
					node->n_transitions = -1;
				}
			}
			continue;
		}
		if(!node->transitions[0]->is_epsilon){continue;}
		
		//if we get this far, we need to remove node
		int divert = node->transitions[0]->identifier;
		free(node->transitions[0]);
		free(node->transitions);
		node->transitions = NULL;
		node->n_transitions = -1;
		
		//look for transitions to this node to divert
		for(j = 0; j < automaton->n_nodes; j++){
			struct automaton_node *from_node = automaton->nodes[j];
			for(k = 0; k < from_node->n_transitions; k++){
				struct automaton_transition *t = from_node->transitions[k];
				if(t->identifier == node->identifier){
					t->identifier = divert;
				}
			}
		}
	}
	
	/*
	 * At this point, all of the rerouting has been done.  Now, we need to make
	 * a new automaton with fewer nodes, but the same structure.
	 */
	//first determine which nodes will carry over
	int *new_identifiers = malloc(automaton->n_nodes * sizeof(int));
	int node_counter = 0;
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		
		if(node->n_transitions < 0){
			//will be removed
			new_identifiers[i] = -1;
		}else{
			//will not be removed
			new_identifiers[i] = node_counter;
			node_counter++;
		}
	}
	
	//Create and populate new automaton
	int newsize = node_counter;
	FiniteAutomaton *reduced = create_automaton_empty(newsize);
	reduced->starting_state = new_identifiers[automaton->starting_state];
	
	//migrate transitions and endstates
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *old_node, *new_node;
		old_node = automaton->nodes[i];
		if(new_identifiers[i] < 0){
			//removed node
			continue;
		}
		new_node = reduced->nodes[new_identifiers[i]];
		new_node->is_ending_state = old_node->is_ending_state;
		
		//transitions
		new_node->n_transitions = old_node->n_transitions;
		new_node->transitions = malloc(new_node->n_transitions * 
		                     sizeof(struct automaton_transition*));
		for(j = 0; j < old_node->n_transitions; j++){
			struct automaton_transition *old_transition, *new_transition;
			old_transition = old_node->transitions[j];
			new_transition = malloc(sizeof(struct automaton_transition));
			
			*new_transition = *old_transition;
			new_transition->identifier = new_identifiers[old_transition->identifier];
			
			new_node->transitions[j] = new_transition;
		}
	}
	
	
	
	//clean up and replace
	delete_nodes(automaton->nodes, automaton->n_nodes);
	free(new_identifiers);
	*automaton = *reduced;
}


FiniteAutomaton *create_automaton_char(char c){
	/**
	 * Creates a simple finite automaton for just a char; succeeds iff the
	 * provided char matches c.
	 */
	FiniteAutomaton *automaton = create_automaton_empty(2);
	
	//make transition
	struct automaton_transition *transition = malloc(sizeof(struct automaton_transition));
	transition->condition = c;
	transition->identifier = 1; //links to node 1.
	
	//link transition to starting node
	struct automaton_node *node = automaton->nodes[0];
	node->n_transitions = 1;
	node->transitions = malloc(1*sizeof(struct automaton_transition*));
	node->transitions[0] = transition;
	
	//set ending state
	automaton->nodes[1]->is_ending_state = 1;
	
	return automaton;
}


FiniteAutomaton *create_automaton_alternation(FiniteAutomaton *a1in, FiniteAutomaton *a2in){
	/**
	 * Creates a finite automaton using alternation on a1 and a2.
	 */
	FiniteAutomaton *a1, *a2;
	a1 = copy_automaton(a1in);
	a2 = copy_automaton(a2in);
	
	if(a1 == a2){
		return a1;
	}
	//first encapsulate, so we can combine
	encapsulate(a1);
	encapsulate(a2);
	
	//make new automaton
	int newsize = 2 + a1->n_nodes + a2->n_nodes;
	FiniteAutomaton *automaton = create_automaton_empty(newsize);
	
	//new start node
	struct automaton_node *start = automaton->nodes[0];
	start->n_transitions = 2;
	start->transitions = malloc(2*sizeof(struct automaton_transition*));
	
	//transitions from start node
	struct automaton_transition *t1, *t2;
	t1 = malloc(sizeof(struct automaton_transition));
	t2 = malloc(sizeof(struct automaton_transition));
	t1->is_epsilon = 1;
	t2->is_epsilon = 1;
	t1->identifier = 1;
	t2->identifier = 1 + a1->n_nodes;
	start->transitions[0] = t1;
	start->transitions[1] = t2;
	
	//copy in nodes
	copy_nodes(a1->nodes, automaton->nodes, a1->n_nodes, 1);
	copy_nodes(a2->nodes, automaton->nodes, a2->n_nodes, 1 + a1->n_nodes);
	
	//take care of ending states
	struct automaton_node *end, *e1, *e2;
	end = automaton->nodes[newsize - 1];
	e1 = automaton->nodes[a1->n_nodes];
	e2 = automaton->nodes[newsize - 2];
	
	e1->is_ending_state = 0;
	e2->is_ending_state = 0;
	end->is_ending_state = 1;
	
	//new transitions to end
	t1 = malloc(sizeof(struct automaton_transition));
	t2 = malloc(sizeof(struct automaton_transition));
	t1->is_epsilon = 1;
	t2->is_epsilon = 1;
	t1->identifier = end->identifier;
	t2->identifier = end->identifier;
	
	//add transitions
	e1->n_transitions = 1;
	e2->n_transitions = 1;
	e1->transitions = malloc(1*sizeof(struct automaton_transition*));
	e2->transitions = malloc(1*sizeof(struct automaton_transition*));
	e1->transitions[0] = t1;
	e2->transitions[0] = t2;
	
	
	delete_automaton(a1);
	delete_automaton(a2);
	reduce(automaton);
	return automaton;
}


FiniteAutomaton *create_automaton_concatenation(FiniteAutomaton *a1in,
                                                FiniteAutomaton *a2in){
	/**
	 * Creates a finite automaton by concatenating the two finite automata
	 * provided.
	 */
	FiniteAutomaton *a1, *a2;
	a1 = copy_automaton(a1in);
	a2 = copy_automaton(a2in);
	
	//encapsulate so we can combine
	encapsulate(a1);
	encapsulate(a2);
	
	//make new automaton
	int newsize = a1->n_nodes + a2->n_nodes;
	FiniteAutomaton *automaton = create_automaton_empty(newsize);
	
	//copy in nodes
	copy_nodes(a1->nodes, automaton->nodes, a1->n_nodes, 0);
	copy_nodes(a2->nodes, automaton->nodes, a2->n_nodes, a1->n_nodes);
	
	//transition nodes
	struct automaton_node *e, *s;
	e = automaton->nodes[a1->n_nodes - 1];
	s = automaton->nodes[a1->n_nodes];
	e->is_ending_state = 0;
	
	//make transition
	struct automaton_transition *t;
	t = malloc(sizeof(struct automaton_transition));
	t->is_epsilon = 1;
	t->identifier = s->identifier;
	
	//add transition
	e->n_transitions = 1;
	e->transitions = malloc(1*sizeof(struct automaton_transition*));
	e->transitions[0] = t;
	
	
	delete_automaton(a1);
	delete_automaton(a2);
	reduce(automaton);
	return automaton;
}


FiniteAutomaton *create_automaton_iteration(FiniteAutomaton *ain){
	/**
	 * Create a finite automaton by interating on the provided the automaton
	 */
	FiniteAutomaton *a = copy_automaton(ain);
	
	encapsulate(a);
	
	//make new automaton with extra end state
	int newsize = a->n_nodes + 1;
	FiniteAutomaton *automaton = create_automaton_empty(newsize);
	
	//copy in nodes
	copy_nodes(a->nodes, automaton->nodes, a->n_nodes, 0);
	
	//new and old end nodes and start node
	struct automaton_node *e, *end, *start;
	start = automaton->nodes[0];
	e = automaton->nodes[newsize - 2];
	end = automaton->nodes[newsize - 1];
	
	e->is_ending_state = 0;
	end->is_ending_state = 1;
	
	//make transitions
	struct automaton_transition *tforward, *tback, *tfinish;
	tforward = malloc(sizeof(struct automaton_transition));
	tback = malloc(sizeof(struct automaton_transition));
	tfinish = malloc(sizeof(struct automaton_transition));
	tforward->is_epsilon = 1;
	tback->is_epsilon = 1;
	tfinish->is_epsilon = 1;
	tforward->identifier = end->identifier;
	tback->identifier = start->identifier;
	tfinish->identifier = end->identifier;
	
	//forward transition
	start->n_transitions++;
	int m = start->n_transitions;
	struct automaton_transition **new_transitions;
	new_transitions = malloc(m*sizeof(struct automaton_transition*));
	
	//copy existing start transitions and set new forward transition
	int i;
	for(i = 0; i < m-1; i++){
		new_transitions[i] = start->transitions[i];
	}
	new_transitions[m - 1] = tforward;
	
	//reset start transitions
	free(start->transitions);
	start->transitions = new_transitions;
	
	//set other transitions
	e->n_transitions = 2;
	e->transitions = malloc(2*sizeof(struct automaton_transition*));
	e->transitions[0] = tback;
	e->transitions[1] = tfinish;
	
	delete_automaton(a);
	reduce(automaton);
	return automaton;
}



FiniteAutomaton *copy_automaton(FiniteAutomaton *original){
	/**
	 * Creates and returns a pointer to a deep copy of the provided finite
	 * automaton.
	 */
	FiniteAutomaton *copy = create_automaton_empty(original->n_nodes);
	
	//deep copy of all nodes
	copy_nodes(original->nodes, copy->nodes, original->n_nodes, 0);
	
	return copy;
}



void print_automaton(FiniteAutomaton *automaton){
	/**
	 * Prints the specified finite automaton to the console.
	 */
	int i, j;
	printf("\n");
	char *type;
	if(automaton_is_deterministic(automaton)){
		type = "Deterministic";
	}else{
		type = "Nondeterministic";
	}
	printf("Finite Automaton (%s) of size %d:\n", type, automaton->n_nodes);
	print_separator_line('-');
	
	for(i = 0; i < automaton->n_nodes; i++){
		struct automaton_node *node = automaton->nodes[i];
		printf("|Node: %2d|Ending State: ", node->identifier);
		if(node->is_ending_state){
			printf("Y");
		}else{
			printf("N");
		}
		printf("|Transitions: %2d", node->n_transitions);
		for(j = 0; j < node->n_transitions; j++){
			struct automaton_transition *t = node->transitions[j];
			if(t->is_epsilon){
				printf(" <eps,%2d>", t->identifier);
			}else{
				printf(" <'%c',%2d>", t->condition, t->identifier);
			}
		}
		
		printf("\n");
	}
	
	print_separator_line('-');
	
}


void delete_automaton(FiniteAutomaton *automaton){
	/**
	 * Frees all memory associated with the specified automaton
	 */
	delete_nodes(automaton->nodes, automaton->n_nodes);
	free(automaton);
}
