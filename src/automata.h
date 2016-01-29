/**
 * Contains data structures necessary for finite automata
 */

struct automaton_transition{
	int is_epsilon; //indicates if the state is an epsilon
	char condition; //transition is valid if a character matches this
	int identifier; //identifier of the node to which this transition goes
};

struct automaton_node {
	int identifier;
	int n_transitions;
	int is_ending_state; //either zero or one
	struct automaton_transition **transitions;//must be of length n_transitions
};

typedef struct finite_automaton {
	/**
	 * Data structure representing finite automaton.  Contains 
	 * a fixed number of nodes and a lookup table for fast reference.  Each node
	 * is identified by a number starting with zero.
	 */
	 int n_nodes;
	 int starting_state; //identifier for the starting state
	 struct automaton_node **nodes;
	 
	 //lookup table data (only applicable for deterministic automata)
	 int *lookup_table;
	 char *transition_chars;
	 int n_transition_chars;
} FiniteAutomaton;

/*
 * Methods for nondeterministic and deterministic finite automata. (automata.c)
 */
FiniteAutomaton *create_automaton_char(char);
FiniteAutomaton *create_automaton_alternation(FiniteAutomaton*, FiniteAutomaton*);
FiniteAutomaton *create_automaton_concatenation(FiniteAutomaton*, FiniteAutomaton*);
FiniteAutomaton *create_automaton_iteration(FiniteAutomaton*);
FiniteAutomaton *copy_automaton(FiniteAutomaton*);
void print_automaton(FiniteAutomaton*);
void delete_automaton(FiniteAutomaton*);

/*
 * Methods specifically for deterministic finite automata. (deterministic_automata.c)
 */
FiniteAutomaton *create_automaton_deterministic(FiniteAutomaton*);
int automaton_is_deterministic(FiniteAutomaton*);
int automaton_test_string(FiniteAutomaton*, char*, int);


