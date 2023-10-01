/* Program to generate text based on the context provided by input prompts.

  Skeleton program written by Artem Polyvyanyy, http://polyvyanyy.com/,
  September 2023, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.
  All included code is (c) Copyright University of Melbourne, 2023.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial skeleton file,
  the program contained in this submission is completely my own individual
  work, except where explicitly noted by further comments that provide details
  otherwise. I understand that work that has been developed by another student,
  or by me in collaboration with other students, or by non-students as a result
  of request, solicitation, or payment, may not be submitted for assessment in
  this subject. I understand that submitting for assessment work developed by
  or in collaboration with other students or non-students constitutes Academic
  Misconduct, and may be penalized by mark deductions, or by other penalties
  determined via the University of Melbourne Academic Honesty Policy, as
  described at https://academicintegrity.unimelb.edu.au.

  (2) I also certify that I have not provided a copy of this work in either
  softcopy or hardcopy or any other form to any other student, and nor will I
  do so until after the marks are released. I understand that providing my work
  to other students, regardless of my intention or any undertakings made to me
  by that other student, is also Academic Misconduct.

  (3) I further understand that providing a copy of the assignment specification
  to any form of code authoring or assignment tutoring service, or drawing the
  attention of others to such services and code that may have been made
  available via such a service, may be regarded as Student General Misconduct
  (interfering with the teaching activities of the University and/or inciting
  others to commit Academic Misconduct). I understand that an allegation of
  Student General Misconduct may arise regardless of whether or not I personally
  make use of such solutions or sought benefit from such actions.

  Signed by: [Liam Zefu Li 1463868]
  Dated:     [22/09/2023]
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define SDELIM "==STAGE %d============================\n"   // stage delimiter
#define MDELIM "-------------------------------------\n"    // delimiter of -'s
#define THEEND "==THE END============================\n"    // end message
#define NOSFMT "Number of statements: %d\n"                 // no. of statements
#define NOCFMT "Number of characters: %d\n"                 // no. of chars
#define NPSFMT "Number of states: %d\n"                     // no. of states
#define TFQFMT "Total frequency: %d\n"                      // total frequency

#define CRTRNC '\r'                             // carriage return character
#define MAXCHARS 1000  // maximum characters per word

/* Debugging area */
#define DEBUG 1
#if DEBUG
#define DUMP_DBL(x) printf("line %d: %s = %.5f\n", __LINE__, #x, x)
#define DUMP_INT(x) printf("line %d: %s = %d\n", __LINE__, #x, x)
#define DUMP_STR(x) printf("line %d: %s = %s\n", __LINE__, #x, x)
#define DUMP_CHR(x) printf("line %d: %s = %c\n", __LINE__, #x, x)
#else
#define DUMP_DBL(x)
#define DUMP_INT(x)
#define DUMP_STR(x)
#define DUMP_CHR(x)
#endif

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef struct state state_t;   // a state in an automaton
typedef struct node  node_t;    // a node in a linked list
typedef char word_t[MAXCHARS];  // an array to store inputs

struct node {                   // a node in a linked list of transitions has
    char*           str;        // ... a transition string
    state_t*        state;      // ... the state reached via the string, and
    node_t*         next;       // ... a link to the next node in the list.
};

typedef struct {                // a linked list consists of
    node_t*         head;       // ... a pointer to the first node and
    node_t*         tail;       // ... a pointer to the last node in the list.
} list_t;

struct state {                  // a state in an automaton is characterized by
    unsigned int    id;         // ... an identifier,
    unsigned int    freq;       // ... frequency of traversal,
    int             visited;    // ... visited status flag, and
    list_t*         outputs;    // ... a list of output states.
};

typedef struct {                // an automaton consists of
    state_t*        ini;        // ... the initial state, and
    unsigned int    nid;        // ... the identifier of the next new state.
} automaton_t;


/* Function prototypes -------------------------------------------------------*/
int mygetchar(void);            // getchar() that skips carriage returns
int read_input(word_t one_line);
automaton_t *init_automaton(void);
node_t *insert_node(state_t *current_state, char ch);
void insert_state(automaton_t *automaton, node_t *node);
void insert_statement(automaton_t *automaton, char **statement, int lineno);
unsigned int state_num(automaton_t *automaton); 


/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[]) {
    /* Stage 0 */
    word_t one_line;
    int lineno = 0, total_char = 0;
    size_t current_size = MAXCHARS;
    char **all_lines = (char **)malloc(current_size * sizeof(*all_lines));
    assert(all_lines != NULL);

    // 1.read statements from the input
    while (read_input(one_line)) {
        if (lineno == current_size) {
            current_size *= 2;
            all_lines = realloc(all_lines, current_size * sizeof(*all_lines));
            assert(all_lines != NULL);
        }
        all_lines[lineno] = strdup(one_line);
        assert(all_lines[lineno]);
        total_char += strlen(all_lines[lineno]);
        lineno++;
    }

    // 2. set up the start state and then insert states and nodes
    automaton_t *automaton = init_automaton();
    insert_statement(automaton, all_lines, lineno);

    // 3.print the information
    printf(SDELIM, 0);
    printf(NOSFMT, lineno);
    printf(NOCFMT, total_char);
    printf(NPSFMT, state_num(automaton));

    /* Stage 1 */
    printf(SDELIM, 1);
    /* Stage 2 */
    printf(SDELIM, 2);
    return EXIT_SUCCESS;        // algorithms are fun!!!
}

/* USEFUL FUNCTIONS ----------------------------------------------------------*/

// An improved version of getchar(); skips carriage return characters.
// NB: Adapted version of the mygetchar() function by Alistair Moffat
int mygetchar() {
    int c;
    while ((c=getchar())==CRTRNC);
    return c;
}

/* MY FUNCTIONS --------------------------------------------------------------*/

/* reading input, return false if there is no input, otherwise return true */
int
read_input(word_t one_line) {
    int i = 0, ch;
    // read input and adjust memory size if needed
    while (((ch = mygetchar()) != EOF) && (ch != '\n') && i <= MAXCHARS) {
        one_line[i++] = ch;
    }
    one_line[i] = '\0';
    // if no input or just a single new line character, return false
    if (i == 0 && ch == '\n') {
        return 0;
    }
    // input read successfully, return true
    return 1;
}

int
is_element_in_auto(automaton_t *automaton, char *str) {
    state_t *current = automaton->ini;
    while (current != NULL) {
        node_t *node = current->outputs->head;
        while (node != NULL) {
            if (strcmp(node->str, str) == 0) {
                return 1;
            }
            node = node->next;
        }
    }
    return 0;
}


automaton_t
*init_automaton(void) {
    automaton_t *automaton = (automaton_t *)malloc(sizeof(*automaton));
    assert(automaton != NULL);
    automaton->ini = (state_t *)malloc(sizeof(state_t));
    assert(automaton->ini != NULL);
    automaton->ini->outputs = (list_t *)malloc(sizeof(list_t));
    assert(automaton->ini->outputs != NULL);
    automaton->ini->outputs->head = NULL;
    automaton->ini->outputs->tail = NULL;
    automaton->nid = 1;
    return automaton;
}


// insert node at foot
node_t 
*insert_node(state_t *current_state, char ch) {
    node_t *new_node = current_state->outputs->head;
    while (new_node != NULL && new_node->str[0] != ch) {
        new_node = new_node->next;
    }

    if (new_node == NULL) {
        // allocate memory for each component
        new_node = (node_t*)malloc(sizeof(node_t));
        assert(new_node != NULL);
        new_node->str = (char*)malloc(1+sizeof(char));
        assert(new_node->str != NULL);
        new_node->state = (state_t *)malloc(sizeof(state_t));
        assert(new_node->state != NULL);
        // initialise everything
        new_node->str[0] = ch;
        new_node->str[1] = '\0';
        new_node->state = NULL; 
        new_node->next = NULL;

        // Add the node to the current state's outputs
        if (current_state->outputs->head == NULL) {
            current_state->outputs->head = new_node;
            current_state->outputs->tail = new_node;
        } else {
            current_state->outputs->tail->next = new_node;
            current_state->outputs->tail = new_node;
        }        
    }
    return new_node;
}

// insert at foot
void
insert_state(automaton_t *automaton, node_t *node) {
    assert(automaton != NULL);
    // set up id and frequency
    if (node->state == NULL) {
        state_t *new_state = (state_t *)malloc(sizeof(*new_state));
        assert(new_state != NULL);
        new_state->outputs = (list_t *)malloc(sizeof(list_t));
        assert(new_state->outputs != NULL);

        node->state = new_state;
        new_state->id = automaton->nid++;
        new_state->freq = 1;
        new_state->visited = 0;
        new_state->outputs->head = new_state->outputs->tail = NULL;
    } else {
        node->state->freq++;
    }
}

// insert a statement into the automaton
void
insert_statement(automaton_t *automaton, char **statement, int lineno) {
    for (int i = 0; i < lineno; i++) {
        state_t *current_state = automaton->ini;
        for (int j = 0; statement[i][j] != '\0'; j++) {
            node_t *node = insert_node(current_state, statement[i][j]);
            insert_state(automaton, node);
            current_state = node->state;
        }
    }
}

// count the total numbers of states in the automaton
unsigned int state_num(automaton_t *automaton) {
    return automaton->nid;
}

/* THE END -------------------------------------------------------------------*/
